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
#include "printk.h"

#define UART_QUEUE_LENGTH			1024
xQueueHandle 						xUsart0Rx	= 0;
xQueueHandle 						xUsart1Rx	= 0;
xQueueHandle 						xUsart2Rx	= 0;
xQueueHandle 						xUsart3Rx	= 0;
int									uart0Fd		= -1;
int									uart1Fd		= -1;
int									uart2Fd		= -1;
int									uart3Fd		= -1;

void usart0_flush(void);
void usart1_flush(void);
void usart2_flush(void);
void usart3_flush(void);


int usart_device_init(){
	return 1;
} // usart_device_init

void usart_device_config(uint8_t port, uint8_t bits, uint8_t parity, uint8_t stopbits, uint32_t baud){
} // usart_device_config

void usart_device_init_0(unsigned int bits, unsigned int parity, unsigned int stopBits, unsigned int baud){
   	
	/* Set-up the Serial Console Echo task */
	if ( pdTRUE == lAsyncIOPtmxOpen( "WIRELESS", &uart0Fd ) )
	{
		xUsart0Rx = xQueueCreate(UART_QUEUE_LENGTH,
			( unsigned portBASE_TYPE ) sizeof( signed portCHAR ));
		(void)lAsyncIORegisterCallback( uart0Fd, vAsyncPtmxIODataAvailableISR, xUsart0Rx );
		usart0_flush();
	}
	else
	{
		 pr_error("Failed to initialize UART 0\r\n");
	}
} // usart_device_init_0

void usart_device_init_1(unsigned int bits, unsigned int parity, unsigned int stopBits, unsigned int baud){
   	
	/* Set-up the Serial Console Echo task */
	if ( pdTRUE == lAsyncIOPtmxOpen( "TELEMETRY", &uart1Fd ) )
	{
		xUsart1Rx = xQueueCreate(UART_QUEUE_LENGTH,
			( unsigned portBASE_TYPE ) sizeof( signed portCHAR ));
		(void)lAsyncIORegisterCallback( uart1Fd, vAsyncPtmxIODataAvailableISR, xUsart1Rx );
		usart1_flush();
	}
	else
	{
		 printf("Failed to initialize TELEMETRY\r\n");
	}
}

void usart_device_init_2(unsigned int bits, unsigned int parity, unsigned int stopBits, unsigned int baud){
   	
	/* Set-up the Serial Console Echo task */
	if ( pdTRUE == lAsyncIOPtmxOpen( "GPS", &uart2Fd ) )
	{
		xUsart2Rx = xQueueCreate(UART_QUEUE_LENGTH,
			( unsigned portBASE_TYPE ) sizeof( signed portCHAR ));
		(void)lAsyncIORegisterCallback( uart2Fd, vAsyncPtmxIODataAvailableISR, xUsart2Rx );
		usart2_flush();
	}
	else
	{
		 printf("Failed to initialize GPS\r\n");
	}
}

void usart_device_init_3(unsigned int bits, unsigned int parity, unsigned int stopBits, unsigned int baud){
   	
	/* Set-up the Serial Console Echo task */
	if ( pdTRUE == lAsyncIOPtmxOpen( "AUX", &uart3Fd ) )
	{
		xUsart3Rx = xQueueCreate(UART_QUEUE_LENGTH,
			( unsigned portBASE_TYPE ) sizeof( signed portCHAR ));
		(void)lAsyncIORegisterCallback( uart3Fd, vAsyncPtmxIODataAvailableISR, xUsart3Rx );
		usart3_flush();
	}
	else
	{
		 printf("Failed to initialize AUX\r\n");
	}
}

void usart_device_init_4(unsigned int bits, unsigned int parity, unsigned int stopBits, unsigned int baud){
}

void usart0_flush(void) {
	char rx;
	while (xQueueReceive(xUsart0Rx, &rx, 0))
		;
}

void usart1_flush(void)
{
	char rx;
	while (xQueueReceive(xUsart1Rx, &rx, 0))
		;
}

void usart2_flush(void)
{
	char rx;
	while (xQueueReceive(xUsart2Rx, &rx, 0))
		;
}

void usart3_flush(void)
{
	char rx;
	while (xQueueReceive(xUsart3Rx, &rx, 0))
		;
}

