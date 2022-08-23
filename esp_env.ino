#include <Arduino.h>
#include <Wire.h>
//#include <WiFi.h>
//#include <ArduinoOTA.h>
#include "util.h"

//ライブラリマネージャからインストール
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "bsec.h"
#include <NimBLEDevice.h>

//手動インストール必須
//#include <AsyncTCP.h>//https://github.com/me-no-dev/AsyncTCP
//#include <ESPAsyncWebServer.h>//https://github.com/me-no-dev/ESPAsyncWebServer

/////グローバル
//struct vec2{uint8_t x;uint8_t y;};
Bsec iaqSensor;
Adafruit_SSD1306 display(128,64,&Wire,-1);//width,height
//vec2 o;
String s;
float contrast=1.;

NimBLEAddress *addr;
static NimBLEUUID CTSserviceUUID("1805");
static NimBLEUUID CTScharUUID("2a2b");
static NimBLEUUID BATTserviceUUID("180f");
static NimBLEUUID BATTcharUUID("2a19");
class svrCB: public NimBLEServerCallbacks{
	void onConnect(NimBLEServer *svr){
		addr=new NimBLEAddress(svr->getPeerInfo(0).getAddress());
		Serial.printf("svr con: %s\n", addr->toString().c_str());
		//svr->getAdvertising()->stop();
	};
	void onDisconnect(NimBLEServer *svr){Serial.printf("svr discon\n");};
};
class cliCB: public NimBLEClientCallbacks{
	void onConnect(NimBLEClient *cli){Serial.printf("cli con\n");};
	void onDisconnect(NimBLEClient *cli){Serial.printf("cli discon\n");};
};

void iaqerr(){
	if(iaqSensor.status!=BSEC_OK){display.printf(iaqSensor.status<BSEC_OK?"BSEC err[%d]\n":"BSEC warn[%d]\n",iaqSensor.status);display.display();delay(5000);}
	if(iaqSensor.bme680Status!=BME680_OK){display.printf(iaqSensor.bme680Status<BME680_OK?"BME680 err[%d]\n":"BME680 warn[%d]\n",iaqSensor.bme680Status);display.display();delay(5000);}
}
float mix(float a,float b,float x){return a*(1-x)+b*x;}


void setup(){
	/////接続
	Serial.begin(115200);while(!Serial)delay(10);//シリアル接続開始 開始待機
	Wire.begin();//i2c接続開始
	display.begin(SSD1306_SWITCHCAPVCC, 0x3C);//動作モードとアドレスを指定してディスプレイ接続開始
	pinMode(39,OUTPUT);digitalWrite(39,HIGH);
	pinMode(34,OUTPUT);digitalWrite(34,LOW);

	/////ディスプレイ初期設定
	display.ssd1306_command(0xd9);display.ssd1306_command(0x11);//precharge
	display.ssd1306_command(0xdb);display.ssd1306_command(0x20);//Vcomh
	//display.setTextSize(2);
	display.setRotation(2);
	display.setTextColor(SSD1306_WHITE);
	display.setTextWrap(false);

	/////ESPRESSIFスプラッシュ画面
	display.clearDisplay();//画面クリア
	display.drawBitmap(0,16,ESP_LOGO_TXT,128,32,SSD1306_WHITE);//画像 x,y,map,dx,dy,col
	display.display();//転送
	delay(1000);

	/////SAZANKAスプラッシュ画面
	display.clearDisplay();
	display.drawBitmap( 0, 0,SAZANKA_LOGO,64,64,SSD1306_WHITE);
	display.drawBitmap(64,24, SAZANKA_TXT,64,32,SSD1306_WHITE);
	display.display();
	delay(1000);

	/////起動シーケンス画面
	display.clearDisplay();
	display.drawBitmap(64,0,SAZANKA_LOGO,64,64,SSD1306_WHITE);
	display.setCursor(0,0);//カーソル原点移動
	display.printf("Loading...\n\n");display.display();delay(200);

	/////センサ起動
	display.printf("Sensor");display.display();
	iaqSensor.begin(BME680_I2C_ADDR_SECONDARY, Wire);iaqerr();//センサ起動 エラーチェック
	bsec_virtual_sensor_t sensorList[6]={//取得項目リスト
		BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_TEMPERATURE,
		BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_HUMIDITY,
		BSEC_OUTPUT_RAW_PRESSURE,
		BSEC_OUTPUT_STATIC_IAQ,
		BSEC_OUTPUT_CO2_EQUIVALENT,
		BSEC_OUTPUT_BREATH_VOC_EQUIVALENT,
	};
	iaqSensor.updateSubscription(sensorList,6,BSEC_SAMPLE_RATE_LP);iaqerr();//リスト登録 モード設定
	display.printf(" OK.\n");display.display();delay(200);

	//BLE時刻取得 CTS
	// NimBLEDevice::init("ESP_Clock");
	// NimBLEDevice::setSecurityAuth(BLE_SM_PAIR_AUTHREQ_SC);
	// NimBLEServer *svr=NimBLEDevice::createServer(); 
	// NimBLEClient *cli=NimBLEDevice::createClient(); 
	// NimBLEAdvertising *adv=svr->getAdvertising();
	// svr->setCallbacks(new svrCB());
	// cli->setClientCallbacks(new cliCB());
	// NimBLEService* battsrv = svr->createService(BATTserviceUUID);
  // NimBLECharacteristic* battchar = battsrv->createCharacteristic(BATTcharUUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);
	// battchar->setValue(10);
	// battsrv->start();
	// adv->addServiceUUID("1812");
	// adv->addServiceUUID(BATTserviceUUID);
	// adv->start();
	// while(addr==NULL)delay(100);
	// cli->connect(addr);

	// while(true){
	// 	std::string val=cli->getValue(CTSserviceUUID,CTScharUUID);
	// 	Serial.printf("CTS val: { length: %d, val: %s }\n",val.length(),val.c_str());
	// 	if(val.length()==10){
	// 		Serial.printf("%d-%02d-%02d %02d:%02d:%02d.%03d %d x%02x\n", val[1] << 8 | val[0], val[2], val[3], val[4], val[5], val[6], val[8]*1000/256, val[7], val[9]);
	// 		break;
	// 	}
	// 	delay(1000);
	// }


	display.printf("Done!\n");display.display();
	delay(500);
}
void loop(){
	int lx=analogRead(36);//uint_16 [ 0 ~ 4096 ]
	contrast+=(min(lx,64)/64.-contrast)*.1;//差の0.1倍を帰還
	display.ssd1306_command(0x81);display.ssd1306_command(uint8_t(contrast*254.)+1);//明るさ

	if(iaqSensor.run()){//新規データがあったら更新 クソコードに見えるが一度に取得すると処理落ちする
		s="{\ntemp: "+String(iaqSensor.temperature);
		s+=",\nhum: "+String(iaqSensor.humidity);
		s+=",\natm: "+String(iaqSensor.pressure/100.);
		s+=",\nacc: "+String(iaqSensor.staticIaqAccuracy);//0~3
		s+=",\nsIAQ: "+String(iaqSensor.staticIaq);
		s+=",\nCO2: "+String(iaqSensor.co2Equivalent);
		s+=",\nbVOC: "+String(iaqSensor.breathVocEquivalent)+"\n}";
		display.clearDisplay();
		display.setCursor(0,0);
		display.printf(s.c_str());
		display.drawBitmap(96,0,SAZANKA_TXT_V,32,64,SSD1306_WHITE);
		display.display();
	}

	delay(50);//だいたい20fps
}