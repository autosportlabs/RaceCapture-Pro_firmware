#include "usart.h"

int initUsart()
{

}

void initUsart0(unsigned int bits, unsigned int parity, unsigned int stopBits, unsigned int baud){

}

void initUsart1(unsigned int bits, unsigned int parity, unsigned int stopBits, unsigned int baud){

}

void usart0_flush(void)
{
}

void usart1_flush(void)
{
}


char usart0_getcharWait(size_t delay){
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

void usart0_putchar(char c){

}

void usart1_putchar(char c){

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
