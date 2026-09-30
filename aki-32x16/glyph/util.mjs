const
size=16,// max 16
family=async w=>(x=>(
	x=Object.assign(x,{inp:w||'system-default'}),
	console.log(`font: ${x.inp} => ${x.slice(0,3)}...`),
	x
))(
	(await Bun.$`fc-match -f"%{family}," -s ${w}`.text()).split(',')//.slice(0,4)
),
reader=async src=>(
	src=await(async f=>await f.exists()?f:(
		await Bun.$`./gen.mjs '${src.inp}'`,Bun.file(f.name)
	))(Bun.file(`${src.inp}.font`)),
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
		(await src.slice(// TODO: ハッシュは最後に
			3,
			3+(await src.slice(0,3).bytes()).reduce((a,x,i)=>a|(x<<(8*i)),0)
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
