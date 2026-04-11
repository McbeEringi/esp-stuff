#include <Arduino.h>

#define BUFLEN 256

HardwareSerial reader(0);
uint8_t buf[BUFLEN];

void setup(){
	Serial.begin(115200);
	reader.begin(9600);
}
void loop(){
	while(reader.available()){
		uint8_t l=min(BUFLEN,reader.available());
		reader.readBytes(buf,l);
		Serial.write(buf,l);
	}
	delay(10);
}
