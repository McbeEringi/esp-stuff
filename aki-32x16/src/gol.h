const uint8_t sum3[]={0,1,1,2,1,2,2,3};

void golInit(bool keep){
	rstcnt=0;
	dispBri(0x80);
	if(!keep)for(uint8_t i=0;i<BUF_SIZE;++i)buf[i]=random(0x100);
}
void golInit(){golInit(false);}
uint32_t gol(){
	uint8_t _buf[BUF_SIZE];
	uint32_t _hash=0;
	memcpy(_buf,buf,BUF_SIZE);
	for(uint8_t i=0;i<BUF_SIZE;++i){
		uint32_t tmp=0;
		uint8_t w=NUM_PANEL*32,h=2,x=i/h,y=i%h,a=0;
		for(uint8_t j=0;j<3;++j){
			uint8_t _x=(x+w-1+j)%w*h;
			tmp=tmp|((((_buf[_x+(y+h-1)%h]<<9)|(_buf[_x+y]<<1)|(_buf[_x+(y+1)%h]>>7))&0x3ff)<<(10*j));
		}
		for(uint8_t j=0;j<8;++j){
			uint8_t
				c=(_buf[i]>>j)&1,
				score=
					sum3[(tmp>>j)&0b111]+
					sum3[(tmp>>(j+10))&0b101]+
					sum3[(tmp>>(j+20))&0b111];

			c=c?(score<=1||4<=score)?0:c:(score==3)?1:c;
			a=a|c<<j;
		}
		buf[i]=a;
		_hash=((_hash<<5)|(_hash>>27))^((i<<8)|a);
	}
	return _hash;
}

