const
WSPrint=class{
	queue=[];
	ws=null;
	sending=false;
	ep=null;

	constructor(w){this.ep=w;this.open();}
	open(){
		this.ws=Object.assign(new WebSocket(this.ep),{
			onopen:e=>(console.log(e),this.queue.length&&this.#send()),
			onmessage:({data:x})=>(
				console.log(x),
				x=='OK'&&(this.queue.length?this.#send():this.sending=false)
			),
			onclose:e=>(console.log(e),setTimeout(_=>this.open(),1000))
		});
		return this;
	}

	#send(){
		this.ws&&this.ws.readyState==1&&(
			this.sending=true,
			this.ws.send(new Uint8Array(this.queue.shift())),
			console.log('remaining:',this.queue.length)
		);
	}
	#chop(w,l=512){return[...Array(Math.ceil(w.length/l))].map((_,i)=>w.slice(i*l,++i*l));}

	send(w){
		this.queue.push(...this.#chop(w));
		this.sending||this.#send();
		return this;
	}
};

export{WSPrint};
