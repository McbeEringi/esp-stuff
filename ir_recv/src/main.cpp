#include <Arduino.h>
hw_timer_t * timer = NULL;
uint16_t t=65535;
uint8_t flag=0;

uint16_t cc=0;

void IRAM_ATTR recv_cb(){t=0;}
void IRAM_ATTR timer_cb(){flag=0;}
void setup(){
  // IrReceiver.begin(4, ENABLE_LED_FEEDBACK); // Start the receiver
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
	if(t<100){
		if(t==0){
			cc=0;
		}
		if(24<=t&&t<40){cc=cc<<1;cc|=digitalRead(4)?1:0;}
		if(40<=t&&cc==0xc68a)
		neopixelWrite(9, 0,4,0);

		t++;
	}else neopixelWrite(9, 4,4,0);
	while(flag)delayMicroseconds(10);
}
