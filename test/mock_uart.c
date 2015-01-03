#include "usart.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "AsyncIO.h"
#include "AsyncIOPtmx.h"


#define UART_QUEUE_LENGTH 					1024
xQueueHandle xUsart0Rx = 0;
int iSerialReceive = 0;


int usart_device_init()
{
	return 1;
}

void usart_device_config(uint8_t port, uint8_t bits, uint8_t parity, uint8_t stopbits, uint32_t baud){

}

void usart_device_init_0(unsigned int bits, unsigned int parity, unsigned int stopBits, unsigned int baud){
   	
	/* Set-up the Serial Console Echo task */
	if ( pdTRUE == lAsyncIOPtmxOpen( "", &iSerialReceive ) )
	{
		xUsart0Rx = xQueueCreate(UART_QUEUE_LENGTH,
			( unsigned portBASE_TYPE ) sizeof( signed portCHAR ));
		(void)lAsyncIORegisterCallback( iSerialReceive, vAsyncPtmxIODataAvailableISR, xUsart0Rx );
	}
	else
	{
		 printf("failed to initialize serial port\r\n");
		 fflush(stdout);
	}
	
	//xTaskCreate( Usart0Task, "Uasrt0Task", 10*configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES -1, NULL );
}

void usart_device_init_1(unsigned int bits, unsigned int parity, unsigned int stopBits, unsigned int baud){

}

void usart0_flush(void) {
	char rx;
	//printf("usart0_flush\r\n");
	while (xQueueReceive(xUsart0Rx, &rx, 0))
		;
}

void usart1_flush(void)
{
	//printf("usart1_flush\r\n");
}


int usart0_getcharWait(char *c, size_t delay) {
	//printf("usart0_getcharWait\r\n");
	
	if(0 != xUsart0Rx)
	{
	   return xQueueReceive( xUsart0Rx, c, delay ) == pdTRUE ? 1 : 0;
    }
    else
    {
		printf("Warning queue is not created yet");
		return 0;
	}
}

char usart0_getchar() {
	char c;
	//printf("usart0_getchar\r\n");
	usart0_getcharWait(&c, portMAX_DELAY);
	return c;
}

char usart1_getcharWait(size_t delay)
{
	//printf("usart1_getcharWait\r\n");
	return 0;
}

char usart1_getchar()
{
	//printf("usart1_getchar\r\n");
	return 0;
}

void usart0_putchar(char c){
   printf("%c", c);
   grantpt(iSerialReceive);
	unlockpt(iSerialReceive);
   if(1 != write(iSerialReceive, &c, 1))
      printf("\r\nFailed to write to %d\r\n", iSerialReceive);
}

void usart1_putchar(char c){
   //printf("usart1_getchar\r\n");
}

void usart0_puts (const char* s )
{
   printf("%s", s);
   grantpt(iSerialReceive);
	unlockpt(iSerialReceive);
   if(strlen(s) != write(iSerialReceive, s, strlen(s)))
   {
      printf("\r\nFailed to write to %d\r\n", iSerialReceive);
  }
}

void usart1_puts (const char* s )
{
   //printf("usart1_puts\r\n");
}

int usart0_readLineWait(char *s, int len, size_t delay) {
	int count = 0;
	char c = 0;
	if(len <= 1)
	{
		   printf("\r\nInvalid read length requested");
		   return 0;
	}
	while (count < len - 1) {
		
		if (!usart0_getcharWait(&c, delay))
			break;
		*s++ = c;
		count++;
		if (c == '\r')
		{
			*(s-1)='\n';
			break;
		}
	}
	*s = '\0';
	return count;
}

int usart0_readLine(char *s, int len) {
	return usart0_readLineWait(s, len, portMAX_DELAY);
}


int usart1_readLineWait(char *s, int len, size_t delay)
{
	printf("usart1_readLineWait\r\n");
	return 0;
}

int usart1_readLine(char *s, int len)
{
	//printf("usart1_readLine\r\n");
	return usart1_readLineWait(s,len,0);
}

int usart_device_init_serial(Serial *serial, uart_id_t id) {
	printf("usart_device_init_serial %d\r\n", id);
	
	switch(id)
	{
		case UART_WIRELESS:
		   serial->init = &usart_device_init_0;
			serial->flush = &usart0_flush;
			serial->get_c = &usart0_getchar;
			serial->get_c_wait = &usart0_getcharWait;
			serial->get_line = &usart0_readLine;
			serial->get_line_wait = &usart0_readLineWait;
			serial->put_c = &usart0_putchar;
			serial->put_s = &usart0_puts;
			usart_device_init_0(0,0,0,0);
		   break;
		default:
		   serial->init = &usart_device_init_1;
			serial->flush = &usart1_flush;
			serial->get_c = &usart1_getchar;
			serial->get_c_wait = &usart1_getcharWait;
			serial->get_line = &usart1_readLine;
			serial->get_line_wait = &usart1_readLineWait;
			serial->put_c = &usart1_putchar;
			serial->put_s = &usart1_puts;
	} // switch
	
	return 1;
}