void usart4_flush(void)
{}

int usart0_getcharWait(char *c, size_t delay) {
	
	if(0 != xUsart0Rx)
	{
	   return xQueueReceive( xUsart0Rx, c, delay ) == pdTRUE ? 1 : 0;
    }
    else
    {
		printf("Warning queue for WIRELESS is not created yet");
		return 0;
	}
}

char usart1_getcharWait(char *c, size_t delay)
{	
	if(0 != xUsart1Rx)
	{
	   return xQueueReceive( xUsart1Rx, c, delay ) == pdTRUE ? 1 : 0;
    }
    else
    {
		printf("Warning queue TELEMETRY is not created yet");
		return 0;
	}
}

char usart2_getcharWait(char *c, size_t delay)
{	
	if(0 != xUsart2Rx)
	{
	   return xQueueReceive( xUsart2Rx, c, delay ) == pdTRUE ? 1 : 0;
    }
    else
    {
		printf("Warning queue for GPS is not created yet");
		return 0;
	}
}

char usart3_getcharWait(char *c, size_t delay)
{	
	if(0 != xUsart3Rx)
	{
	   return xQueueReceive( xUsart3Rx, c, delay ) == pdTRUE ? 1 : 0;
    }
    else
    {
		printf("Warning queue for AUX is not created yet");
		return 0;
	}
}

char usart4_getcharWait(char *c, size_t delay)
{	
	return 0;
}

char usart0_getchar() {
	char c;
	usart0_getcharWait(&c, portMAX_DELAY);
	return c;
}

char usart1_getchar()
{
	char c;
	usart1_getcharWait(&c, portMAX_DELAY);
	return c;
}

char usart2_getchar()
{
	char c;
	usart2_getcharWait(&c, portMAX_DELAY);
	return c;
}

char usart3_getchar()
{
	char c;
	usart3_getcharWait(&c, portMAX_DELAY);
	return c;
}

char usart4_getchar()
{
	return '\0';
}

void usart0_putchar(char c){
   int result = 0;
   // Added this while testing, may not be required
   grantpt(uart0Fd);
   unlockpt(uart0Fd);
   result = write(uart0Fd, &c, 1);
   if(1 != result)
      printf("\r\nFailed to write 1 byte to WIRELESS (%d %d)\r\n", uart0Fd, result);
}

void usart1_putchar(char c){
   // Added this while testing, may not be required
   //grantpt(uart1Fd);
   //unlockpt(uart1Fd);
   
   if(1 != write(uart1Fd, &c, 1))
      printf("\r\nFailed to write to TELEMETRY\r\n");
}

void usart2_putchar(char c){
   
   // Added this while testing, may not be required
   //grantpt(uart2Fd);
   //unlockpt(uart2Fd);
   
   if(1 != write(uart2Fd, &c, 1))
      printf("\r\nFailed to write to GPS\r\n");
}

void usart3_putchar(char c){
   
   // Added this while testing, may not be required
   //grantpt(uart3Fd);
   //unlockpt(uart3Fd);
   
   if(1 != write(uart3Fd, &c, 1))
      printf("\r\nFailed to write to AUX\r\n");
}

void usart4_putchar(char c){
}

void usart0_puts (const char* s )
{
	int result = -1;
	
   // Added this for testing, may not be required.
   //grantpt(uart0Fd);
   //unlockpt(uart0Fd);
   
   result = write(uart0Fd, s, strlen(s));
   
   if(strlen(s) != result)
   {
      printf("\r\nFailed to write to WIRELESS (%d vs %d)\r\n", result,strlen(s) );
  }
}

void usart1_puts (const char* s )
{
   // Added this for testing, may not be required.
   //grantpt(uart1Fd);
   //unlockpt(uart1Fd);
   
   if(strlen(s) != write(uart1Fd, s, strlen(s)))
   {
      printf("\r\nFailed to write to TELEMETRY\r\n");
  }
}

void usart2_puts (const char* s )
{
   // Added this for testing, may not be required.
   //grantpt(uart2Fd);
   //unlockpt(uart2Fd);
   
   if(strlen(s) != write(uart2Fd, s, strlen(s)))
   {
      printf("\r\nFailed to write to GPS\r\n");
  }
}

