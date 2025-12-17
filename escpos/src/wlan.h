#ifndef WLAN_H
#define WLAN_H

#include <WiFi.h>
#include <vector>

struct WiFiEntry{String ssid;String psk;};
typedef std::vector<WiFiEntry> WiFiEntries;
class WiFiDict{
	public:
		WiFiEntries a;
		WiFiDict(File& w){
			while(w.available()){
				String x=w.readStringUntil('\n');
				x.trim();
				if(!x.isEmpty()){
					int n=x.indexOf('\t');
					if(~n)a.push_back((WiFiEntry){x.substring(0,n),x.substring(n+1)});
				}
			}
		}
		const WiFiEntry* find(String w){
			for(const auto& x:a){if(x.ssid==w)return &x;}
			return nullptr;
		}
};

void wait_wifi_conn(){
	Serial.printf("connecting...\n");
	for(uint8_t i=10;i&&(WiFi.status()!=WL_CONNECTED);--i)delay(500);
}

bool wlan(bool APmode,uint8_t npdi,FS& fs,const char* dict,const char* ssid,const char* psk){
	bool isAPmode=false;
	if(!APmode){
		neopixelWrite(npdi,16,16,0);
		WiFi.begin();
		// WiFi.disconnect();
		wait_wifi_conn();

		if(WiFi.status()!=WL_CONNECTED){
			neopixelWrite(npdi,16,4,0);
			WiFi.disconnect();
			File f=fs.open(dict);
			WiFiDict w(f);

			Serial.printf("scanning...\n");
			uint8_t n=WiFi.scanNetworks();
			Serial.printf("scan done: n=%d\n",n);
			neopixelWrite(npdi,16,16,0);
			for(uint8_t i=0;i<n;++i){
				Serial.printf("ssid: %s\n",WiFi.SSID(i).c_str());
				const WiFiEntry* x=w.find(WiFi.SSID(i));
				if(x){
					Serial.printf("known ssid\n");
					WiFi.begin(x->ssid,x->psk);
					wait_wifi_conn();
					if(WiFi.status()==WL_CONNECTED)break;
				}
			}
			WiFi.scanDelete();
		}
		if(WiFi.status()==WL_CONNECTED){
			Serial.printf("connected!\n");
			WiFi.setAutoReconnect(true);
		}
	}

	if(APmode||WiFi.status()!=WL_CONNECTED){
		isAPmode=true;
		WiFi.softAP(ssid,psk);
		delay(100);// https://github.com/espressif/arduino-esp32/issues/985
		const IPAddress ip(192,168,1,1);// 192.168.0.1 not works
		const IPAddress subnet(255,255,255,0);
		WiFi.softAPConfig(ip,ip,subnet);
		Serial.printf("APmode!\n");
	}
	return isAPmode;
}

#endif
