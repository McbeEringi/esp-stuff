#include <Arduino.h>
#include <LittleFS.h>
#include <ESPAsyncWebSrv.h>
#include <ESPmDNS.h>
#define PIN 8
#define NAME "mysvr"

AsyncWebServer svr(80);
AsyncWebSocket ws("/ws");
float col[3]={};

void onWS(AsyncWebSocket *ws,AsyncWebSocketClient *client,AwsEventType type,void *arg,uint8_t *data,size_t len){
	if(type==WS_EVT_DATA){
		AwsFrameInfo *info=(AwsFrameInfo*)arg;
		if(info->final&&info->index==0&&info->len==len){
			for(uint8_t i=0;i<3;i++)col[i]=*(float*)(data+(i<<2));
			neopixelWrite(PIN,col[0],col[1],col[2]);
			Serial.printf("%8.3f,%8.3f,%8.3f\n",col[0],col[1],col[2]);
		}
	}
}

void setup(){
	Serial.begin(9600);LittleFS.begin();neopixelWrite(PIN,16,0,0);
	delay(1000);
	WiFi.softAP(NAME);Serial.printf("WiFi");
	neopixelWrite(PIN,0,16,0);
	MDNS.begin(NAME);
	ws.onEvent(onWS);svr.addHandler(&ws);
	svr.onNotFound([](AsyncWebServerRequest *request){request->redirect("/");});
	svr.serveStatic("/",LittleFS,"/").setDefaultFile("index.html");
	svr.begin();
}
void loop(){
	Serial.printf("SSID: %s  IP: %s\n",WiFi.SSID().c_str(),WiFi.localIP().toString().c_str());
	delay(2000);
}
