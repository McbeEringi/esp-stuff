#!/bin/env bun
import{createCanvas}from'@napi-rs/canvas';
import{open}from'node:fs/promises';
import{size,family}from'./util.mjs';

const
ffam=await family(Bun.argv[2]),
dst=`${ffam.ns}.font`,
range_sjis=w=>(tds=>w.map(([s,e,o={}])=>Object.assign(
	[...Array(e-s+1)[Symbol.iterator]().map(
		(_,i)=>s+i
	).filter(
		e<=0xff?
		_=>1:
		x=>(x=x&0xff,0x40<=x&&x<=0xfc&&x!=0x7f)
	).map(
		x=>tds.decode(new Uint8Array(x<=0xff?[x]:[x>>8,x&0xff]))
	).filter(
		x=>x.length==1&&x!='\ufffd'
	)],
	o
)))(new TextDecoder('sjis')),
range=w=>w.map(([s,e,o={}])=>Object.assign(
	[...Array(e-s+1)[Symbol.iterator]().map((_,i)=>String.fromCodePoint(s+i))],
	o
)),

w=(await[
	...range_sjis([
		[0x20,0x7e,{hankaku:true,descender:true}],// alphanum
		[0xa1,0xdf,{hankaku:true}],// kata

		[0x8140,0x81fc],// symbol
		[0x824f,0x829a,{descender:true}],// alphanum
		[0x829f,0x8396],// hira kata
		[0x839f,0x83d6,{hankaku:true,descender:true}],// greek
		[0x8440,0x8491,{hankaku:true,descender:true}],// cyrillic
		[0x849f,0x84be],// box-drawing
		[0x8740,0x8753],// rounded-num
		[0x8754,0x875d],// roman-num-upper
		[0x875f,0x8775],// units
		[0x877e,0x878f],// ligature
		[0x8790,0x879c],// math-symbol

		[0x889f,0x9872],// kanji
		[0x989f,0x9ffc],
		[0xe040,0xeaa4],
		[0xed40,0xeeec],

		[0xeeef,0xeef8],// roman-num-lower
		[0xeef9,0xeefc],// symbol
	]),
	...range([
		[0x00c0,0x02af,{hankaku:true}],// latin-ext
	])
].reduce(async(
	a,
	w,
	{
		c=createCanvas(size,size*(w.hankaku?.5:1)),
		ctx=(ctx=>(
			ctx.textBaseline='top',
			ctx.font=`${size}px ${ffam.raw}`,
				ctx.rotate(Math.PI/2),
			(({
				actualBoundingBoxDescent:d,
				actualBoundingBoxAscent:a
				// fontBoundingBoxDescent:d,
				// fontBoundingBoxAscent:a
			})=>(
				ctx.font=`${size/(d-a)*size}px ${ffam.raw}`
			))(ctx.measureText(w.join(''))),
			ctx
		))(c.getContext('2d'))
	},
)=>(
	a=await a,
	await w.reduce(async(b,x)=>(
		await b,
		x=new String(x),
		x.cp=x.codePointAt(),
		x.name=x.cp.toString(16).padStart(4,0),
		x.file=Bun.file(`${dst}.part/${x.name}`),
		x.size=((c.width-1)<<4)|((c.height-1)&15),
		await x.file.exists()||(
			console.log(`\x1b[1A${x.name}`),
			a.push(x),
			ctx.fillStyle='#000',
			ctx.fillRect(0,-size,c.height,c.width),
			ctx.fillStyle='#fff',
			ctx.fillText(x,0,-size,c.height),

			await Bun.write(
				x.file,
				new Uint8Array([
					...ctx.getImageData(0,0,c.width,c.height).data[Symbol.iterator]()
						.filter((_,i)=>!(i%4))
						.reduce((a,x,i)=>(
							x=127<x,
							i%8?(a[a.length-1]|=x<<(7-i%8)):a.push(x<<7),
							a
						),[])
				])
			),
			x.file=Bun.file(x.file.name)
		)
	),0),
	a
),[],console.log('gen: render...\n'))).sort(),
f=(s=>({
	write:x=>new Promise(f=>s.write(x,f)),
	end:_=>new Promise(f=>s.end(f))
}))((
	await Bun.write(dst,''),
	await open(dst,{flags:'a'})
).createWriteStream()),
le24=x=>new Uint8Array([(x>>>0)&255,(x>>>8)&255,(x>>>16)&255]),
le16248=(x,y,z)=>new Uint8Array([(x>>>0)&255,(x>>>8)&255,(y>>>0)&255,(y>>>8)&255,(y>>>16)&255,z&255]);


// console.log(w);

console.log('gen: index size...');
await f.write(le24((2+3+1)*w.length));

console.log('gen: index...');
await w.reduce(async(a,x)=>(
	a=await a,
	await f.write(le16248(x.cp,a,x.size)),
	a+x.file.size,
),3+(2+3+1)*w.length);

console.log('gen: data...\n');
await w.reduce(async(a,x)=>(
	await a,
	console.log(`\x1b[1A${x.name}`),
	await f.write(await x.file.bytes())
),0)

f.end();
await Bun.$`rm -rf ${dst}.part`;
console.log('gen: done!');


