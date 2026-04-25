typedef struct{
	uint16_t i;
	uint32_t x;
}u1632_t;

typedef struct{
	uint32_t o;
	uint8_t w;
	uint8_t h;
	uint8_t l;
}bmprop_t;

File font;
uint16_t ftsize;
u1632_t *ft=NULL;

void fontInit(const char* path){
	{
		File s=SD.open(path);
		if(s)do{
			File d=FSYS.open(path);
			if(d){
				uint32_t shash,dhash;
				s.read((uint8_t*)&shash,4);s.seek(0);
				d.read((uint8_t*)&dhash,4);d.close();
				if(shash==dhash)break;
			}

			d=FSYS.open(path,FILE_WRITE);
			uint8_t buf[4096];
			size_t n;
			while((n=s.read(buf,4096))>0)d.write(buf,n);
			s.close();d.close();
		}while(0);
	}

	font=FSYS.open(path);
	if(!font)return;
	uint32_t size=0;
	font.seek(4);
	font.read((uint8_t*)&size,3);
	ftsize=size/6;
	if(ft)free(ft);
	ft=(u1632_t*)calloc(ftsize,sizeof(u1632_t));
	for(uint16_t i=0;i<ftsize;++i){
		font.read((uint8_t*)&ft[i].i,2);
		font.read((uint8_t*)&ft[i].x,4);
	}
}
uint32_t ftx(uint16_t cp){
	uint16_t L=1,R=ftsize,m;
	while(1){
		if(R<L)return 0;
		m=L+(R-L)/2;
		if(ft[m-1].i<cp)L=m+1;
		else if(ft[m-1].i>cp)R=m-1;
		else return ft[m-1].x;
	}
}
bmprop_t bmprop(uint32_t x){
	uint8_t
		s=x>>24,
		w=(s>>4)+1,
		h=(s&15)+1,
		l=(w*h+7)/8;

	return(bmprop_t){
		.o=x&0xffffff,
		.w=w,
		.h=h,
		.l=l
	};
}

uint8_t drawFont(uint16_t cp,uint8_t o){
	uint32_t x=ftx(cp);
	if(!x)return 0;

	bmprop_t p=bmprop(x);
	font.seek(p.o);
	font.read(buf+o*2,p.l);
	return p.h;
}

void scrollTxt(const char *path){
	File txt=SD.open(path);
	if(txt){
		uint16_t n=txt.available();
		auto read=[&txt,&n]()->uint8_t{uint8_t x;n-=txt.read(&x,1);return x;};
		uint32_t x;
		uint8_t _x;
		while(n){
			_x=read();
			if(_x>>7==0)x=_x;
			else if(_x>>5==0b00110)x=((_x&0x1f)<< 6)|((read()&0x3f)<< 0);
			else if(_x>>4==0b01110)x=((_x&0x0f)<<12)|((read()&0x3f)<< 6)|((read()&0x3f)<<0);
			else if(_x>>3==0b11110)x=((_x&0x07)<<18)|((read()&0x3f)<<12)|((read()&0x3f)<<6)|((read()&0x3f)<<0);
			x=ftx(x);
			if(x){
				bmprop_t p=bmprop(x);
				font.seek(p.o);
				uint8_t *a=(uint8_t*)malloc(p.l);
				font.read(a,p.l);

				for(uint8_t i=0;i<p.h;++i){
					scrollX();
					memcpy(buf+BUF_SIZE-2,a+i*2,2);
					delay(20);
				}
				free(a);
			}
		}
	}
	txt.close();
}
