typedef struct{
	uint16_t i;
	uint32_t x;
}u1632_t;

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
uint8_t drawFont(uint16_t cp,uint8_t o){
	uint32_t x=ftx(cp);
	if(!x)return 0;
	font.seek(x&0xffffff);
	x=x>>24;
	uint8_t
		w=(x>>4)+1,
		h=(x&15)+1;
	font.read(buf+o*2,(w*h+7)/8);
	return h;
}

