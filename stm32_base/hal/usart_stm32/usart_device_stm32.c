#include "usart_device.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "stm32f4xx_usart.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_misc.h"
#include "stm32f4xx_rcc.h"
#include "printk.h"

#define UART_QUEUE_LENGTH 		300

#define UART_IRQ_PRIORITY 		5
#define UART_IRQ_SUB_PRIORITY 	0

xQueueHandle xUsart0Tx;
xQueueHandle xUsart0Rx;

xQueueHandle xUsart1Tx;
xQueueHandle xUsart1Rx;

xQueueHandle xUsart2Tx;
xQueueHandle xUsart2Rx;

xQueueHandle xUsart3Tx;
xQueueHandle xUsart3Rx;

static int initQueues() {

	int success = 1;
	/* Create the queues used to hold Rx and Tx characters. */
	xUsart0Rx = xQueueCreate(UART_QUEUE_LENGTH,
			( unsigned portBASE_TYPE ) sizeof( signed portCHAR ));
	xUsart0Tx = xQueueCreate(UART_QUEUE_LENGTH + 1,
			( unsigned portBASE_TYPE ) sizeof( signed portCHAR ));

	xUsart1Rx = xQueueCreate(UART_QUEUE_LENGTH,
			( unsigned portBASE_TYPE ) sizeof( signed portCHAR ));
	xUsart1Tx = xQueueCreate(UART_QUEUE_LENGTH + 1,
			( unsigned portBASE_TYPE ) sizeof( signed portCHAR ));

	xUsart2Rx = xQueueCreate(UART_QUEUE_LENGTH,
			( unsigned portBASE_TYPE ) sizeof( signed portCHAR ));
	xUsart2Tx = xQueueCreate(UART_QUEUE_LENGTH + 1,
			( unsigned portBASE_TYPE ) sizeof( signed portCHAR ));

	xUsart3Rx = xQueueCreate(UART_QUEUE_LENGTH,
			( unsigned portBASE_TYPE ) sizeof( signed portCHAR ));
	xUsart3Tx = xQueueCreate(UART_QUEUE_LENGTH + 1,
			( unsigned portBASE_TYPE ) sizeof( signed portCHAR ));

	if (	xUsart0Rx == NULL || xUsart0Tx == NULL ||
			xUsart1Rx == NULL || xUsart1Tx == NULL ||
			xUsart2Rx == NULL || xUsart2Tx == NULL ||
			xUsart3Rx == NULL || xUsart3Tx == NULL
			)
		success = 0;
	return success;
}

int usart_device_init() {
	if (!initQueues())
		return 0;
	usart_device_init_1(8, 0, 1, 9600);
	usart_device_init_2(8, 0, 1, 9600);
	usart_device_init_3(8, 0, 1, 9600);
	usart_device_init_0(8, 0, 1, 9600);
	return 1;
}

void usart_device_config(uint8_t port, uint8_t bits, uint8_t parity,
		uint8_t stopbits, uint32_t baud) {
	switch (port) {
	case 0:
		usart_device_init_0(bits, parity, stopbits, baud);
		break;
	case 1:
		usart_device_init_1(bits, parity, stopbits, baud);
		break;
	case 2:
		usart_device_init_2(bits, parity, stopbits, baud);
		break;
	case 3:
		usart_device_init_3(bits, parity, stopbits, baud);
		break;
	default:
		break;
	}
}

