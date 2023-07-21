// 2022 SAZANKA Robotics outreach

#include <Arduino.h>
#include <Wire.h>
#include "util.h"

//ライブラリマネージャからインストール
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "bsec.h"

/////グローバル
Bsec iaqSensor;
Adafruit_SSD1306 display(128,64,&Wire,-1);//width,height
String s;
float contrast=1.;

void iaqerr(){
	if(iaqSensor.status!=BSEC_OK){display.printf(iaqSensor.status<BSEC_OK?"BSEC err[%d]\n":"BSEC warn[%d]\n",iaqSensor.status);display.display();delay(5000);}
	if(iaqSensor.bme680Status!=BME680_OK){display.printf(iaqSensor.bme680Status<BME680_OK?"BME680 err[%d]\n":"BME680 warn[%d]\n",iaqSensor.bme680Status);display.display();delay(5000);}
}
//float mix(float a,float b,float x){return a*(1-x)+b*x;}


void setup(){
	/////接続
	Serial.begin(115200);while(!Serial)delay(10);//シリアル接続開始 開始待機
	Wire.begin();//i2c接続開始
	display.begin(SSD1306_SWITCHCAPVCC, 0x3C);//動作モードとアドレスを指定してディスプレイ接続開始

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

	display.printf("Done!\n");display.display();
	delay(500);
}
void loop(){
	int lx=analogRead(39);//uint_16 [ 0 ~ 4096 ]
  Serial.println(lx);
	contrast+=(min(lx,64)/64.-contrast)*.1;//差の0.1倍を帰還
	display.ssd1306_command(0x81);display.ssd1306_command(uint8_t(contrast*254.)+1);//明るさ

	if(iaqSensor.run()){//新規データがあったら更新 一度に取得すると処理落ちする
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
