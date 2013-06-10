#ifndef USART_H
#define USART_H
#include <stddef.h>

int initUsart();

void initUsart0(unsigned int bits, unsigned int parity, unsigned int stopBits, unsigned int baud);

void initUsart1(unsigned int bits, unsigned int parity, unsigned int stopBits, unsigned int baud);

void usart0_flush(void);

char usart0_getcharWait(size_t delay);

char usart0_getchar();

int usart0_putchar(char c);

int usart0_puts (const char* s );

int usart0_readLine(char *s, int len);

int usart0_readLineWait(char *s, int len, size_t delay);

char usart1_getcharWait(size_t delay);

char usart1_getchar();

int usart1_putchar(char c);

int usart1_puts (const char* s );

int usart1_readLine(char *s, int len);

int usart1_readLineWait(char *s, int len, size_t delay);

void usart1_flush(void);

#endif //#ifndef USART_H