void usart3_puts (const char* s )
{
   // Added this for testing, may not be required.
   //grantpt(uart3Fd);
   //unlockpt(uart3Fd);
   
   if(strlen(s) != write(uart3Fd, s, strlen(s)))
   {
      printf("\r\nFailed to write to AUX\r\n");
  }
}

void usart4_puts (const char* s )
{}

int usart0_readLineWait(char *s, int len, size_t delay) {
	int count = 0;
	char c = 0;
	if(len <= 1)
	{
		   printf("\r\nInvalid read length requested on TELEMETRY");
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

int usart1_readLineWait(char *s, int len, size_t delay)
{
	int count = 0;
	char c = 0;
	if(len <= 1)
	{
		   printf("\r\nInvalid read length requested on UART1");
		   return 0;
	}
	while (count < len - 1) {
		
		if (!usart1_getcharWait(&c, delay))
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

int usart2_readLineWait(char *s, int len, size_t delay)
{
	int count = 0;
	char c = 0;
	if(len <= 1)
	{
		   printf("\r\nInvalid read length requested on UART2");
		   return 0;
	}
	while (count < len - 1) {
		
		if (!usart2_getcharWait(&c, delay))
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

int usart3_readLineWait(char *s, int len, size_t delay)
{
	int count = 0;
	char c = 0;
	if(len <= 1)
	{
		   printf("\r\nInvalid read length requested on UART3");
		   return 0;
	}
	while (count < len - 1) {
		
		if (!usart3_getcharWait(&c, delay))
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

int usart4_readLineWait(char *s, int len, size_t delay)
{
	*s = '\0';
	return 0;
}

int usart0_readLine(char *s, int len) {
	return usart0_readLineWait(s, len, portMAX_DELAY);
}

int usart1_readLine(char *s, int len)
{
	return usart1_readLineWait(s,len,0);
}

int usart2_readLine(char *s, int len)
{
	return usart2_readLineWait(s,len,0);
}

int usart3_readLine(char *s, int len)
{
	return usart3_readLineWait(s,len,0);
}

int usart4_readLine(char *s, int len)
{
	return usart4_readLineWait(s,len,0);
}

int usart_device_init_serial(Serial *serial, uart_id_t id) {
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
		case UART_TELEMETRY:
		   serial->init = &usart_device_init_1;
			serial->flush = &usart1_flush;
			serial->get_c = &usart1_getchar;
			serial->get_c_wait = &usart1_getcharWait;
			serial->get_line = &usart1_readLine;
			serial->get_line_wait = &usart1_readLineWait;
			serial->put_c = &usart1_putchar;
			serial->put_s = &usart1_puts;
			usart_device_init_1(0,0,0,0);
		   break;
		case UART_GPS:
		   serial->init = &usart_device_init_2;
			serial->flush = &usart2_flush;
			serial->get_c = &usart2_getchar;
			serial->get_c_wait = &usart2_getcharWait;
			serial->get_line = &usart2_readLine;
			serial->get_line_wait = &usart2_readLineWait;
			serial->put_c = &usart2_putchar;
			serial->put_s = &usart2_puts;
			usart_device_init_2(0,0,0,0);
		   break;
		case UART_AUX:
		   serial->init = &usart_device_init_3;
			serial->flush = &usart3_flush;
			serial->get_c = &usart3_getchar;
			serial->get_c_wait = &usart3_getcharWait;
			serial->get_line = &usart3_readLine;
			serial->get_line_wait = &usart3_readLineWait;
			serial->put_c = &usart3_putchar;
			serial->put_s = &usart3_puts;
			usart_device_init_3(0,0,0,0);
		   break;
		default:
		   pr_error("Unknown serial port initialization requested\r\n");
		   serial->init = &usart_device_init_4;
			serial->flush = &usart4_flush;
			serial->get_c = &usart4_getchar;
			serial->get_c_wait = &usart4_getcharWait;
			serial->get_line = &usart4_readLine;
			serial->get_line_wait = &usart4_readLineWait;
			serial->put_c = &usart4_putchar;
			serial->put_s = &usart4_puts;
	} // switch
	
	return 1;
}
