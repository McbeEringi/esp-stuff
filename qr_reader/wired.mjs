#!/bin/node
import{SerialPort}from'serialport';
import{txt2bin}from'./modules/txt2bin.mjs';
import{cmd2bin}from'./modules/cmd2bin.mjs';

let t,b=[];
const
td=new TextDecoder(),
qr=new SerialPort({path:'/dev/ttyACM0',baudRate:115200}),
tp=new SerialPort({path:'/dev/ttyACM1',baudRate:115200}),
print=x=>(
	x=td.decode(new Uint8Array(b)),
	x={x:x[0],w:x.slice(1,-1)},
	console.log(x),
	tp.write(new Uint8Array([
		...cmd2bin('GS !'),0x11,
		...txt2bin(x.w+'\n')
	])),
	b=[]
);
qr.on('data',x=>(
	b.push(...new Uint8Array(x)),
	clearTimeout(t),
	t=setTimeout(print,150)
));

await new Promise(_=>0)
// serialport.write('ROBOT POWER ON')



// import{SerialPort,readlineParser}from'bun-serialport';
//
// const 
// qr=new SerialPort({path:'/dev/ttyACM0',baudRate: 115200}).pipe(readlineParser()),
// tp=new SerialPort({path:'/dev/ttyACM1',baudRate: 115200});
//
//
// qr.on('data',async x=>(
// 	console.log(x),
//   await tp.write(x.slice(1,-1)+'\n')
// ))
