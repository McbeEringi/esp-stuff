#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "bsec.h"
#include <WiFi.h>
#include <ArduinoOTA.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "util.h"

Bsec iaqSensor;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

int pwm[3];
float t[3];uint8_t o[2],l[4];
String s;

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len){
	switch(type){
		case WS_EVT_CONNECT:
			Serial.printf("ws[%u] connect: %s\n", client->id(), client->remoteIP().toString().c_str());
			client->printf(
				"{\"purpose\":\"init\",\"cid\":%u,\"cip\":\"%s\",\"max\":%u,\"zpad\":%u}",
				client->id(), client->remoteIP().toString().c_str(), PWM_MAX, ZPAD
			);
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
          int cmd=str.substring(0,1).toInt();
          switch(cmd){
            case 0:
              pwm[0]=str.substring(1       ,1+ZPAD  ).toInt();
              pwm[1]=str.substring(1+ZPAD  ,1+ZPAD*2).toInt();
              pwm[2]=str.substring(1+ZPAD*2,1+ZPAD*3).toInt();
              ledcWrite(R_PWM,pwm[0]);
              ledcWrite(G_PWM,pwm[1]);
              ledcWrite(B_PWM,pwm[2]);
              break;
          }
				}
			}
			break;
	}
}
void iaqerr(){
  if(iaqSensor.status!=BSEC_OK){display.printf(iaqSensor.status<BSEC_OK?"BSEC err[%d]\n":"BSEC warn[%d]\n",iaqSensor.status);display.display();delay(5000);}
  if(iaqSensor.bme680Status!=BME680_OK){display.printf(iaqSensor.bme680Status<BME680_OK?"BME680 err[%d]\n":"BME680 warn[%d]\n",iaqSensor.bme680Status);display.display();delay(5000);}
}
//float fabs(float x){return x>0?x:-x;}

void setup(){
  ledcSetup(R_PWM,PWM_FREQ,PWM_BIT);ledcAttachPin(R_PIN,R_PWM);
  ledcSetup(G_PWM,PWM_FREQ,PWM_BIT);ledcAttachPin(G_PIN,G_PWM);
  ledcSetup(B_PWM,PWM_FREQ,PWM_BIT);ledcAttachPin(B_PIN,B_PWM);

	Serial.begin(115200);
  while(!Serial)delay(10);
  Wire.begin();
  
	display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS);
	display.clearDisplay();
	//display.setTextSize(2);
	display.setTextColor(SSD1306_WHITE);
	display.setTextWrap(false);
	display.drawBitmap(0, 16, esp_logo, 128, 32, SSD1306_WHITE);
	display.display();
	delay(1000);

  display.clearDisplay();
  display.drawBitmap(0, 0, logo_bmp, 64, 64, SSD1306_WHITE);
  display.drawBitmap(64, 0, logo_txt, 64, 64, SSD1306_WHITE);
  display.display();
  delay(1000);

  display.clearDisplay();
  display.drawBitmap(64, 0, logo_bmp, 64, 64, SSD1306_WHITE);
  display.setCursor(0,0);
  display.display();

  display.printf("WIFI");display.display();
  WiFi.begin("SSID", "PASS");
  while(WiFi.status()!=WL_CONNECTED)delay(100);
  display.printf(" ready.\n");display.display();

  display.printf("Server");display.display();
	ws.onEvent(onEvent);
	server.addHandler(&ws);
	server.on("/",HTTP_GET,[](AsyncWebServerRequest *request){
    if(!request->authenticate("admin","12345678"))return request->requestAuthentication();
		request->send_P(200,"text/html",html);
	});
  server.on("/logout",HTTP_GET,[](AsyncWebServerRequest *request){
    request->send(401);
  });
	server.begin();
  display.printf(" ready.\n");display.display();
  delay(200);

  display.printf("OTA");display.display();
	ArduinoOTA
		.setPassword("mcbeeringi")
		.onStart([](){display.clearDisplay();display.setCursor(0,0);display.printf("OTA update started.\n");display.display();})
		.onEnd([](){display.printf("Done!");display.display();})
		.onProgress([](unsigned int progress, unsigned int total){display.clearDisplay();display.setCursor(0,0);display.printf("Updating %s...\n%u/%u\n%5.1f%%\n",ArduinoOTA.getCommand()==U_FLASH?"FLASH":"SPIFFS",progress,total,float(progress)/float(total)*100.);display.display();})
		.onError([](ota_error_t err){display.printf("Err[%u]",err);display.display();delay(5000);})
		.begin();
  display.printf(" ready.\n");display.display();
  delay(200);

  display.printf("Sensor");display.display();
  iaqSensor.begin(BME680_I2C_ADDR_SECONDARY, Wire);
  iaqerr();
  bsec_virtual_sensor_t sensorList[6]={
    BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_TEMPERATURE,
    BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_HUMIDITY,
    BSEC_OUTPUT_RAW_PRESSURE,
    BSEC_OUTPUT_STATIC_IAQ,
    BSEC_OUTPUT_CO2_EQUIVALENT,
    BSEC_OUTPUT_BREATH_VOC_EQUIVALENT,
  };
  iaqSensor.updateSubscription(sensorList,6,BSEC_SAMPLE_RATE_LP);
  iaqerr();
  display.printf(" ready.\n");display.display();
  delay(200);

  display.printf("NTP");display.display();
  configTzTime("JST-9","ntp.nict.jp");
  struct tm tm;
  getLocalTime(&tm);
  display.printf(" ready.\n");display.display();
  delay(200);

  display.printf("Setup");display.display();
  display.ssd1306_command(0xd9);display.ssd1306_command(0x11);//precharge
  display.ssd1306_command(0xdb);display.ssd1306_command(0x20);//Vcomh
  display.printf(" done!\n");display.display();
  delay(500);
}

