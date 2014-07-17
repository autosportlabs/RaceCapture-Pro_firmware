#include "usart.h"
#include "usart_device.h"
#include <stdint.h>

int usart_init(){
	return usart_device_init();
}

int usart_init_serial(Serial *serial, size_t port){
	return usart_device_init_serial(serial, port);
}

void usart_config(uint8_t port, uint8_t bits, uint8_t parity, uint8_t stopbits, uint32_t baud){
	usart_device_config(port, bits, parity, stopbits, baud);
}



