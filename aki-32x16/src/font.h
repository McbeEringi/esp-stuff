typedef struct{
	uint16_t i;
	uint32_t x;
}u1632_t;

File font;
uint16_t ftsize;
u1632_t *ft=NULL;

void fontInit(const char* path){
	font=SD.open(path);
	if(!font)return;
	uint32_t size=0;
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
	uint16_t L=0,R=ftsize-1,m;
	while(1){
		if(R<L)return 0;
		m=L+(R-L)/2;
		if(ft[m].i<cp)L=m+1;
		else if(ft[m].i>cp)R=m-1;
		else return ft[m].x;
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

