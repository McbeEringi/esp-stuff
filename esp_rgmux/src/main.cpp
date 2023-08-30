#include <Arduino.h>
#include <driver/ledc.h>
#include <LittleFS.h>
#define FSYS LittleFS
#include <ESPAsyncWebSrv.h>
#include <ESPmDNS.h>


#define SER 12
#define OE_ 14
#define RCLK 27
#define SRCLK 26

#define PWM_OE_ 0

#define PWM_FREQ 9700
#define PWM_BIT LEDC_TIMER_12_BIT
#define PWM_MAX (1<<12)

TaskHandle_t *h_flush;
uint16_t disp[256]={//rg
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,146,112,136,254,212,254,212,0,41984,65024,12288,39936,45056,24064,13312,0,31868,32896,16448,0,0,1028,14392,0,196,52,142,132,120,4,24,0,50176,13312,36352,33792,31232,0,6656,0,1028,1028,29812,35980,38036,33924,5140,0,4,20,172,164,126,4,4,0,50176,13312,36352,33792,30720,1024,6144,0,0,1028,514,41634,4626,4626,3084,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};
uint8_t displ=128;

/*

 0  1  2  3  4  5  6  7  X
0                      |
1                      |
2                      |
3                      |
4                      |
5                      |
6                      |
7                      |

0                      |
1                      |
2                      |
3                      |
4                      |
5                      |
6                      |
7                      |

0                      |
1                      |
2                      |
3                      |
4                      |
5                      |
6                      |
7                      |
        ||||||
Y


>>> + - - -

+
reg A   B   C   D   E   F   G   H
pin 3   22  6   19  9   16  12  13
fun y4  y0  y5  y1  y6  y2  y7  y3

-
reg A   B   C   D   E   F   G   H   A   B   C   D   E   F   G   H
pin 1   2   4   5   7   8   10  11  24  23  21  20  18  17  15  14
fun x4g x4r x5g x5r x6g x6r x7g x7r x0g x0r x1g x1r x2g x2r x3g x3r

*/

uint16_t swap(uint16_t x){return~(
	((x&0x0001)<<9)|((x&0x0002)<<10)|((x&0x0004)<<11)|((x&0x0008)<<12)|((x&0x0010)>>3)|((x&0x0020)>>2)|((x&0x0040)>>1)|((x&0x0080)>>0)|
	((x&0x0100)<<0)|((x&0x0200)<<1)|((x&0x0400)<<2)|((x&0x0800)<<3)|((x&0x1000)>>12)|((x&0x2000)>>11)|((x&0x4000)>>10)|((x&0x8000)>>9)
);}//r7-0 g7-0  
void send(uint8_t i,uint8_t o,uint16_t mask){
	const uint8_t anode[]={6,4,2,0,7,5,3,1};
	uint16_t a=swap(disp[i+o]&mask),b=swap(disp[i+o+8]&mask),c=swap(disp[i+o+16]&mask);
	shiftOut(SER,SRCLK,LSBFIRST,a&0xff);shiftOut(SER,SRCLK,LSBFIRST,a>>8);
	shiftOut(SER,SRCLK,LSBFIRST,b&0xff);shiftOut(SER,SRCLK,LSBFIRST,b>>8);
	shiftOut(SER,SRCLK,LSBFIRST,c&0xff);shiftOut(SER,SRCLK,LSBFIRST,c>>8);
	shiftOut(SER,SRCLK,LSBFIRST,1<<anode[i]);
	digitalWrite(RCLK,HIGH);digitalWrite(RCLK,LOW);
}
void flush(void *_){
	while(1)for(uint8_t k=0;k<=(displ-24);k++)for(uint8_t j=0;j<8;j++)for(uint8_t i=0;i<8;i++){send(i,k,0x00ff);delayMicroseconds(1000);send(i,k,0xffff);delayMicroseconds(100);}
}
void dispInit(){
	pinMode(SER,OUTPUT);digitalWrite(SER,LOW);
	pinMode(OE_,OUTPUT);ledcSetup(PWM_OE_,PWM_FREQ,PWM_BIT);ledcAttachPin(OE_,PWM_OE_);ledcWrite(PWM_OE_,PWM_MAX*(1.-2./3.3));
	pinMode(RCLK,OUTPUT);digitalWrite(RCLK,LOW);
	pinMode(SRCLK,OUTPUT);digitalWrite(SRCLK,LOW);
	xTaskCreateUniversal(flush,"flush",1024,NULL,1,h_flush,CONFIG_ARDUINO_RUNNING_CORE);
}

void setup(){
	dispInit();
}
void loop(){
}