int usart_device_init_serial(Serial *serial, size_t id) {
	int rc = 1;
	switch (id) {
	case 0:
		serial->init = &usart_device_init_0;
		serial->flush = &usart0_flush;
		serial->get_c = &usart0_getchar;
		serial->get_c_wait = &usart0_getcharWait;
		serial->get_line = &usart0_readLine;
		serial->get_line_wait = &usart0_readLineWait;
		serial->put_c = &usart0_putchar;
		serial->put_s = &usart0_puts;
		break;

	case 1:
		serial->init = &usart_device_init_1;
		serial->flush = &usart1_flush;
		serial->get_c = &usart1_getchar;
		serial->get_c_wait = &usart1_getcharWait;
		serial->get_line = &usart1_readLine;
		serial->get_line_wait = &usart1_readLineWait;
		serial->put_c = &usart1_putchar;
		serial->put_s = &usart1_puts;
		break;

	case 2:
		serial->init = &usart_device_init_2;
		serial->flush = &usart2_flush;
		serial->get_c = &usart2_getchar;
		serial->get_c_wait = &usart2_getcharWait;
		serial->get_line = &usart2_readLine;
		serial->get_line_wait = &usart2_readLineWait;
		serial->put_c = &usart2_putchar;
		serial->put_s = &usart2_puts;
		break;

	case 3:
		serial->init = &usart_device_init_3;
		serial->flush = &usart3_flush;
		serial->get_c = &usart3_getchar;
		serial->get_c_wait = &usart3_getcharWait;
		serial->get_line = &usart3_readLine;
		serial->get_line_wait = &usart3_readLineWait;
		serial->put_c = &usart3_putchar;
		serial->put_s = &usart3_puts;
		break;

	default:
		rc = 0;
		break;
	}
	return rc;
}

void initUsart(USART_TypeDef* USARTx, unsigned int bits, unsigned int parity, unsigned int stopBits, unsigned int baud) {

	uint16_t wordLengthFlag;
	switch (bits) {
	case 9:
		wordLengthFlag = USART_WordLength_9b;
		break;
	case 8:
	default:
		wordLengthFlag = USART_WordLength_8b;
		break;
	}

	uint16_t stopBitsFlag;
	switch (stopBits) {
	case 1:
		stopBitsFlag = USART_StopBits_1;
		break;
	case 2:
		stopBitsFlag = USART_StopBits_2;
		break;
	}

	uint16_t parityFlag;
	switch (parity) {
	case 0:
		parityFlag = USART_Parity_No;
		break;
	case 1:
		parityFlag = USART_Parity_Even;
		break;
	case 2:
		parityFlag = USART_Parity_Odd;
		break;
	}
	USART_InitTypeDef usart;
	usart.USART_BaudRate = baud;
	usart.USART_WordLength = wordLengthFlag;
	usart.USART_StopBits = stopBitsFlag;
	usart.USART_Parity = parityFlag;
	usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	usart.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	USART_Init(USARTx, &usart);
	USART_Cmd(USARTx, ENABLE);
	USART_ITConfig(USARTx, USART_IT_RXNE | USART_IT_TXE, ENABLE);

}

static void initGPIO(GPIO_TypeDef* GPIOx, uint32_t gpioPins){
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = gpioPins;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOx, &GPIO_InitStructure);
}

static void initNVIC(uint8_t usartIrq){
	NVIC_InitTypeDef NVIC_InitStructure;
	/* Configure the NVIC Preemption Priority Bits */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	/* Enable the USART Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = usartIrq;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = UART_IRQ_PRIORITY;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = UART_IRQ_SUB_PRIORITY;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void usart_device_init_0(unsigned int bits, unsigned int parity, unsigned int stopBits, unsigned int baud) {
	/* --------------------------- System Clocks Configuration -----------------*/
	/* USART1 clock enable */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	/* GPIOA clock enable */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	/*-------------------------- GPIO Configuration ----------------------------*/
	initGPIO(GPIOA, GPIO_Pin_9 | GPIO_Pin_10);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);

	/*-------------------------- NVIC Configuration ----------------------------*/
	initNVIC(USART1_IRQn);

	/* USART1 configuration ------------------------------------------------------*/
	initUsart(USART1, bits, parity, stopBits, baud);
	USART_ITConfig(USART1, USART_IT_RXNE |USART_IT_TXE, ENABLE);
}

