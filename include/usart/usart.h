#ifndef USART_H
#define USART_H
#include "serial.h"
#include <stdint.h>

typedef enum {
	USART_GPS,
	USART_TELEMETRY,
	USART_WIRELESS,
	USART_AUX
} usart_id_t;

int usart_init();
int usart_init_serial(Serial *serial, size_t port);
void usart_config(usart_id_t port, uint8_t bits, uint8_t parity, uint8_t stopbits, uint32_t baud);

#endif //#ifndef USART_H
