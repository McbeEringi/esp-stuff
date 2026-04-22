#!/bin/env bun
import{png}from'@mcbeeringi/petit/png';

const
td=new TextDecoder(),
ffam=(x=>(console.log(`using "${x}"`),x))(
	await Bun.$`fc-match -f"%{family}" ${Bun.argv[2]}`.text()
),
src=Bun.file(`${ffam.replace(/\s/g,'')}.font`),

d=(await src.slice(
	3,
	3+(await src.slice(0,3).bytes()).reduce((a,x,i)=>a|(x<<(8*i)),0)
).bytes()).reduce((a,x,i)=>([
	_=>a.i=x,_=>a.i|=x<<8,
	_=>a.x=x,_=>a.x|=x<<8,
	_=>(
		a.x|=x<<16,
		a.a[String.fromCodePoint(a.i)]=a.x
	)
][i%5](),a),{a:{}}).a,
get=async(x,{
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
);

let w=process.stdin.isTTY?
	// https://github.com/yuru7/moralerspace
	'Lorem ipsum であのイーハトーヴォの世界が広がります':
	await Bun.stdin.text();

w=await w[Symbol.iterator]().reduce(async(a,x)=>(
	a=await a,
	x=await get(x),
	x&&(
		a.push(...Array(x.h)[Symbol.iterator]().map((_,i)=>(
			x.bin.slice(i*x.w/8,++i*x.w/8).reduce((a,x)=>a+x.toString(2).padStart(8,0),'')
		)))
	),
	a
),[]);

w=[...Array(w[0].length)[Symbol.iterator]().map((_,i)=>w.map(x=>x[w[0].length-1-i]).join(''))];
// console.log(w.join('\n'));

await Bun.write('test.png',png({data:[...w.join('')].map(x=>+x),width:w[0].length,height:w.length,palette:[0x222222,0xff0000]}).toBlob());
