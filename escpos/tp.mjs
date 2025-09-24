#!/bin/env -S bun
import{usb,getDeviceList,webusb}from'usb';

const
w=getDeviceList().filter(({deviceDescriptor:{idVendor:vid,idProduct:pid}})=>vid==0x28e9&&pid==0x0289),
delay=x=>new Promise(f=>setTimeout(f,x)),
te=new TextEncoder(),
u=x=>new Uint8Array(x),
cmd=x=>u(x.split(/\s/).flatMap(x=>
(console.log(x),x=>~x?[x]:null)(`NUL SOH STX ETX EOT ENQ ACK BEL BS HT LF VT FF CR SO SI
DLE DC1 DC2 DC3 DC4 NAK SYN ETB CAN EM SUB ESC FS GS RS US
SP`.split(/\s/).findIndex(y=>y==x))??(/^[0-9a-f]{2}$/i.test(x)?[+('0x'+x)]:null)??[...te.encode(x)]
));

w.forEach(async x=>(
	x.open(),
	await(async(
		xif=x.interface(0),
		[oe,ie]=(
			xif.isKernelDriverActive()&&xif.detachKernelDriver(),
			xif.claim(),
			(x=>[x.out[0],x.in[0]])(xif.endpoints.reduce((a,x)=>(a[x.direction].push(x),a),{in:[],out:[]}))
		),
		send=x=>(console.log(x=cmd(x)),oe.transfer(x))
	)=>(
		ie.on('data',console.log),
		ie.startPoll(),
		send('ESC @'),// init
		// send('ESC ##SBDR 80 25 00 00'),// baud 9600
		// send('ESC ##SBDR 00 c2 01 00'),// baud 115200
		// send('ESC ##SLAN 00'),// pc437
		oe.transfer(u([...Array(0xff-0x20)].map((_,i)=>i+0x20))),
		// oe.transfer(te.encode('hello world!\n'),console.log),
		send('GS V 00'),// cut
		// await delay(1000),
		// await new Promise(_=>_),
		await new Promise(f=>ie.stopPoll(f))
	))(),
	x.close()
))
// console.log(await webusb.getDevices());
// console.log(
// await webusb.requestDevice({ filters: [{ vendorId: 0x28e9 }] })
// )
