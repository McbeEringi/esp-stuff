#include <Arduino.h>
#include "Wire.h"
#define ADDR 0x3e
const char neko[]={200, 186, 198, 197, 218, 217, 214, 32, 0};
const char nyan[]={198, 172, 176, 221, 0};

void print(const char *p){
	Wire.beginTransmission(ADDR);
	Wire.write(0x40);for(;*p;p++)Wire.write(*p);
	Wire.endTransmission();
}
void cursor(uint8_t x,uint8_t y){
	Wire.beginTransmission(ADDR);
	Wire.write(0);Wire.write(0x80|(y==0?0:0x40)|(x&0x7));
	Wire.endTransmission();
}

void setup(){
	pinMode(26,OUTPUT);digitalWrite(26,LOW);
	pinMode(25,OUTPUT);digitalWrite(25,HIGH);
	pinMode(33,OUTPUT);
	pinMode(32,OUTPUT);

	Wire.begin(32,33,100000);

	Wire.beginTransmission(ADDR);
	Wire.write(0x80);Wire.write(0x39);
	Wire.write(0x80);Wire.write(0x14);
	Wire.write(0x80);Wire.write(0x70);
	Wire.write(0x80);Wire.write(0x56);
	Wire.write(0x80);Wire.write(0x6c);
	delay(200);
	Wire.write(0x80);Wire.write(0x38);
	Wire.write(0x80);Wire.write(0x0c);
	Wire.write(0x80);Wire.write(0x01);
	Wire.write(0x00);Wire.write(0x02);
	Wire.endTransmission();

	// ((s,d=[...' 。「」、・をぁぃぅぇぉゃゅょっーあいうえおかきくけこさしすせそたちつてとなにぬねのはひふへほまみむめもやゆよらりるれろわん'].reduce((a,x,i)=>(a[x]=i+0xa0,a),{}))=>[...s].map(x=>d[x]))('ねこになれるよ')

	cursor(0,0);print(neko);
	// cursor(0,1);
	print(nyan);print(nyan);print(nyan);print(nyan);print(nyan);print(nyan);print(nyan);


}
void loop(){

	delay(500);
	Wire.beginTransmission(ADDR);
	Wire.write(0x00);Wire.write(0x18);
	Wire.endTransmission();
}