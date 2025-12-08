#include <Arduino.h>
HardwareSerial escpos(0);
uint8_t btn=0;
void setup(){
	Serial.begin(115200);
	escpos.begin(115200);
	pinMode(2,INPUT_PULLUP);
	pinMode(3,INPUT_PULLUP);
	delay(1000);
	escpos.write("\x1b\x40");
	// escpos.write("\x1b##SELF");
}
void loop(){
	while(Serial.available()){
		uint8_t x=Serial.read();
		Serial.write(x);
		escpos.write(x);
	}
	uint8_t x=((!digitalRead(2))<<1)|(!digitalRead(3));
	if(x!=btn){
		if(x&0b10)escpos.write("\x1b##SELF");
		btn=x;
	}
	// while(Serial1.available())Serial.write(Serial1.read());
	neopixelWrite(1,
		(sin(millis()/1000.    )*.5+.5)*16.,
		(sin(millis()/1000.+2.1)*.5+.5)*16.,
		(sin(millis()/1000.+4.2)*.5+.5)*16.
	);
	delay(10);
}
