#include <Arduino.h>
#include <LittleFS.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoOTA.h>

#define FSYS LittleFS
#define NAME "esp-qr-reader"
#define PASS "mcbeeringi"


#define BUFLEN 256

HardwareSerial reader(0);
uint8_t buf[BUFLEN];

static AsyncWebServer svr(80);
static AsyncWebSocket ws("/ws");


void setup(){
	FSYS.begin();
	Serial.begin(115200);
	reader.begin(9600);
	delay(1000);
	Serial.printf("wifi...\n");
	WiFi.begin();
	while(WiFi.status()!=WL_CONNECTED)delay(50);
	Serial.printf("connected!\n");
	WiFi.setAutoReconnect(true);

	ArduinoOTA
		.setHostname(NAME).setPassword(PASS)
		.onStart([](){ws.enable(false);ws.textAll("OTA update started.");ws.closeAll();FSYS.end();})
		.onError([](ota_error_t e){delay(3000);ws.enable(true);FSYS.begin();})
		.begin();

	svr.addHandler(&ws);
	svr.onNotFound([](AsyncWebServerRequest *req){
		req->send(FSYS,"/index.html");
	});
	svr.begin();
}
void loop(){
	ArduinoOTA.handle();
	while(reader.available()){
		delay(100);
		uint8_t l=min(BUFLEN,reader.available());
		reader.readBytes(buf,l);
		Serial.write(buf,l);
		ws.binaryAll(buf,l);
	}
	delay(10);
}
