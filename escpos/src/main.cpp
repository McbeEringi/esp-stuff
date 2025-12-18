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
#define CTS 10

#define P_INIT "\x1b\x40"
#define P_SELF "\x1b##SELF"

#define BUFLEN 8

HardwareSerial escpos(0);

static AsyncWebServer svr(80);
static AsyncWebSocketMessageHandler wsh;
static AsyncWebSocket ws("/ws",wsh.eventHandler());

bool isAPmode=false;
uint8_t buf[BUFLEN];

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
	pinMode(BTNA,INPUT_PULLUP);
	pinMode(BTNB,INPUT_PULLUP);
	pinMode(CTS,INPUT);
	isAPmode=digitalRead(BTNA)==LOW&&digitalRead(BTNB)==LOW;
	if(isAPmode)neopixelWrite(NPDI,8,8,8);
	else neopixelWrite(NPDI,16,0,0);
	FSYS.begin();
	Serial.begin(115200);
	escpos.begin(115200);
	escpos.write(P_INIT);
	attachInterruptArg(BTNA,isr_btn_a,&btn,FALLING);
	attachInterruptArg(BTNB,isr_btn_b,&btn,FALLING);
	delay(1000);

	isAPmode=wlan(
		isAPmode,NPDI,
		FSYS,WIFI_CFG,
		NAME,PASS
	);

	for(uint8_t i=8;i;--i){
		if(isAPmode)neopixelWrite(NPDI,0,0,32);
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

	wsh.onMessage([](
		AsyncWebSocket *svr,AsyncWebSocketClient *cli,
		const uint8_t *w,size_t l
	){
		if(1024<l)cli->text("Too long! Should be 1024 or less.\n");
		else{while(digitalRead(CTS))delay(50);escpos.write(w,l);cli->text("OK");}
	});
	svr.addHandler(&ws);
	svr.onNotFound([](AsyncWebServerRequest *req){
		if(req->url().startsWith(PUB_DIR)&&fs_send(req))return;
		if(req->url().startsWith(PVT_DIR)){
			if(!req->authenticate(NAME,PASS))return req->requestAuthentication(AsyncAuthType::AUTH_BASIC);
			else if(fs_send(req))return;
		}
		req->redirect(PUB_DIR);
	});
	svr.begin();
}
void loop(){
	ArduinoOTA.handle();
	while(!digitalRead(CTS)&&Serial.available()){
		uint8_t l=min(BUFLEN,Serial.available());
		Serial.readBytes(buf,l);
		Serial.write(buf,l);
		escpos.write(buf,l);
	}
	while(escpos.available()){
		uint8_t l=min(BUFLEN,escpos.available());
		escpos.readBytes(buf,l);
		Serial.write(buf,l);
		ws.binaryAll(buf,l);
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
