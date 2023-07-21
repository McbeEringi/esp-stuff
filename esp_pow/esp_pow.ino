#include <Arduino.h>
#include <Ticker.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <ArduinoOTA.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "SparkFunBME280.h"
#include "util.h"

Ticker tickerbme;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
BME280 sensor;
const IPAddress ip(192,168,1,1);
const IPAddress subnet(255,255,255,0);
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
const int PWM_MAX=pow(2,PWM_BIT),
	ZPAD=1+(int)log10(PWM_MAX),
	ZPAD2=1+(int)log10(PWM_MAX*2);
float bme[3],flake[FLAKE_NUM][5];
int disp=0,btn[3]={0,0,0},pwm[2]={0,0},crs=0;
unsigned long flag;

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len){
	switch(type){
		case WS_EVT_CONNECT:
			Serial.printf("ws[%u] connect: %s\n", client->id(), client->remoteIP().toString().c_str());
			client->printf(
				"{\"purpose\":\"init\",\"cid\":%u,\"cip\":\"%s\",\"max\":%u,\"zpad\":%u,\"zpad2\":%u,\"disp\":%u,\"pwm0\":%d,\"pwm1\":%d,\"pwm2\":%d}",
				client->id(), client->remoteIP().toString().c_str(), PWM_MAX, ZPAD, ZPAD2, disp, pwm[0], pwm[1]
			);
			//client->ping();
			break;
		case WS_EVT_DISCONNECT:Serial.printf("ws[%u] disconnect\n", client->id());break;
		case WS_EVT_ERROR:Serial.printf("ws[%u] error(%u): %s\n", client->id(), *((uint16_t*)arg), (char*)data);break;
		case WS_EVT_PONG:Serial.printf("ws[%u] pong\n", client->id());break;
		case WS_EVT_DATA:
			{
				AwsFrameInfo *info=(AwsFrameInfo*)arg;
				if(info->final && info->index==0 && info->len==len && info->opcode==WS_TEXT){
					data[len]=0;
					String str=(char*)data;
					Serial.printf("ws[%u] text-msg[%llu]: %s\n", client->id(), info->len, str.c_str());
					ws.printfAll("{\"purpose\":\"pong\",\"pong\":\"%s\"}",str.c_str());
					if(str.startsWith("DISP")){
						disp=str.substring(4,5).toInt();
						Serial.printf("disp: %u\n",disp);
						ws.printfAll("{\"purpose\":\"check\",\"disp\":%u}",disp);
					}
					else if(str.startsWith("PWM")){
						int tmp=str.substring(3,4).toInt();
						pwm[tmp]=str.substring(4,4+ZPAD2).toInt()-PWM_MAX;
						Serial.printf("pwm%d: %d\n",tmp,pwm[tmp]);
						ws.printfAll("{\"purpose\":\"check\",\"pwm%d\":%d}", tmp, pwm[tmp]);
					}
				}
			}
			break;
	}
}
void flag1(){flag=1;}
float ipp(int a){return (float)(a*100)/PWM_MAX;}

