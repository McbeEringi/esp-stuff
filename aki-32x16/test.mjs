#!/bin/env bun
import{png}from'@mcbeeringi/petit/png';
import{family,reader}from'./util.mjs';

const
ffam=await family(Bun.argv[2]),
dst=`${ffam.ns}.test.png`,
get=await reader(ffam);

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

await Bun.write(dst,png({data:[...w.join('')].map(x=>+x),width:w[0].length,height:w.length,palette:[0x222222,0xff0000]}).toBlob());
