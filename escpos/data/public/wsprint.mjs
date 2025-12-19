const
WSPrint=class{
	queue=[];
	ws=null;
	sending=false;
	ep=null;
	event=new EventTarget();
	reconnect=true;

	constructor(w){this.ep=w;}
	open(){
		this.ws&&[0,1].includes(this.ws.readyState)&&this.close();
		this.reconnect=true;
		this.ws=Object.assign(new WebSocket(this.ep),{
			onopen:e=>(
				this.event.dispatchEvent(new CustomEvent('open',{detail:e})),
				this.queue.length&&this.#send()
			),
			onmessage:e=>(
				e.data=='OK'?(this.queue.length?this.#send():(
					this.sending=false,
					this.event.dispatchEvent(new CustomEvent('idle',{detail:e}))
				)):this.event.dispatchEvent(new CustomEvent('msg',{detail:e}))
			),
			onclose:e=>(
				this.event.dispatchEvent(new CustomEvent('close',{detail:e})),
				this.reconnect&&setTimeout(_=>this.open(),1000)
			)
		});
		return this;
	}

	#send(){
		this.ws&&this.ws.readyState==1&&(
			this.sending=true,
			this.ws.send(new Uint8Array(this.queue.shift())),
			this.event.dispatchEvent(new CustomEvent('send',{detail:{remaining:this.queue.length}}))
		);
	}
	#chop(w,l=512){return[...Array(Math.ceil(w.length/l))].map((_,i)=>w.slice(i*l,++i*l));}

	send(w){
		this.queue.push(...this.#chop(w));
		this.sending||this.#send();
		return this;
	}
	close(){this.reconnect=false;this.ws.close();}
};

export{WSPrint};
