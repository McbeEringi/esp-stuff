#include <driver/ledc.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"


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

#include "buf.h"

uint8_t hashi=0,rstcnt=0;
uint32_t hash[HASHL]={};

#include "disp.h"
#include "gol.h"
#include "font.h"

void setup(){
	randomSeed(analogRead(0));
	SPI.begin(SPICLK,SPIMISO,SPIMOSI,SPICS);
	SD.begin(SPICS);
	pinMode(BTN,INPUT_PULLUP);
	dispInit();
	fontInit("/main.font");
	for(uint8_t i=0,o=0;i<6;++i){
		o+=drawFont(((uint16_t[]){0x5de5,0x5b66,0x7814,0x7a76,0x90e8,0xff01})[i],o);
	}
	delay(2000);
	for(uint16_t i=0;i<16*2;++i){
		scrollY();
		delay(200);
	}
	for(uint16_t i=0;i<NUM_PANEL*32*8;++i){
		scrollX();
		delay(100);
	}
	delay(500);
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
