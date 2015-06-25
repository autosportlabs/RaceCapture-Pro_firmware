#include "usart.h"

int usart_device_init()
{
    return 1;
}

int usart_device_init_serial(Serial *serial, uart_id_t id)
{
    return 1;
}

void usart_device_config(uint8_t port, uint8_t bits, uint8_t parity, uint8_t stopbits, uint32_t baud)
{

}

void usart_device_init_0(unsigned int bits, unsigned int parity, unsigned int stopBits, unsigned int baud)
{

}

void usart_device_init_1(unsigned int bits, unsigned int parity, unsigned int stopBits, unsigned int baud)
{

}

void usart0_flush(void)
{
}

void usart1_flush(void)
{
}


char usart0_getcharWait(size_t delay)
{
    return 0;
}

char usart0_getchar()
{
    return 0;
}

char usart1_getcharWait(size_t delay)
{
    return 0;
}

char usart1_getchar()
{
    return 0;
}

void usart0_putchar(char c)
{

}

void usart1_putchar(char c)
{

}

void usart0_puts (const char* s )
{

}

void usart1_puts (const char* s )
{

}


int usart0_readLineWait(char *s, int len, size_t delay)
{
    return 0;
}


int usart0_readLine(char *s, int len)
{
    return usart0_readLineWait(s,len,0);
}

int usart1_readLineWait(char *s, int len, size_t delay)
{
    return 0;
}

int usart1_readLine(char *s, int len)
{
    return usart1_readLineWait(s,len,0);
}
