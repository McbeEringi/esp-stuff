#include <Arduino.h>
HardwareSerial escpos(0);
void setup(){
	Serial.begin(115200);
	escpos.begin(115200);
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
	// while(Serial1.available())Serial.write(Serial1.read());
	delay(10);
}