void usart_device_init_1(unsigned int bits, unsigned int parity, unsigned int stopBits, unsigned int baud) {

	/* --------------------------- System Clocks Configuration -----------------*/
	/* USART3 clock enable */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

	/* GPIOD clock enable */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

	/*-------------------------- GPIO Configuration ----------------------------*/
	initGPIO(GPIOD, GPIO_Pin_8 | GPIO_Pin_9);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource8, GPIO_AF_USART3);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource9, GPIO_AF_USART3);


	/*-------------------------- NVIC Configuration ----------------------------*/
	initNVIC(USART3_IRQn);

	/* USART1 configuration ------------------------------------------------------*/
	initUsart(USART3, bits, parity, stopBits, baud);
	USART_ITConfig(USART3, USART_IT_RXNE |USART_IT_TXE, ENABLE);

}

void usart_device_init_2(unsigned int bits, unsigned int parity, unsigned int stopBits, unsigned int baud) {

	/* --------------------------- System Clocks Configuration -----------------*/
	/* USART2 clock enable */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

	/* GPIOD clock enable */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

	/*-------------------------- GPIO Configuration ----------------------------*/
	initGPIO(GPIOD, GPIO_Pin_5 | GPIO_Pin_6);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource5, GPIO_AF_USART2);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource6, GPIO_AF_USART2);


	/*-------------------------- NVIC Configuration ----------------------------*/
	initNVIC(USART2_IRQn);

	/* USART2 configuration ------------------------------------------------------*/
	initUsart(USART2, bits, parity, stopBits, baud);
	USART_ITConfig(USART2, USART_IT_RXNE |USART_IT_TXE, ENABLE);
}

void usart_device_init_3(unsigned int bits, unsigned int parity, unsigned int stopBits, unsigned int baud) {

	/* --------------------------- System Clocks Configuration -----------------*/
	/* USART2 clock enable */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);

	/* GPIOD clock enable */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	/*-------------------------- GPIO Configuration ----------------------------*/
	initGPIO(GPIOA, GPIO_Pin_0 | GPIO_Pin_1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource0, GPIO_AF_UART4);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource1, GPIO_AF_UART4);


	/*-------------------------- NVIC Configuration ----------------------------*/
	initNVIC(UART4_IRQn);

	/* USART2 configuration ------------------------------------------------------*/
	initUsart(UART4, bits, parity, stopBits, baud);
	USART_ITConfig(UART4, USART_IT_RXNE |USART_IT_TXE, ENABLE);
}

void usart0_flush(void)
{
	char rx;
	while(xQueueReceive( xUsart0Rx, &rx, 0 ));
}

void usart1_flush(void)
{
	char rx;
	while(xQueueReceive( xUsart1Rx, &rx, 0 ));
}

void usart2_flush(void)
{
	char rx;
	while(xQueueReceive( xUsart2Rx, &rx, 0 ));
}

void usart3_flush(void)
{
	char rx;
	while(xQueueReceive( xUsart3Rx, &rx, 0 ));
}

char usart0_getcharWait(size_t delay){
	char rx = 0;

	/* Get the next character from the buffer.  Return false if no characters
	are available, or arrive before xBlockTime expires. */
	xQueueReceive( xUsart0Rx, &rx, delay );
	return rx;
}

char usart1_getcharWait(size_t delay)
{
	char rx = 0;

	/* Get the next character from the buffer.  Return false if no characters
	are available, or arrive before xBlockTime expires. */
	xQueueReceive( xUsart1Rx, &rx, delay );
	return rx;
}

char usart2_getcharWait(size_t delay)
{
	char rx = 0;

	/* Get the next character from the buffer.  Return false if no characters
	are available, or arrive before xBlockTime expires. */
	xQueueReceive( xUsart2Rx, &rx, delay );
	return rx;
}

char usart3_getcharWait(size_t delay)
{
	char rx = 0;

	/* Get the next character from the buffer.  Return false if no characters
	are available, or arrive before xBlockTime expires. */
	xQueueReceive( xUsart3Rx, &rx, delay );
	return rx;
}

