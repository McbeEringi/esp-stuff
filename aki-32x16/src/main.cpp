#include <Arduino.h>
#include <driver/ledc.h>
#include <LittleFS.h>
#define FSYS LittleFS
#include "SD.h"


#define NUM_PANEL 3
#define BUF_SIZE 192 // 16*32*NUM_PAMEL/8
#define HASHL (NUM_PANEL*4)

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

#include "buf.h"

uint8_t hashi=0,rstcnt=0;
uint32_t hash[HASHL]={};
// auto *txt=u8"Lorem ipsum であのイーハトーヴォの世界が広がります　　　　あのイーハトーヴォのすきとおった風、夏でも底に冷たさをもつ青いそら、うつくしい森で飾られたモリーオ市、郊外のぎらぎらひかる草の波。";

#include "disp.h"
#include "gol.h"
#include "font.h"

void setup(){
	randomSeed(analogRead(0));
	SPI.begin(SPICLK,SPIMISO,SPIMOSI,SPICS);
	SD.begin(SPICS);
	FSYS.begin();
	pinMode(BTN,INPUT_PULLUP);
	dispInit();
	fontInit("/main.font");
	delay(1000);

	File txt=SD.open("/main.txt");
	if(txt)while(1){
		uint32_t x;
		auto read=[&txt]()->uint8_t{uint8_t x;txt.read(&x,1);return x;};
		uint8_t _x=read();
		if(!_x){txt.close();break;}
		if(_x>>7==0)x=_x;
		else if(_x>>5==0b00110)x=((_x&0x1f)<< 6)|((read()&0x3f)<< 0);
		else if(_x>>4==0b01110)x=((_x&0x0f)<<12)|((read()&0x3f)<< 6)|((read()&0x3f)<<0);
		else if(_x>>3==0b11110)x=((_x&0x07)<<18)|((read()&0x3f)<<12)|((read()&0x3f)<<6)|((read()&0x3f)<<0);
		x=ftx(x);
		if(x){
			font.seek(x&0xffffff);
			x=x>>24;
			uint8_t
				w=(x>>4)+1,
				h=(x&15)+1,
				l=(w*h+7)/8;
			uint8_t *a=(uint8_t*)malloc(l);
			font.read(a,l);

			for(uint8_t i=0;i<h;++i){
				scrollX();
				memcpy(buf+BUF_SIZE-2,a+i*2,2);
				delay(20);
			}
			free(a);
		}
	}
	delay(1000);
	golInit(true);
}
void loop(){
	if(digitalRead(BTN)==LOW)++rstcnt;
	uint32_t _hash=gol();

	if(rstcnt){
		dispBri(0x20);
		if(40<++rstcnt)golInit();
	}else{
		for(uint8_t i=0;i<HASHL;++i){
			if(hash[i]==_hash)++rstcnt;
		}
		if(hashi%16==0)hash[hashi/16]=_hash;
		hashi=(hashi+1)%HASHL;
	}

	delay(50);
}
