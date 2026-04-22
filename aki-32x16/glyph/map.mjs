#!/bin/env bun
import{createCanvas,ImageData}from'@napi-rs/canvas';
import{size,family,reader}from'./util.mjs';

const
ffam=await family(Bun.argv[2]),
dst=`${ffam.ns}.map.png`,
get=await reader(ffam),

c=createCanvas(256*size,256*size),
ctx=c.getContext('2d');


console.log('map: render...\n');

ctx.fillStyle="#fff2";
[...Array(256)].forEach((_,i)=>(
	i=[i/16|0,i%16,1,1],
	((i[0]^i[1])&1)&&ctx.fillRect(...i.map(x=>x*16*size))
));

await Object.keys(get.d).reduce(async(a,x)=>(
	a=await a,
	x=Object.assign(await get(x),{i:x.codePointAt()}),
	console.log(`\x1b[1A${x.i.toString(16).padStart(4,0)}`),
	x.img=new ImageData(new Uint8ClampedArray(
		[...Array(x.w)].flatMap((_,j)=>(
			j=x.w-1-j,
			[...Array(x.h)].flatMap((b,i)=>(
				i=i*x.w+j,
				(x.bin[i/8|0]>>(7-i%8))&1?
					[255,255,255,255]:
					[128,128,128,255]
			))
		))
	),x.h,x.w),
	ctx.putImageData(
		x.img,
		(((x.i>>>8)&15)*16+(x.i&15))*size,
		((x.i>>>12)*16+((x.i>>>4)&15))*size
	),
	a
),0);

console.log('map: export...');
await Bun.write(dst,await c.encode('png'));
console.log('map: done!');