void drawFlakes(){
	for(int i=0;i<FLAKE_NUM;i++){
		int j=flake[i][4];
		display.drawBitmap(flake[i][0], flake[i][1], flake_bmp[j], flake_s[j], flake_s[j], SSD1306_WHITE);
		flake[i][0]+=flake[i][2]*.2;if(flake[i][0]<=0.){flake[i][0]=-flake[i][0];flake[i][2]=-flake[i][2];}else if(flake[i][0]>=SCREEN_WIDTH-flake_s[j]){flake[i][0]=(SCREEN_WIDTH-flake_s[j])*2-flake[i][0];flake[i][2]=-flake[i][2];}
		flake[i][1]+=flake[i][3]*.2;if(flake[i][1]<=0.){flake[i][1]=-flake[i][1];flake[i][3]=-flake[i][3];}else if(flake[i][1]>=SCREEN_HEIGHT-flake_s[j]){flake[i][1]=(SCREEN_HEIGHT-flake_s[j])*2-flake[i][1];flake[i][3]=-flake[i][3];}
	}
}
void setup(){
	Serial.begin(115200);
	Wire.begin();
  sensor.setI2CAddress(0x76);
	sensor.beginI2C();
	display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS);
	display.clearDisplay();
	display.setTextSize(2);
	display.setTextColor(SSD1306_WHITE);
	display.drawBitmap(0, 16, esp_logo, 128, 32, SSD1306_WHITE);
	display.display();
	ledcSetup(I01PWM,PWM_FREQ,PWM_BIT);ledcAttachPin(I01,I01PWM);
	ledcSetup(I02PWM,PWM_FREQ,PWM_BIT);ledcAttachPin(I02,I02PWM);
	ledcSetup(I11PWM,PWM_FREQ,PWM_BIT);ledcAttachPin(I11,I11PWM);
	ledcSetup(I12PWM,PWM_FREQ,PWM_BIT);ledcAttachPin(I12,I12PWM);
	pinMode(BTNG, OUTPUT);digitalWrite(BTNG,LOW);
	pinMode(BTN0, INPUT_PULLUP);
	pinMode(BTN1, INPUT_PULLUP);
	pinMode(BTN2, INPUT_PULLUP);
	for(int i=0;i<FLAKE_NUM;i++){
		flake[i][0]=random(SCREEN_WIDTH);//posX
		flake[i][1]=random(SCREEN_HEIGHT);//posY
		flake[i][3]=random(628)/100.;//theta
		flake[i][2]=cos(flake[i][3]);//velX
		flake[i][3]=sin(flake[i][3]);//velY
		flake[i][4]=floor(random(FLAKE_MAP));//map
	}
	delay(1000);

	WiFi.mode(WIFI_AP_STA);
	WiFi.softAP(ssid,pass);
	delay(100);
	WiFi.softAPConfig(ip,ip,subnet);
	Serial.printf("SSID: %s\nPASS: %s\nAPIP: %s\n",ssid,pass,WiFi.softAPIP().toString().c_str());

	ws.onEvent(onEvent);
	server.addHandler(&ws);
	server.on("/",HTTP_GET,[](AsyncWebServerRequest *request){
		request->send_P(200,"text/html",html);
	});
	server.begin();
	Serial.println("server started");

	ArduinoOTA
		.setPassword("mcbeeringi")
		.onStart([](){Serial.printf("Start updating %s\n",ArduinoOTA.getCommand()==U_FLASH?"sketch":"filesystem");})
		.onEnd([](){Serial.println("\nEnd");})
		.onProgress([](unsigned int progress, unsigned int total){Serial.printf("Progress: %u%%\r",(progress/(total/100)));})
		.onError([](ota_error_t error){Serial.printf("Error[%u]", error);})
		.begin();

	flag=millis();
	while(millis()-flag<3000){
		display.clearDisplay();
    display.drawBitmap(0, 0, logo_bmp, 64, 64, SSD1306_WHITE);
    display.drawBitmap(64, 0, logo_txt, 64, 64, SSD1306_WHITE);
		drawFlakes();
		display.display();
	}
	tickerbme.attach_ms(500,flag1);
}

void loop(){
	if(flag>0){
		bme[0]=sensor.readTempC();
		bme[1]=sensor.readFloatHumidity();
		bme[2]=sensor.readFloatPressure()/100.;
    Serial.printf("°C:%f,hum%%:%f,atm%%:%f,pwm0%%:%f,pwm1%%:%f\n",bme[0],bme[1],bme[2]/10.1325,ipp(pwm[0]),ipp(pwm[1]));
		ws.printfAll("{\"purpose\":\"info\",\"bme\":[%.2f,%.2f,%.2f]}",bme[0],bme[1],bme[2]);
		flag=0;
	}
	ArduinoOTA.handle();
	ws.cleanupClients();
	ledcWrite(I01PWM,min(PWM_MAX+pwm[0],PWM_MAX));ledcWrite(I02PWM,min(PWM_MAX-pwm[0],PWM_MAX));
	ledcWrite(I11PWM,min(PWM_MAX+pwm[1],PWM_MAX));ledcWrite(I12PWM,min(PWM_MAX-pwm[1],PWM_MAX));// short
	//ledcWrite(I11PWM,max(pwm[1],0));ledcWrite(I12PWM,max(-pwm[1],0));// HI-Z
	if(digitalRead(BTN0)==LOW)btn[0]++;else btn[0]=0;
	if(digitalRead(BTN1)==LOW)btn[1]++;else btn[1]=0;
	if(digitalRead(BTN2)==LOW)btn[2]++;else btn[2]=0;

	display.clearDisplay();
	switch(disp){
		case 0:
			display.setCursor(0,8);
			display.printf("%.2f\n%.2f\n%.2f",bme[0],bme[1],bme[2]);
      display.drawBitmap(112, 8, cphpa, 16, 48, SSD1306_WHITE);
			if(btn[1]==2)disp=1;
			crs=0;
			break;
		case 1:
			display.setCursor(0,8);
			display.printf("%6.1f %%%s\n\n%6.1f %%%s",ipp(pwm[0]),crs==1?"<":"",ipp(pwm[1]),crs==2?"<":"");
			if(btn[1]<2){
				if(btn[0]==2)crs=(crs+1)%3;
				if(btn[2]==2)crs=(crs+2)%3;
			}else if(btn[1]==2){
				if(crs==0)disp=0;
			}else{
				if(btn[0]>=2&&btn[2]>=2)pwm[crs-1]=0;
				else{
					bool tmp=pwm[crs-1]==0;
					if((btn[0]>=2&&!tmp)||(btn[0]==2&&tmp))pwm[crs-1]=min(max(pwm[crs-1]-1,-PWM_MAX),PWM_MAX);
					if((btn[2]>=2&&!tmp)||(btn[2]==2&&tmp))pwm[crs-1]=min(max(pwm[crs-1]+1,-PWM_MAX),PWM_MAX);
				}
			}

			break;
	}
	drawFlakes();
	display.display();
	//display.dim(true);
}
