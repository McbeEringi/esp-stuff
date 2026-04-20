#!/bin/env bun

const
w=await Bun.argv[2]?.[Symbol.iterator]().reduce(async(a,x)=>(
	a=await a,
	x=Bun.file(`font/${x.codePointAt().toString(16).padStart(4,0)}.pbm`),
	await x.exists()&&a.push((await x.bytes()).slice(9)),
	a
),[]);

w&&await Bun.write('a.pbm',new Blob([`P4 16 ${16*w.length}\n`,...w]));
await Bun.$`magick a.pbm a.png`;
