#!/bin/bun
import{txt2bin}from'./esp-tp-modules/txt2bin.mjs';
import{WSPrint}from'./esp-tp-modules/wsprint.mjs';

const
td=new TextDecoder(),
tp=new WSPrint('ws://esp-tp.local/ws').open(),
main=_=>(
	console.log('connecting...'),
	self.ws=Object.assign(new WebSocket('ws://esp-qr-reader.local/ws'),{
		binaryType:'arraybuffer',
		onopen:_=>(self.c=true,console.log('open')),
		onclose:_=>(self.c=false,setTimeout(main,500)),
		onmessage:e=>(console.log(e=td.decode(e.data)),tp.send(txt2bin(e.slice(1,-1)+'\n')))
	})
);
setInterval(_=>(self.c&&self.ws?.send(new Uint8Array([1]))),2000);
main();
tp.event.addEventListener('open',_=>console.log('tp open'));
