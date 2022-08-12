#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "util.h"

/////グローバル
Adafruit_SSD1306 display(128,64,&Wire,-1);//width,height

void setup(){
	/////接続
	Serial.begin(115200);while(!Serial)delay(10);//シリアル接続開始&開始待機
	Wire.begin();//i2c接続開始
	display.begin(SSD1306_SWITCHCAPVCC, 0x3C);//動作モードとアドレスを指定してディスプレイ接続開始

	/////ディスプレイ初期設定
	//display.setTextSize(2);
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
}
void loop(){
	delay(1000);
}