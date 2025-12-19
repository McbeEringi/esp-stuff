const
img2bin=({
	genctx,img,printer_width=384,
	background_color:bg='#fff',
	transform:{rotate=0,scale=1}={},
})=>((
	imgwh=[img.naturalWidth,img.naturalHeight],
	[bb,sc]=((
		rwh=((
			[x,y]=imgwh,t=rotate,
			c=Math.cos(t),s=Math.sin(t),
			cx=c*x,sx=s*x,cy=c*y,sy=s*y
		)=>[
			[0,cx,cx-sy,-sy],
			[0,sx,sx+cy,cy]
		].map(w=>Math.max(...w)-Math.min(...w)))(),
		s=printer_width*scale/rwh[0]
	)=>[rwh.map(x=>x*s),s])(),
	cs=bb.map(Math.ceil),
	ctx=genctx(...cs),
	gs
)=>(
	ctx.fillStyle=bg,
	ctx.fillRect(0,0,...cs),
	ctx.translate(...bb.map(x=>x/2)),
	ctx.rotate(-rotate),ctx.scale(sc,sc),
	ctx.drawImage(img,...imgwh.map(x=>x/-2)),
	gs=((
		a=[...ctx.getImageData(0,0,...cs).data],
		pal=[.299,.587,.114]
	)=>[...Array(cs[1])].map((_,y)=>[...Array(cs[0])].map((i,x,{length:w})=>(
		i=(w*y+x)*4,
		a.slice(i,i+=3).reduce((a,x,i)=>a+x*pal[i],0)*(a[i]/255)
	))))(),

	// dither
	[...Array(cs[1])].forEach((_,y)=>[...Array(cs[0])].forEach((e,x)=>(
		e=gs[y][x]-(gs[y][x]=128<=gs[y][x]?255:0),
		[
			[ 1,0,1/8],[ 2,0,1/8],
			[-1,1,1/8],[ 0,1,1/8],[ 1,1,1/8],
			[ 0,2,1/8]
		].forEach(([dx,dy,d])=>(dy+=y)<cs[1]&&(dx+=x)<cs[0]&&(gs[dy][dx]+=d*e))
	))),

	ctx.putImageData(new ImageData(new Uint8ClampedArray(gs.flatMap(x=>x.flatMap(x=>[x,x,x,255]))),...cs),0,0),
	1
))();


export{img2bin};
