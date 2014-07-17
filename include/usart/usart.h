#ifndef USART_H
#define USART_H
#include "serial.h"
#include <stdint.h>

int usart_init();
int usart_init_serial(Serial *serial, size_t port);
void usart_config(uint8_t port, uint8_t bits, uint8_t parity, uint8_t stopbits, uint32_t baud);

#endif //#ifndef USART_H
