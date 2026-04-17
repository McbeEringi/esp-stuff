#include <Arduino.h>
#include <driver/ledc.h>


#define NUM_PANEL 3
#define BUF_SIZE 192 // 16*32*NUM_PAMEL/8
#define HASHL (NUM_PANEL*2)

#define S0 4
#define S1 0
#define S2 5
#define SCK 1
#define RCK 6
#define OE 2
#define OE_CH 0

#define BTN 9

#define SPIMISO 8
#define SPICLK 7
#define SPIMOSI 3
#define SPICS 10

// #define _BV(X) (1<<(X))
const uint32_t smask=_BV(S0)|_BV(S1)|_BV(S2);
const uint8_t sum3[]={0,1,1,2,1,2,2,3};

uint8_t buf[2][BUF_SIZE]={{
0b00000001,0b00000000,0b00000000,0b00000000, 0b00000000,0b00000000,0b00100000,0b00001000, 0b00100010,0b00000000,0b00000010,0b00000000,
0b10000001,0b00000000,0b00000000,0b01100000, 0b00000000,0b00011100,0b00100000,0b11111111, 0b11111110,0b00111111,0b11111110,0b00000000,
0b01000001,0b00000000,0b01000000,0b00100000, 0b00000000,0b11110000,0b00100000,0b00001000, 0b00100000,0b00100000,0b01000000,0b00000000,
0b00100010,0b00000100,0b00100000,0b00100000, 0b00000000,0b00010000,0b00100010,0b00010001, 0b00010010,0b00100000,0b10000000,0b00000000,
0b00000011,0b11111110,0b00100100,0b00111110, 0b00000000,0b00010001,0b00100100,0b11111111, 0b11111110,0b00100111,0b11111100,0b00000000,
0b00010010,0b01000100,0b00100011,0b11100000, 0b00000000,0b11111101,0b00101000,0b00010001, 0b00010000,0b00100100,0b00000100,0b00000000,
0b00010100,0b01001000,0b01000000,0b00100000, 0b00000000,0b00010001,0b00110000,0b00010001, 0b11110000,0b00100100,0b00000100,0b00000000,
0b00100100,0b01001000,0b01000000,0b00100000, 0b00000000,0b00010010,0b00100000,0b00010000, 0b00000000,0b00100111,0b11111100,0b00000000,
0b00100000,0b01000000,0b01000000,0b00100000, 0b00000000,0b00111000,0b01010000,0b00011111, 0b11111100,0b00100100,0b00000100,0b00000000,
0b00100000,0b10100000,0b01000000,0b00100000, 0b00000000,0b01010100,0b01010000,0b00000001, 0b00000000,0b00100100,0b00000100,0b00000000,
0b11000000,0b10100000,0b01000000,0b00100000, 0b00000000,0b01010010,0b01010000,0b00000001, 0b00000010,0b00100111,0b11111100,0b00000000,
0b01000001,0b00010000,0b01010011,0b11110000, 0b00000000,0b10010000,0b10001000,0b11111111, 0b11111110,0b01000000,0b01000000,0b00000000,
0b01000001,0b00010000,0b01010100,0b00101100, 0b00000000,0b10010000,0b10001000,0b00000101, 0b01000000,0b01000100,0b01001000,0b00000000,
0b01000010,0b00001000,0b00100010,0b00100010, 0b00000000,0b00010001,0b00000100,0b00001001, 0b00100000,0b01000100,0b01000100,0b00000000,
0b00001100,0b00000100,0b00100001,0b11100000, 0b00000000,0b00010010,0b00000100,0b00110001, 0b00011000,0b10001000,0b01000010,0b00000000,
0b00110000,0b00000010,0b00000000,0b00000000, 0b00000000,0b00010100,0b00000010,0b11000001, 0b00000110,0b10010001,0b11000010,0b00000000
// 0b00000000,0,0,0, 0,0,0,0, 0,0,0,0,
// 0b00010000,0,0,0, 0,0,0,0, 0,0,0,0,
// 0b00001100,0,0,0, 0,0,0,0, 0,0,0,0,
// 0b00011000,0,0,0, 0,0,0,0, 0,0,0,0,
// 0,0,0,0, 0,0,0,0, 0,0,0,0,
// 0,0,0,0, 0,0,0,0, 0,0,0,0,
// 0,0,0,0, 0,0,0,0, 0,0,0,0,
// 0,0,0,0, 0,0,0,0, 0,0,0,0,
// 0,0,0,0, 0,0,0,0, 0,0,0,0,
// 0,0,0,0, 0,0,0,0, 0,0,0,0,
// 0,0,0,0, 0,0,0,0, 0,0,0,0,
// 0,0,0,0, 0,0,0,0, 0,0,0,0,
// 0,0,0,0, 0,0,0,0, 0,0,0,0,
// 0,0,0,0, 0,0,0,0, 0,0,0,0,
// 0,0,0,0, 0,0,0,0, 0,0,0,0,
// 0,0,0,0, 0,0,0,0, 0,0,0,0,
},{}};
uint8_t bufi=0,hashi=0,rstcnt=0;
uint32_t hash[HASHL]={};