void loop(){
	ArduinoOTA.handle();
	ws.cleanupClients();

  if(iaqSensor.run()){
    s="{\"temperature\":"+String(iaqSensor.temperature);
    s+=",\"humidity\":"+String(iaqSensor.humidity);
    s+=",\"pressure\":"+String(iaqSensor.pressure/100.);
    s+=",\"sIaqAccuracy\":"+String(iaqSensor.staticIaqAccuracy);//0~3
    s+=",\"sIaq\":"+String(iaqSensor.staticIaq);
    s+=",\"co2Equivalent\":"+String(iaqSensor.co2Equivalent);
    s+=",\"breathVocEquivalent\":"+String(iaqSensor.breathVocEquivalent)+"}";
    ws.printfAll("{\"purpose\":\"info\",\"bme\":%s}",s.c_str());
  }

	display.clearDisplay();
  struct timeval tv;struct tm tm;gettimeofday(&tv,NULL);localtime_r(&tv.tv_sec,&tm);//https://8ttyan.hatenablog.com/entry/2015/02/03/003428

  o[0]=32;o[1]=32;l[3]=30;
	l[0]=16;t[0]=(tm.tm_hour%12+tm.tm_min/60.)/6.*PI;//hour
	l[1]=24;t[1]=(tm.tm_min+tm.tm_sec/60.)/30.*PI;//min
	l[2]=24;t[2]=(tm.tm_sec+tv.tv_usec/1000000.)/30.*PI;//sec
	for(uint8_t i=0;i<3;i++)display.drawLine(o[0],o[1],o[0]+sin(t[i])*l[i],o[1]-cos(t[i])*l[i],SSD1306_WHITE);
	for(uint8_t i=0;i<12;i++)display.drawPixel(o[0]+sin(i/6.*PI)*l[3],o[1]-cos(i/6.*PI)*l[3],SSD1306_WHITE);

  o[0]=80;o[1]=16;l[3]=14;
  l[0]=12;t[0]=iaqSensor.temperature/20.*PI;//temp 0~40
  l[1]=10;t[1]=iaqSensor.humidity/50.*PI;//hum 0~100
  for(uint8_t i=0;i<2;i++)display.drawLine(o[0],o[1],o[0]-sin(t[i])*l[i],o[1]+cos(t[i])*l[i],SSD1306_WHITE);
  for(uint8_t i=0;i<8;i++)display.drawPixel(o[0]-sin(i/4.*PI)*l[3],o[1]+cos(i/4.*PI)*l[3],SSD1306_WHITE);

  o[0]=112;o[1]=16;l[3]=14;
  l[0]=12;t[0]=(iaqSensor.co2Equivalent-400.)/600.*PI;//co2 400~1600
  l[1]=10;t[1]=iaqSensor.staticIaq/150.*PI;//sAIQ 0~300
  for(uint8_t i=0;i<2;i++)display.drawLine(o[0],o[1],o[0]-sin(t[i])*l[i],o[1]+cos(t[i])*l[i],SSD1306_WHITE);
  for(uint8_t i=0;i<12;i++)display.drawPixel(o[0]-sin(i/6.*PI)*l[3],o[1]+cos(i/6.*PI)*l[3],SSD1306_WHITE);

  display.setCursor(64,32);display.printf("%02d/%02d %s %s",tm.tm_mon+1,tm.tm_mday,week[tm.tm_wday],pwm[0]==0&&pwm[1]==0&&pwm[2]==0?"":"*");
  switch(tm.tm_sec/6){
    case 0:case 9:{
      display.setCursor(64,44);display.printf(tv.tv_usec<500000?"%02u:%02u:%02u":"%02u %02u %02u",tm.tm_hour,tm.tm_min,tm.tm_sec);
      display.setCursor(64,56);display.printf("Acc %u/3",iaqSensor.staticIaqAccuracy);
      break;
    }
    case 1:{
      display.setCursor(64,44);display.printf("Temp");
      display.setCursor(64,56);display.printf("%5.2f C",iaqSensor.temperature);
      break;
    }
    case 2:{
      display.setCursor(64,44);display.printf("Hum");
      display.setCursor(64,56);display.printf("%5.2f %%",iaqSensor.humidity);
      break;
    }
    case 3:{
      display.setCursor(64,44);display.printf("Atm");
      display.setCursor(64,56);display.printf("%6.1f hPa",iaqSensor.pressure/100.);
      break;
    }
    case 4:case 7:{
      display.setCursor(64,44);display.printf("sIAQ");float tmp=iaqSensor.staticIaq;
      display.setCursor(64,56);display.printf("%-5.1f %u/6",tmp,tmp<51?6:tmp<101?5:tmp<151?4:tmp<201?3:tmp<251?2:tmp<351?1:0);
      break;
    }
    case 5:case 8:{
      display.setCursor(64,44);display.printf("CO2");
      display.setCursor(64,56);display.printf("%6.1f ppm",iaqSensor.co2Equivalent);
      break;
    }
    case 6:{
      display.setCursor(64,44);display.printf("bVOC");
      display.setCursor(64,56);display.printf("%6.2f ppm",iaqSensor.breathVocEquivalent);
      break;
    }
  }

	display.display();

  t[0]=fabs(tm.tm_hour+tm.tm_min/60.-12.);
  t[0]=1.-(constrain(t[0],4.,8.)/4.-1.);
  //t[0]=sin(tm.tm_sec+tv.tv_usec/1000000.)*.5+.5;
  display.ssd1306_command(0x81);display.ssd1306_command(uint8_t(t[0]*254.)+1);//contrast

  delay(50);
}
