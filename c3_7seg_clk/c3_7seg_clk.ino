#include <driver/ledc.h>
#include <WiFi.h>

#define REV 1

#if REV==0
  #define VCC 9
  #define SER 7
  #define OE_ 5
  #define RCLK 6
  #define SRCLK 8
  // ABCDEFGH<<ABCDEFGH<<MCU
  // _789abc_  _654321_
  // _b32fa1_  _4gc.de_
  // _+--++-_  _-+++++_
  #define SWAP(x,i) ((x&128)<<3)|((x&64)<<8)|((x&32)>>1)|((x&16)>>2)|((x&8)>>2)|((x&4)<<9)|((x&2)<<4)|((x&1)<<3)|((~i&1)<<9)|((~i&2)<<11)|((~i&4)<<11)|((~i&8)<<3)
#elif REV==1
  #define SER 4
  #define OE_ 5
  #define RCLK 6
  #define SRCLK 7
  #define ALS 3
  // HGFEDCBAHGFEDCBA<<MCU
  // 9abcdefg87654321
  // __f_cagb4.3e:d21
  // __+_++++-+-+-+--
  #define SWAP(x,i) ((x&128)<<3)|((x&64)<<2)|((x&32)<<6)|((x&16)>>2)|((x&8)<<1)|((x&4)<<11)|((x&2)<<8)|((x&1)<<6)|(~i&3)|((~i&4)<<3)|((~i&8)<<4)|((~i&16)>>1)
#endif

#define PWM_OE_ 0

#define PWM_FREQ 9700
#define PWM_BIT LEDC_TIMER_12_BIT
#define PWM_MAX (1<<12)
typedef struct timeval timeval_t;
typedef struct tm tm_t;

TaskHandle_t *h_flush,*h_dimmer;
timeval_t tv;tm_t tm;
const uint8_t num[]={0b11111100,0b01100000,0b11011010,0b11110010,0b01100110,0b10110110,0b10111110,0b11100000,0b11111110,0b11110110,0b11101110,0b00111110,0b10011100,0b01111010,0b10011110,0b10001110};
bool synced;

uint8_t _disp[5]={2,2,2,2,0};
void send(uint8_t x,uint8_t i){uint16_t a=SWAP(x,i);shiftOut(SER,SRCLK,MSBFIRST,a>>8);shiftOut(SER,SRCLK,MSBFIRST,a&0xff);digitalWrite(RCLK,HIGH);digitalWrite(RCLK,LOW);}// abcdefg. ___:4321
void flush(void *_){uint8_t i;while(1){for(i=0;i<5;i++){send(_disp[i],1<<i);delay(2);}}}
#ifdef ALS
  void dimmer(void *_){uint16_t a,x=1;uint8_t i;while(1){a=analogRead(ALS);for(i=0;i<20;i++){if(a!=x)x+=abs(a-x)<16?(a<x?-1:1):(a-x)/16;ledcWrite(PWM_OE_,PWM_MAX*(1.-x/4095.));delay(100);}}}
#endif
void dispInit(){
  #ifdef VCC
    pinMode(VCC,OUTPUT);digitalWrite(VCC,HIGH);
  #endif
  pinMode(SER,OUTPUT);digitalWrite(SER,LOW);
  pinMode(OE_,OUTPUT);ledcSetup(PWM_OE_,PWM_FREQ,PWM_BIT);ledcAttachPin(OE_,PWM_OE_);ledcWrite(PWM_OE_,PWM_MAX*(1.-1./3.3));//PWM_MAX-x x=2~45676 Vf=2.1~2.3
  pinMode(RCLK,OUTPUT);digitalWrite(RCLK,LOW);
  pinMode(SRCLK,OUTPUT);digitalWrite(SRCLK,LOW);
  xTaskCreateUniversal(flush,"flush",512,NULL,1,h_flush,CONFIG_ARDUINO_RUNNING_CORE);
  #ifdef ALS
    pinMode(ALS,INPUT);
    xTaskCreateUniversal(dimmer,"dimmer",512,NULL,1,h_dimmer,CONFIG_ARDUINO_RUNNING_CORE);
  #endif
}

void sync(void *_){
  synced=false;
  WiFi.begin();
  for(uint8_t i=0;i<10;i++){
    synced=!(i&1);
    if(WiFi.status()==WL_CONNECTED){synced=false;configTzTime("JST-9","ntp.nict.jp");getLocalTime(&tm);synced=true;break;}
    delay(1000);
  }
  WiFi.disconnect(true);
  vTaskDelete(NULL);
}
void ntpSync(){xTaskCreateUniversal(sync,"sync",4096,NULL,1,NULL,CONFIG_ARDUINO_RUNNING_CORE);}

void setup(){
	dispInit();
  WiFi.begin();
	for(uint8_t i=0;WiFi.status()!=WL_CONNECTED;i++){
		if(i>20){
      _disp[0]=0b10011100;_disp[1]=0b10001110;_disp[2]=0b10111100;_disp[3]=0b00010000;
      WiFi.beginSmartConfig();while(!WiFi.smartConfigDone());i=0;for(uint8_t i=0;i<4;i++)_disp[i]=2;
    }
		delay(500);
	}
  _disp[4]=0b11000000;
  configTzTime("JST-9","ntp.nict.jp");getLocalTime(&tm);synced=true;
  WiFi.disconnect(true);
}
void loop(){
  if(!tm.tm_min&&!tm.tm_sec&&tv.tv_usec<200000)ntpSync();
	gettimeofday(&tv,NULL);localtime_r(&tv.tv_sec,&tm);//https://8ttyan.hatenablog.com/entry/2015/02/03/003428
  switch(tm.tm_sec%10){
    case 4:{int8_t t=int8_t(floor(temperatureRead()))-5;_disp[0]=t<0?2:0;_disp[1]=num[abs(t/10%10)];_disp[2]=num[abs(t%10)];_disp[3]=0b10011100;_disp[4]=0b00100000;break;}
    case 5:_disp[0]=(tm.tm_mon+1)<10?0:num[(tm.tm_mon+1)/10];_disp[1]=num[(tm.tm_mon+1)%10]|1;_disp[2]=tm.tm_mday<10?0:num[tm.tm_mday/10];_disp[3]=num[tm.tm_mday%10]|synced;_disp[4]=0;break;
    default:_disp[0]=tm.tm_hour<10?0:num[tm.tm_hour/10];_disp[1]=num[tm.tm_hour%10];_disp[2]=num[tm.tm_min/10];_disp[3]=num[tm.tm_min%10]|synced;_disp[4]=tm.tm_sec&1?0b11000000:0;
  }
	delay(1000-tv.tv_usec/1000);
}
