#include <Arduino.h>
#include <driver/ledc.h>

#define SER 12
#define OE_ 14
#define RCLK 27
#define SRCLK 26

#define PWM_OE_ 0

#define PWM_FREQ 9700
#define PWM_BIT LEDC_TIMER_12_BIT
#define PWM_MAX (1<<12)

const uint8_t hello[]={
	0x7c, 0x10, 0x10, 0x7c, 0x30, 0x58, 0x58, 0x58, 0x00, 0x42, 0x7e, 0x40, 0x00, 0x42, 0x7e, 0x40, 
	0x30, 0x48, 0x48, 0x30, 0x00, 0x00, 0x5e, 0x00
};

/*

Y
↑

7-------------------------------------------------
6                                                |
5                                                |-----
4                                                |-----
3                                                |-----
2                                                |-----
1                                                |
0 1 2 3 4 5 6 7  8 9 a b c d e f  g h i j k l m n    →X


>>> + - - -

+
reg A   B   C   D   E   F   G   H
pin 3   22  6   19  9   16  12  13
fun x4  x0  x5  x1  x6  x2  x7  x3

-
reg A   B   C   D   E   F   G   H   A   B   C   D   E   F   G   H
pin 1   2   4   5   7   8   10  11  24  23  21  20  18  17  15  14
fun y4g y4r y5g y5r y6g y6r y7g y7r y0g y0r y1g y1r y2g y2r y3g y3r

*/

uint16_t swap(uint16_t x){return~(
	((x&0x0001)<<9)|((x&0x0002)<<10)|((x&0x0004)<<11)|((x&0x0008)<<12)|((x&0x0010)>>3)|((x&0x0020)>>2)|((x&0x0040)>>1)|((x&0x0080)>>0)|
	((x&0x0100)<<0)|((x&0x0200)<<1)|((x&0x0400)<<2)|((x&0x0800)<<3)|((x&0x1000)>>12)|((x&0x2000)>>11)|((x&0x4000)>>10)|((x&0x8000)>>9)
);}//r7-0 g7-0  
void send(uint8_t i,uint8_t col){
	const uint8_t anode[]={6,4,2,0,7,5,3,1};
	uint16_t a=swap(hello[i]<<col),b=swap(hello[i+8]<<col),c=swap(hello[i+16]<<col);
	shiftOut(SER,SRCLK,LSBFIRST,a&0xff);shiftOut(SER,SRCLK,LSBFIRST,a>>8);
	shiftOut(SER,SRCLK,LSBFIRST,b&0xff);shiftOut(SER,SRCLK,LSBFIRST,b>>8);
	shiftOut(SER,SRCLK,LSBFIRST,c&0xff);shiftOut(SER,SRCLK,LSBFIRST,c>>8);
	shiftOut(SER,SRCLK,LSBFIRST,1<<anode[i]);
	digitalWrite(RCLK,HIGH);digitalWrite(RCLK,LOW);
}
void setup(){
	pinMode(SER,OUTPUT);digitalWrite(SER,LOW);
	pinMode(OE_,OUTPUT);ledcSetup(PWM_OE_,PWM_FREQ,PWM_BIT);ledcAttachPin(OE_,PWM_OE_);ledcWrite(PWM_OE_,PWM_MAX*(1.-2./3.3));
	pinMode(RCLK,OUTPUT);digitalWrite(RCLK,LOW);
	pinMode(SRCLK,OUTPUT);digitalWrite(SRCLK,LOW);
}
void loop(){
	for(uint8_t i=0;i<8;i++){send(i,0);delay(1);send(i,8);}
}