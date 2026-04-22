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
			(x=>(
				w=(x>>4)+1,
				h=(x&15)+1
			))((await src.slice(x,++x).bytes())[0]),
			{
				w,h,
				bin:await src.slice(x,x+Math.ceil(w*h/8)).bytes()
			}
		),
		{d}
	))(
		(await src.slice(
			3,
			3+(await src.slice(0,3).bytes()).reduce((a,x,i)=>a|(x<<(8*i)),0)
		).bytes()).reduce((a,x,i)=>([
			_=>a.i=x,_=>a.i|=x<<8,
			_=>a.x=x,_=>a.x|=x<<8,
			_=>(
				a.x|=x<<16,
				a.a[String.fromCodePoint(a.i)]=a.x
			)
		][i%5](),a),{a:{}}).a
	)
);

export{size,family,reader};
