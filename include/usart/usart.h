#ifndef USART_H
#define USART_H
#include "serial.h"
#include <stdint.h>

typedef enum {
    UART_GPS = 0,
    UART_TELEMETRY,
    UART_WIRELESS,
    UART_AUX
} uart_id_t;

#define SERIAL_COUNT 5

int usart_init();
int usart_init_serial(Serial *serial, uart_id_t port);
void usart_config(uart_id_t port, uint8_t bits, uint8_t parity, uint8_t stopbits, uint32_t baud);

#endif //#ifndef USART_H
