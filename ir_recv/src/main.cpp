#include <Arduino.h>
hw_timer_t * timer = NULL;
uint16_t t=255;
uint8_t flag=0;

uint8_t cnt=0;
uint8_t prev=0;
uint32_t data=0;

void IRAM_ATTR recv_cb(){if(150<t){t=0;cnt=0;prev=0;data=0;}}
void IRAM_ATTR timer_cb(){flag=0;}

uint8_t rev8(uint8_t x){uint8_t r=0;for(uint8_t i=0;i<8;++i)r=(r<<1)|((x>>i)&1);return r;}
uint32_t rev(uint32_t x){uint32_t r=0;for(uint8_t i=0;i<32;i+=8)r|=rev8(x>>i)<<i;return r;}

void setup(){
	Serial.begin(115200);
	neopixelWrite(9, 4,0,4);
	attachInterrupt(4, recv_cb,FALLING);
	timer=timerBegin(0, getApbFrequency()/1000000, true);
  timerAttachInterrupt(timer,timer_cb,true);
  timerAlarmWrite(timer,562,true);
  timerAlarmEnable(timer);
	delay(500);
}

void loop(){
	flag=1;
	delayMicroseconds(10);
	if(t<200){
		uint8_t x=digitalRead(4)?0:1;
		if(x^prev&x){data=data<<1;data|=2<cnt;cnt=0;}
		if(148==t){
			data=rev(data);
			Serial.printf("%08lx\n",data);
			if(!((data>>16)^0x6ca8))Serial.printf("\t%d\n",(data>>8)&0xff);
		}

		neopixelWrite(9, 0,4,0);
		cnt++;
		t++;
	}else neopixelWrite(9, 4,4,0);
	while(flag)delayMicroseconds(10);
}
