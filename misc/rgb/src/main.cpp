#include <Arduino.h>
void setup(){}
void loop(){neopixelWrite(8,
	(sin(millis()/1000.    )*.5+.5)*16.,
	(sin(millis()/1000.+2.1)*.5+.5)*16.,
	(sin(millis()/1000.+4.2)*.5+.5)*16.
);delay(1);}