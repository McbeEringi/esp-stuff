#include <Arduino.h>
#include "EspUsbHost.h"

class MyEspUsbHost : public EspUsbHost {
  void onKeyboardKey(uint8_t ascii, uint8_t keycode, uint8_t modifier) {
    if (' ' <= ascii && ascii <= '~') {
      Serial.printf("%c", ascii);
    } else if (ascii == '\r') {
      Serial.println();
    }

		neopixelWrite(48,
			random(16),
			random(16),
			random(16)
		);
  };
  void onMouseMove(hid_mouse_report_t const report){
    Serial.printf("buttons=0x%02x(%c%c%c%c%c), x=%d, y=%d, wheel=%d\n",
                  report.buttons,
                  (report.buttons & MOUSE_BUTTON_LEFT) ? 'L' : ' ',
                  (report.buttons & MOUSE_BUTTON_RIGHT) ? 'R' : ' ',
                  (report.buttons & MOUSE_BUTTON_MIDDLE) ? 'M' : ' ',
                  (report.buttons & MOUSE_BUTTON_BACKWARD) ? 'B' : ' ',
                  (report.buttons & MOUSE_BUTTON_FORWARD) ? 'F' : ' ',
                  report.x,
                  report.y,
                  report.wheel);
  };
};

MyEspUsbHost usbHost;

void setup() {
  Serial.begin(115200);
  delay(500);

  usbHost.begin();
  usbHost.setHIDLocal(HID_LOCAL_Japan_Katakana);
}

void loop() {
  usbHost.task();
}