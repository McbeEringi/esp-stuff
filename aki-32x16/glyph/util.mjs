const
size=16,// max 16
family=async w=>(x=>(console.log(`using "${x}"`),{raw:x,ns:x.replace(/\s/g,'')}))(
	(await Bun.$`fc-match -f"%{family}" ${w}`.text()).split(',').filter(x=>!x.match(/[^\w\s]/)).sort((a,b)=>b.length-a.length).pop()
),
reader=async src=>(
	src=await(async f=>await f.exists()?f:(
		await Bun.$`./gen.mjs '${src.raw}'`,Bun.file(f.name)
	))(Bun.file(`${src.ns}.font`)),
	(d=>Object.assign(
		async(x,{
			w,h
		}={})=>(x=d[x])&&(
			{
				w:x.w,h:x.h,
				bin:await src.slice(x.o,x.o+Math.ceil(x.w*x.h/8)).bytes()
			}
		),
		{d}
	))(
		(await src.slice(
			7,
			7+(await src.slice(4,7).bytes()).reduce((a,x,i)=>a|(x<<(8*i)),0)
		).bytes()).reduce((a,x,i)=>([
			_=>a.i=x,_=>a.i|=x<<8,
			_=>a.o=x,_=>a.o|=x<<8,_=>a.o|=x<<16,
			_=>(
				a.w=(x>>4)+1,
				a.h=(x&15)+1,
				a.a[String.fromCodePoint(a.i)]={o:a.o,w:a.w,h:a.h}
			)
		][i%6](),a),{a:{}}).a
	)
);

export{size,family,reader};