char usart0_getchar()
{
	return usart0_getcharWait(portMAX_DELAY);
}

char usart1_getchar()
{
	return usart1_getcharWait(portMAX_DELAY);
}

char usart2_getchar()
{
	return usart2_getcharWait(portMAX_DELAY);
}

char usart3_getchar()
{
	return usart3_getcharWait(portMAX_DELAY);
}

void usart0_putchar(char c){
	if (TRACE_LEVEL){
		char buf[2];
		buf[0] = c;
		buf[1] = '\0';
		pr_debug(buf);
	}
//	USART_SendData(USART1, '!');
	xQueueSend( xUsart0Tx, &c, portMAX_DELAY );
	//Enable transmitter interrupt
	USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
}

void usart1_putchar(char c){
	if (TRACE_LEVEL){
		char buf[2];
		buf[0] = c;
		buf[1] = '\0';
		pr_debug(buf);
	}
	xQueueSend( xUsart1Tx, &c, portMAX_DELAY );
	//Enable transmitter interrupt
	USART_ITConfig(USART3, USART_IT_TXE, ENABLE);
}

void usart2_putchar(char c){
	if (TRACE_LEVEL){
		char buf[2];
		buf[0] = c;
		buf[1] = '\0';
		pr_debug(buf);
	}
	xQueueSend( xUsart2Tx, &c, portMAX_DELAY );
	//Enable transmitter interrupt
	USART_ITConfig(USART2, USART_IT_TXE, ENABLE);
}

void usart3_putchar(char c){
	if (TRACE_LEVEL){
		char buf[2];
		buf[0] = c;
		buf[1] = '\0';
		pr_debug(buf);
	}
	xQueueSend( xUsart3Tx, &c, portMAX_DELAY );
	//Enable transmitter interrupt
	USART_ITConfig(UART4, USART_IT_TXE, ENABLE);
}

void usart0_puts (const char* s )
{
	while ( *s ) usart0_putchar(*s++ );
}

void usart1_puts (const char* s )
{
	while ( *s ) usart1_putchar(*s++ );
}

void usart2_puts (const char* s )
{
	while ( *s ) usart2_putchar(*s++ );
}

void usart3_puts (const char* s )
{
	while ( *s ) usart3_putchar(*s++ );
}

int usart0_readLineWait(char *s, int len, size_t delay)
{
	int count = 0;
	while(count < len - 1){
		int c = usart0_getcharWait(delay);
		if (c == 0) break; //timeout
		*s++ = c;
		count++;
		if (c == '\n') break;
	}
	*s = '\0';
	return count;
}

int usart1_readLineWait(char *s, int len, size_t delay)
{
	int count = 0;
	while(count < len - 1){
		int c = usart1_getcharWait(delay);
		if (c == 0) break; //timeout
		*s++ = c;
		count++;
		if (c == '\n') break;
	}
	*s = '\0';
	return count;
}

int usart2_readLineWait(char *s, int len, size_t delay)
{
	int count = 0;
	while(count < len - 1){
		int c = usart2_getcharWait(delay);
		if (c == 0) break; //timeout
		*s++ = c;
		count++;
		if (c == '\n') break;
	}
	*s = '\0';
	return count;
}

int usart3_readLineWait(char *s, int len, size_t delay)
{
	int count = 0;
	while(count < len - 1){
		int c = usart3_getcharWait(delay);
		if (c == 0) break; //timeout
		*s++ = c;
		count++;
		if (c == '\n') break;
	}
	*s = '\0';
	return count;
}

int usart0_readLine(char *s, int len)
{
	return usart0_readLineWait(s,len,portMAX_DELAY);
}

int usart1_readLine(char *s, int len)
{
	return usart1_readLineWait(s,len,portMAX_DELAY);
}

int usart2_readLine(char *s, int len)
{
	return usart2_readLineWait(s,len,portMAX_DELAY);
}

int usart3_readLine(char *s, int len)
{
	return usart3_readLineWait(s,len,portMAX_DELAY);
}
