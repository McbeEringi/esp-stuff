#!/bin/env bun
import{createCanvas,GlobalFonts}from'@napi-rs/canvas';

const
family='sansserif',
size=32,
pfam=await Bun.$`fc-match -f"%{family}" ${family}`.text(),
ctx=createCanvas(32,32).getContext('2d'),
x='jp',
tds=new TextDecoder('sjis'),
range_sjis=w=>w.map(([s,e,o={}])=>Object.assign(
	Array(e-s+1)[Symbol.iterator]().map(
		(_,i)=>s+i
	).filter(
		e<=0xff?
		_=>1:
		x=>(x=x&0xff,0x40<=x&&x<=0xfc&&x!=0x7f)
	).map(
		x=>tds.decode(new Uint8Array(x<=0xff?[x]:[x>>8,x&0xff]))
	).filter(
		x=>x.length==1&&x!='\ufffd'
	),
	o
)),
range=w=>w.map(([s,e,o={}])=>Object.assign(
	Array(e-s+1)[Symbol.iterator]().map((_,i)=>String.fromCodePoint(s+i)),
	o
));


let
d=[...new Set(
	[
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
	].flatMap(x=>[...x])
)];
console.log(d.length*32);
d=Object.entries(
	d.reduce((a,x)=>(
		x=Object.assign(x,{cp:x.codePointAt().toString(16).padStart(4,0)}),
		a[x.cp.slice(0,-2)]??=[],
		a[x.cp.slice(0,-2)].push(x),
		a
	),{})
)
.sort((a,b)=>`0x${a[0]}`-`0x${b[0]}`)
.length*32*256;
// .map(([i,x])=>(
// 	x=x.sort((a,b)=>a.cp-b.cp),
// 	`${i}X: ${x.length},`
// )).join('\n');

console.log(d);


// ctx.textBaseline='bottom';
// ctx.font=`${size}px ${pfam}`;
// (t=>(
// 	ctx.fillStyle='#404',
// 	ctx.fillRect(0,0,ctx.canvas.width,ctx.canvas.height),
//
// 	ctx.font=`${size/(t.actualBoundingBoxDescent-t.actualBoundingBoxAscent)*size}px ${pfam}`,
// 	console.log(x,t=ctx.measureText(x)),
//
// 	ctx.fillStyle='#fff',
// 	ctx.fillText(x,0,0)
// ))(ctx.measureText(x));

// await Bun.write('a.png',await ctx.canvas.encode('png'));
