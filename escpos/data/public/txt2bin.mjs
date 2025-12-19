const
GBK=class{
	td=new TextDecoder('gbk');
	r=[...[...Array(0x80)].map((_,i)=>[i]),...[
		[[0xa1,0xa9],[0xa1,0xfe]],
		[[0xb0,0xf7],[0xa1,0xfe]],
		[[0x81,0xa0],[0x40,0xfe]],
		[[0xaa,0xfe],[0x40,0xa0]],
		[[0xa8,0xa9],[0x40,0xa0]]
	].flatMap(([x,y])=>(
		[...Array(x[1]-x[0]+1)].flatMap((_,i)=>(
			i+=x[0],
			[...Array(y[1]-y[0]+1)].flatMap((_,j)=>(
				j+=y[0],
				j==0x7f?[]:[[i,j]]
			))
		))
	))];
	e=this.r.reduce((a,x)=>(
		a[this.td.decode(new Uint8Array(x))]=x,a
	),{});
	encode(x){return[...x].flatMap(x=>this.e[x]??(console.log('not found!',x),this.e['?']));}
	decode(x){return this.td.decode(new Uint8Array(x));}
},
gbk=new GBK(),
txt2bin=w=>gbk.encode(w);

export{GBK,gbk,txt2bin};
