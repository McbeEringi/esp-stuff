const uint32_t smask=_BV(S0)|_BV(S1)|_BV(S2);

TaskHandle_t *h_flush;

void flush(void *_){
	while(1){
		for(uint8_t j=0;j<16;++j){
			for(uint8_t k=0;k<NUM_PANEL;++k){
				for(uint8_t i=0;i<16;++i){
					uint8_t
					// 	o=(NUM_PANEL*j+(NUM_PANEL-1-k))*4+(1-i/8),
					// 	_i=i%8;
						o=(NUM_PANEL-1-k)*64+(16-1-i)*2+(1-j/8),
						_i=j%8;

					uint32_t
						sdata=(
							(i==j)<<S0
						)|(
							((buf[o   ]>>_i)&1)<<S1
						)|(
						// 	((buf[o+2]>>_i)&1)<<S2
							((buf[o+32]>>_i)&1)<<S2
						);

					GPIO.out_w1tc.val=~sdata&smask;
					GPIO.out_w1ts.val= sdata&smask;

					GPIO.out_w1ts.val=_BV(SCK);
					GPIO.out_w1tc.val=_BV(SCK);
				}
			}
			GPIO.out_w1tc.val=_BV(RCK);
			GPIO.out_w1ts.val=_BV(RCK);
			delayMicroseconds(500);
		}
	}
}

void dispBri(uint8_t x){ledcWrite(OE_CH,0xff-x);}
void dispInit(){
	pinMode(S0,OUTPUT);
	pinMode(S1,OUTPUT);
	pinMode(S2,OUTPUT);
	pinMode(SCK,OUTPUT);
	pinMode(RCK,OUTPUT);
	// GPIO.enable_w1ts.val=smask|_BV(SCK)|_BV(RCK);
	GPIO.out_w1ts.val=_BV(RCK);

	ledcSetup(OE_CH,65536,LEDC_TIMER_8_BIT);ledcAttachPin(OE,OE_CH);
	dispBri(0x80);
	xTaskCreateUniversal(flush,"flush",512,NULL,1,h_flush,CONFIG_ARDUINO_RUNNING_CORE);
}

void scrollX(bool inv){
	uint8_t x[2];
	if(inv){memcpy(x,buf+BUF_SIZE-2,2);memmove(buf+2,buf,BUF_SIZE-2);memcpy(buf,x,2);}
	else{memcpy(x,buf,2);memmove(buf,buf+2,BUF_SIZE-2);memcpy(buf+BUF_SIZE-2,x,2);}
}
void scrollX(){scrollX(false);}
void scrollY(bool inv){
	uint8_t w=NUM_PANEL*32,h=2;
	for(uint8_t i=0;i<w;++i){
		uint8_t x=0;
		if(inv){
			for(uint8_t j=h-1,_;j<h;--j){
				_=buf[i*h+j]>>7;
				buf[i*h+j]=(buf[i*h+j]<<1)|x;
				x=_;
			}
			buf[i*h+h-1]|=x;
		}else{
			for(uint8_t j=0,_;j<h;++j){
				_=buf[i*h+j]&1;
				buf[i*h+j]=(x<<7)|(buf[i*h+j]>>1);
				x=_;
			}
			buf[i*h]|=x<<7;
		}
	}
}
void scrollY(){scrollY(false);}