TaskHandle_t *h_flush;

void flush(void *_){
	while(1){
		for(uint8_t j=0;j<16;++j){
			for(uint8_t k=0;k<NUM_PANEL;++k){
				for(uint8_t i=0;i<16;++i){
					uint8_t
						o=(NUM_PANEL*j+(NUM_PANEL-1-k))*4+(1-i/8),
						_i=i%8;

					uint32_t
						sdata=(
							(i==j)<<S0
						)|(
							((buf[bufi][o  ]>>_i)&1)<<S1
						)|(
							((buf[bufi][o+2]>>_i)&1)<<S2
						);

					GPIO.out_w1tc.val=~sdata&smask;
					GPIO.out_w1ts.val= sdata&smask;

					GPIO.out_w1ts.val=_BV(SCK);
					GPIO.out_w1tc.val=_BV(SCK);
				}
			}
			GPIO.out_w1tc.val=_BV(RCK);
			GPIO.out_w1ts.val=_BV(RCK);
			delayMicroseconds(500);
		}
	}
}

void dispBri(uint8_t x){ledcWrite(OE_CH,0xff-x);}
void dispInit(){
	pinMode(S0,OUTPUT);
	pinMode(S1,OUTPUT);
	pinMode(S2,OUTPUT);
	pinMode(SCK,OUTPUT);
	pinMode(RCK,OUTPUT);
	// GPIO.enable_w1ts.val=smask|_BV(SCK)|_BV(RCK);
	GPIO.out_w1ts.val=_BV(RCK);

	ledcSetup(OE_CH,65536,LEDC_TIMER_8_BIT);ledcAttachPin(OE,OE_CH);
	dispBri(0x80);
	xTaskCreateUniversal(flush,"flush",512,NULL,1,h_flush,CONFIG_ARDUINO_RUNNING_CORE);
}

void lgInit(bool keep){
	rstcnt=0;
	dispBri(0x80);
	if(!keep)for(uint8_t i=0;i<BUF_SIZE;++i)buf[bufi][i]=random(0x100);
}
void lgInit(){lgInit(false);}

void setup(){
	randomSeed(analogRead(0));
	pinMode(BTN,INPUT_PULLUP);
	dispInit();
	delay(3000);
	lgInit(true);
}
void loop(){
	if(digitalRead(BTN)==LOW)++rstcnt;
	uint8_t bufin=!bufi;
	uint32_t _hash=0;
	for(uint8_t i=0;i<BUF_SIZE;++i){
		// buf[bufi][i]=random(0x100);

		uint16_t tmp[3];
		uint8_t w=NUM_PANEL*4,h=16,x=i%w,y=i/w,a=0;
		for(uint8_t j=0;j<3;++j){
			uint8_t _y=(y+h-1+j)%h*w;
			tmp[j]=(buf[bufi][(x+w-1)%w+_y]<<12)|(buf[bufi][x+_y]<<4)|(buf[bufi][(x+1)%w+_y]>>4);
		}
		for(uint8_t j=0;j<8;++j){
			uint8_t
				c=(buf[bufi][i]>>j)&1,
				score=
					sum3[(tmp[0]>>(j+3))&0b111]+
					sum3[(tmp[1]>>(j+3))&0b101]+
					sum3[(tmp[2]>>(j+3))&0b111];

			c=c?(score<=1||4<=score)?0:c:(score==3)?1:c;
			a=a|c<<j;
		}
		buf[bufin][i]=a;
		_hash=((_hash<<5)|(_hash>>27))^((i<<8)|a);
	}
	bufi=bufin;

	if(rstcnt){
		dispBri(0x20);
		if(40<++rstcnt)lgInit();
	}else{
		for(uint8_t i=0;i<HASHL;++i){
			if(hash[i]==_hash)++rstcnt;
		}
		if(hashi%16==0)hash[hashi/16]=_hash;
		hashi=(hashi+1)%HASHL;
	}
	delay(50);
}
