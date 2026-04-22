#!/bin/env bun
import{png}from'@mcbeeringi/petit/png';

const td=new TextDecoder();
let w=Bun.argv[2]??'Lorem ipsum であのイーハトーヴォの世界が広がります';// https://github.com/yuru7/moralerspace

w=await w[Symbol.iterator]().reduce(async(a,x)=>(
	a=await a,
	x=Bun.file(`font/${x.codePointAt().toString(16).padStart(4,0)}.pbm`),
	await x.exists()&&(
		x=await x.bytes(),
		x=(([_,w,h])=>({
			bin:x.slice(9),
			w:+w,
			h:+h
		}))(td.decode(x.slice(0,9)).match(/\S+/g)),
		a.push(...Array(x.h)[Symbol.iterator]().map((_,i)=>(
			x.bin.slice(i*x.w/8,++i*x.w/8).reduce((a,x)=>a+x.toString(2).padStart(8,0),'')
		)))
	),
	a
),[]);

w=[...Array(w[0].length)[Symbol.iterator]().map((_,i)=>w.map(x=>x[w[0].length-1-i]).join(''))];
// console.log(w.join('\n'));

w={
	// pbm:new Blob([
	// 	`P4 ${w[0].length} ${w.length}\n`,
	// 	(w=>new Uint8Array(
	// 		Array(w.length/8)[Symbol.iterator]().map((_,i)=>+`0b${w.slice(i*8,++i*8)}`)
	// 	))(w.join(''))
	// ]),
	png:png({data:[...w.join('')].map(x=>+x),width:w[0].length,height:w.length,palette:[0x222222,0xff0000]}).toBlob()
};

// await Bun.write('a.pbm',w.pbm);
await Bun.write('a.png',w.png);

// await Bun.$`magick - b.png<${w.pbm}`
