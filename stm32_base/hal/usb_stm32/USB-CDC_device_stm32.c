#include "USB-CDC_device.h"

int USB_CDC_device_init(){
	return 1;
}

void USB_CDC_send_debug(portCHAR *string){

}

void USB_CDC_SendByte( portCHAR cByte ){

}

portBASE_TYPE USB_CDC_ReceiveByte(portCHAR *data){
	return 0;
}

portBASE_TYPE USB_CDC_ReceiveByteDelay(portCHAR *data, portTickType delay ){
	return 0;
}

int USB_CDC_is_initialized(){
	return 1;
}

