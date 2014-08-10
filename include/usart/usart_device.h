#ifndef USART_DEVICE_H
#define USART_DEVICE_H
#include <stddef.h>
#include "serial.h"

int usart_device_init();

void usart_device_config(uint8_t port, uint8_t bits, uint8_t parity, uint8_t stopbits, uint32_t baud);

int usart_device_init_serial(Serial *serial, size_t id);

void usart_device_init_0(unsigned int bits, unsigned int parity, unsigned int stopBits, unsigned int baud);

void usart_device_init_1(unsigned int bits, unsigned int parity, unsigned int stopBits, unsigned int baud);

void usart_device_init_2(unsigned int bits, unsigned int parity, unsigned int stopBits, unsigned int baud);

void usart_device_init_3(unsigned int bits, unsigned int parity, unsigned int stopBits, unsigned int baud);

void usart0_flush(void);

char usart0_getchar();

char usart0_getcharWait(size_t delay);

void usart0_putchar(char c);

void usart0_puts (const char* s );

int usart0_readLine(char *s, int len);

int usart0_readLineWait(char *s, int len, size_t delay);


void usart1_flush(void);

char usart1_getchar();

char usart1_getcharWait(size_t delay);

void usart1_putchar(char c);

void usart1_puts (const char* s );

int usart1_readLine(char *s, int len);

int usart1_readLineWait(char *s, int len, size_t delay);


void usart2_flush(void);

char usart2_getchar();

char usart2_getcharWait(size_t delay);

void usart2_putchar(char c);

void usart2_puts (const char* s );

int usart2_readLine(char *s, int len);

int usart2_readLineWait(char *s, int len, size_t delay);


void usart3_flush(void);

char usart3_getchar();

char usart3_getcharWait(size_t delay);

void usart3_putchar(char c);

void usart3_puts (const char* s );

int usart3_readLine(char *s, int len);

int usart3_readLineWait(char *s, int len, size_t delay);

#endif //#ifndef USART_DEVICE_H
