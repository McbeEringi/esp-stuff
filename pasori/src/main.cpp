#include <Arduino.h>
#include <usb/usb_host.h>

static const char *TAG="USB";

typedef struct {
	usb_host_client_handle_t cli_hdl;
	uint8_t addr;
	usb_device_handle_t hdl;
	usb_device_info_t info;
	const usb_device_desc_t *d_desc;
	const usb_config_desc_t *c_desc;
} class_device_t;



static void send(uint8_t l,uint8_t *x){}

static void send_packet(uint8_t l,uint8_t *x){
  uint8_t w[128]={0,0,0xff,l,0x100-l};
  memcpy(&w[5],x,l);
  uint8_t a=0;
  for(uint8_t i=0;i<l;i++)a+=x[i];
  w[l+5]=0x100-a;
  send(l+7,w);
}



static void client_event_cb(const usb_host_client_event_msg_t *e, void *arg){
	class_device_t *d=(class_device_t *)arg;
	switch(e->event){
		case USB_HOST_CLIENT_EVENT_NEW_DEV :d->addr=e->new_dev.address;break;
		case USB_HOST_CLIENT_EVENT_DEV_GONE:d->addr=0;break;
	}
}
static void printd(class_device_t d){
	ESP_LOGI(TAG,"\t%s speed",(char *[]){"Low","Full","High"}[d.info.speed]);
	ESP_LOGI(TAG,"\tbConfigurationValue %d",d.info.bConfigurationValue);
	if(d.info.str_desc_manufacturer){
		ESP_LOGI(TAG,"*** Manufacturer string descriptor ***");
		usb_print_string_descriptor(d.info.str_desc_manufacturer);
	}
	if(d.info.str_desc_product){
		ESP_LOGI(TAG,"*** Product string descriptor ***");
		usb_print_string_descriptor(d.info.str_desc_product);
	}
	if(d.info.str_desc_serial_num){
		ESP_LOGI(TAG,"*** Serial Number string descriptor ***");
		usb_print_string_descriptor(d.info.str_desc_serial_num);
	}
	usb_print_device_descriptor(d.d_desc);
	usb_print_config_descriptor(d.c_desc,NULL);
}
static void class_device_task(void *arg){
	class_device_t d={0};
	usb_host_client_config_t client_config={
		.is_synchronous=false,
		.max_num_event_msg=5,
		.async={
			.client_event_callback=client_event_cb,
			.callback_arg=(void *)&d,
		},
	};
	usb_host_client_register(&client_config,&d.cli_hdl);
	while(1){
		usb_host_client_handle_events(d.cli_hdl,portMAX_DELAY);
		if(d.addr){
			ESP_LOGI(TAG,"ACT_OP");
			d.hdl=NULL;
			ESP_LOGI(TAG,"Opening device at address %d ...",d.addr);
			ESP_ERROR_CHECK(usb_host_device_open(d.cli_hdl,d.addr,&d.hdl));
			ESP_ERROR_CHECK(usb_host_device_info(d.hdl, &d.info));
			ESP_ERROR_CHECK(usb_host_get_device_descriptor(d.hdl,&d.d_desc));
			ESP_ERROR_CHECK(usb_host_get_active_config_descriptor(d.hdl,&d.c_desc));
			printd(d);
			
			if(d.d_desc->idVendor==0x54c&&d.d_desc->idProduct==0x1bb){
				ESP_LOGI(TAG,"Looks like RC-S320!");
				ESP_LOGI(TAG,"Opening interface...");
				usb_host_interface_claim(d.cli_hdl,d.hdl,0,0);
				usb_transfer_t *tx;
				usb_host_transfer_alloc(128,0,&tx);
				memset(tx->data_buffer, 0xAA, 128);
				tx->num_bytes=128;
				tx->device_handle=d.hdl;
				tx->bEndpointAddress=0x81;
				tx->callback=transfer_cb;
				usb_host_transfer_submit(transfer);
				usb_host_interface_release(d.cli_hdl,d.hdl,0);
			}else{
				ESP_LOGI(TAG,"Unknown device. Closing...");
				ESP_ERROR_CHECK(usb_host_device_close(d.cli_hdl,d.hdl));
				d.hdl=NULL;
			}
		}else if(d.hdl!=NULL){
			ESP_LOGI(TAG,"Unplugged. Closing...");
			ESP_ERROR_CHECK(usb_host_device_close(d.cli_hdl,d.hdl));
		}
	}
}

static void usb_host_lib_task(void *arg){
	usb_host_config_t host_config={
		.skip_phy_setup=false,
		.intr_flags=ESP_INTR_FLAG_LEVEL1,
	};
	ESP_ERROR_CHECK(usb_host_install(&host_config));
	xTaskNotifyGive(arg);

	while(1){
		uint32_t event_flags;
		usb_host_lib_handle_events(portMAX_DELAY,&event_flags);
		if(event_flags&USB_HOST_LIB_EVENT_FLAGS_NO_CLIENTS)ESP_ERROR_CHECK(usb_host_device_free_all());
	}
	/*ESP_LOGI(TAG,"No more clients and devices");*/
	/*usb_host_uninstall();*/
	/*vTaskDelete(NULL);*/
}

void setup(){
	xTaskCreatePinnedToCore(usb_host_lib_task,"usb_host",4096,xTaskGetCurrentTaskHandle(),2,NULL,0);
	ulTaskNotifyTake(pdFALSE,portMAX_DELAY);
	xTaskCreatePinnedToCore(class_device_task,"class",4096,NULL,3,NULL,0);
}
void loop(){neopixelWrite(38,
	(sin(millis()/1000.    )*.5+.5)*16.,
	(sin(millis()/1000.+2.1)*.5+.5)*16.,
	(sin(millis()/1000.+4.2)*.5+.5)*16.
);delay(1);}
