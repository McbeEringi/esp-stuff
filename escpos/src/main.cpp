#include <Arduino.h>
#include <LittleFS.h>
#include "wlan.h"
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoOTA.h>

#define FSYS LittleFS
#define NAME "esp-tp"
#define PASS "mcbeeringi"

#define PUB_DIR "/public/"
#define PVT_DIR "/private/"
#define WIFI_CFG "/private/wifi.tsv"

#define NPDI 1
#define BTNA 2
#define BTNB 3

#define P_INIT "\x1b\x40"
#define P_SELF "\x1b##SELF"

HardwareSerial escpos(0);

static AsyncWebServer svr(80);
static AsyncWebSocket ws("/ws");

bool isAPmode=false;

struct Btn{bool a;bool b;};
Btn btn={false,false};
void ARDUINO_ISR_ATTR isr_btn_a(void *arg){(static_cast<Btn *>(arg))->a=true;}
void ARDUINO_ISR_ATTR isr_btn_b(void *arg){(static_cast<Btn *>(arg))->b=true;}


bool fs_send(AsyncWebServerRequest *req){
	String x=req->url();
	if(x[x.length()-1]=='/')x+="index.html";
	if(FSYS.exists(x)){req->send(FSYS,x);return true;}
	return false;
}

void setup(){
	neopixelWrite(NPDI,16,0,0);
	FSYS.begin();
	Serial.begin(115200);
	escpos.begin(115200);
	escpos.write(P_INIT);
	pinMode(BTNA,INPUT_PULLUP);
	attachInterruptArg(BTNA,isr_btn_a,&btn,FALLING);
	pinMode(BTNB,INPUT_PULLUP);
	attachInterruptArg(BTNB,isr_btn_b,&btn,FALLING);
	delay(1000);

	isAPmode=wlan(
		NPDI,
		FSYS,WIFI_CFG,
		NAME,PASS
	);

	for(uint8_t i=8;i;--i){
		if(isAPmode)neopixelWrite(NPDI,0,16,16);
		else neopixelWrite(NPDI,0,16,0);
		delay(62);
		neopixelWrite(NPDI,0,0,0);
		delay(62);
	}

	ArduinoOTA
		.setHostname(NAME).setPassword(PASS)
		.onStart([](){ws.enable(false);ws.textAll("OTA update started.");ws.closeAll();FSYS.end();})
		.onProgress([](unsigned int x,unsigned int a){neopixelWrite(NPDI,(a-x)*64/a,x*64/a,0);})
		.onError([](ota_error_t e){neopixelWrite(NPDI,64,0,64);delay(3000);ws.enable(true);FSYS.begin();})
		.begin();

	ws.onEvent([](
		AsyncWebSocket *svr,AsyncWebSocketClient *cli,
		AwsEventType type,void *arg,
		uint8_t *w,size_t l
	){
		if (type == WS_EVT_CONNECT) {
		} else if (type == WS_EVT_DISCONNECT) {
		} else if (type == WS_EVT_ERROR) {
		} else if (type == WS_EVT_PONG) {
		} else if (type == WS_EVT_DATA) {
			escpos.write(w,l);
			svr->binaryAll(w,l);
			// AwsFrameInfo *info = (AwsFrameInfo *)arg;
			// if (info->final && info->index == 0 && info->len == len) {
			// }
		}
	});
	svr.addHandler(&ws);
	// svr.onNotFound([](AsyncWebServerRequest *r){r->redirect(PUB_DIR);});
	// svr.serveStatic(PUB_DIR,FSYS,PUB_DIR).setDefaultFile("index.html").setTryGzipFirst(false);
	// svr.serveStatic(PVT_DIR,FSYS,PVT_DIR).setDefaultFile("index.html").setTryGzipFirst(false).setAuthentication(NAME,PASS,AsyncAuthType::AUTH_BASIC);
	svr.onNotFound([](AsyncWebServerRequest *req){
			if(req->url().startsWith(PUB_DIR)&&fs_send(req))return;
			if(req->url().startsWith(PVT_DIR)){
				if(!req->authenticate(NAME,PASS))return req->requestAuthentication();
				else if(fs_send(req))return;
			}
			req->redirect(PUB_DIR);
	});
	svr.begin();
}
void loop(){
	ArduinoOTA.handle();
	while(Serial.available()){
		uint8_t x=Serial.read();
		Serial.write(x);
		escpos.write(x);
	}
	// uint8_t x=((!digitalRead(BTNA))<<1)|(!digitalRead(BTNB));
	// if(x!=btn){
	// 	if(x&0b10)escpos.write(P_SELF);
	// 	btn=x;
	// }
	neopixelWrite(NPDI,
		(sin(millis()/1000.    )*.5+.5)*16.,
		(sin(millis()/1000.+2.1)*.5+.5)*16.,
		(sin(millis()/1000.+4.2)*.5+.5)*16.
	);
	delay(10);
}
