#include "usart_device.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "stm32f4xx_usart.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_misc.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_dma.h"
#include "printk.h"
#include "mem_mang.h"
#include "LED.h"

#define UART_QUEUE_LENGTH 	1024
#define GPS_BUFFER_SIZE		132

#define UART_WIRELESS_IRQ_PRIORITY 	7
#define UART_AUX_IRQ_PRIORITY 		8
#define UART_GPS_IRQ_PRIORITY 		5
#define UART_TELEMETRY_IRQ_PRIORITY 	6

#define DEFAULT_WIRELESS_BAUD_RATE	115200
#define DEFAULT_AUX_BAUD_RATE		115200
#define DEFAULT_TELEMETRY_BAUD_RATE	115200
#define DEFAULT_GPS_BAUD_RATE		921600

typedef enum {
    UART_RX_IRQ = 1,
    UART_TX_IRQ = 2
} uart_irq_type_t;

xQueueHandle xUsart0Tx;
xQueueHandle xUsart0Rx;

xQueueHandle xUsart1Tx;
xQueueHandle xUsart1Rx;

xQueueHandle xUsart2Tx;
xQueueHandle xUsart2Rx;

xQueueHandle xUsart3Tx;
xQueueHandle xUsart3Rx;

static uint8_t *gpsRxBuffer;

static int initQueues()
{
    gpsRxBuffer = (uint8_t *) portMalloc(sizeof(uint8_t) * GPS_BUFFER_SIZE);

    int success = 1;

    /* Create the queues used to hold Rx and Tx characters. */
    xUsart0Rx = xQueueCreate(UART_QUEUE_LENGTH,
                             (unsigned portBASE_TYPE)sizeof(signed portCHAR));
    xUsart0Tx = xQueueCreate(UART_QUEUE_LENGTH + 1,
                             (unsigned portBASE_TYPE)sizeof(signed portCHAR));
    if (xUsart0Rx == NULL || xUsart0Tx == NULL) {
        success = 0;
        goto cleanup_and_return;
    }


    xUsart1Rx = xQueueCreate(UART_QUEUE_LENGTH,
                             (unsigned portBASE_TYPE)sizeof(signed portCHAR));
    xUsart1Tx = xQueueCreate(UART_QUEUE_LENGTH + 1,
                             (unsigned portBASE_TYPE)sizeof(signed portCHAR));
    if (xUsart1Rx == NULL || xUsart1Tx == NULL) {
        success = 0;
        goto cleanup_and_return;
    }

    xUsart2Rx = xQueueCreate(UART_QUEUE_LENGTH,
                             (unsigned portBASE_TYPE)sizeof(signed portCHAR));
    xUsart2Tx = xQueueCreate(UART_QUEUE_LENGTH + 1,
                             (unsigned portBASE_TYPE)sizeof(signed portCHAR));
    if (xUsart2Rx == NULL || xUsart2Tx == NULL) {
        success = 0;
        goto cleanup_and_return;
    }

    xUsart3Rx = xQueueCreate(UART_QUEUE_LENGTH,
                             (unsigned portBASE_TYPE)sizeof(signed portCHAR));
    xUsart3Tx = xQueueCreate(UART_QUEUE_LENGTH + 1,
                             (unsigned portBASE_TYPE)sizeof(signed portCHAR));
    if (xUsart3Rx == NULL || xUsart3Tx == NULL) {
        success = 0;
        goto cleanup_and_return;
    }

cleanup_and_return:
    return success;
}

static void initUsart(USART_TypeDef * USARTx, unsigned int bits,
                      unsigned int parity, unsigned int stopBits,
                      unsigned int baud)
{
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
    case 2:
        stopBitsFlag = USART_StopBits_2;
        break;
    case 1:
    default:
        stopBitsFlag = USART_StopBits_1;
        break;
    }

    uint16_t parityFlag;
    switch (parity) {
    case 1:
        parityFlag = USART_Parity_Even;
        break;
    case 2:
        parityFlag = USART_Parity_Odd;
        break;
    case 0:
    default:
        parityFlag = USART_Parity_No;
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
}

int usart_device_init()
{
    if (!initQueues()) {
        return 0;
    }

    usart_device_init_0(8, 0, 1, DEFAULT_WIRELESS_BAUD_RATE);	//wireless
    usart_device_init_1(8, 0, 1, DEFAULT_AUX_BAUD_RATE);	//auxilary
    usart_device_init_2(8, 0, 1, DEFAULT_GPS_BAUD_RATE);	//GPS
    usart_device_init_3(8, 0, 1, DEFAULT_TELEMETRY_BAUD_RATE);	//telemetry
    return 1;
}

void usart_device_config(uart_id_t port, uint8_t bits, uint8_t parity,
                         uint8_t stopbits, uint32_t baud)
{
    switch (port) {
    case UART_WIRELESS:
        initUsart(USART1, bits, parity, stopbits, baud);
        break;
    case UART_AUX:
        initUsart(USART3, bits, parity, stopbits, baud);
        break;
    case UART_GPS:
        initUsart(USART2, bits, parity, stopbits, baud);
        break;
    case UART_TELEMETRY:
        initUsart(UART4, bits, parity, stopbits, baud);
        break;
    default:
        break;
    }
}

int usart_device_init_serial(Serial * serial, uart_id_t id)
{
    int rc = 1;

    switch (id) {
    case UART_WIRELESS:
        serial->init = &usart_device_init_0;
        serial->flush = &usart0_flush;
        serial->get_c = &usart0_getchar;
        serial->get_c_wait = &usart0_getcharWait;
        serial->get_line = &usart0_readLine;
        serial->get_line_wait = &usart0_readLineWait;
        serial->put_c = &usart0_putchar;
        serial->put_s = &usart0_puts;
        break;

    case UART_AUX:
        serial->init = &usart_device_init_1;
        serial->flush = &usart1_flush;
        serial->get_c = &usart1_getchar;
        serial->get_c_wait = &usart1_getcharWait;
        serial->get_line = &usart1_readLine;
        serial->get_line_wait = &usart1_readLineWait;
        serial->put_c = &usart1_putchar;
        serial->put_s = &usart1_puts;
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
        break;

    case UART_TELEMETRY:
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

static void initGPIO(GPIO_TypeDef * GPIOx, uint32_t gpioPins)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = gpioPins;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOx, &GPIO_InitStructure);
}

static void enableRxDMA(uint32_t RCC_AHB1Periph,
                        DMA_Stream_TypeDef * DMA_stream, uint32_t DMA_channel,
                        uint8_t * rxBuffer, uint32_t rxBufferSize,
                        USART_TypeDef * USARTx, uint8_t NVIC_IRQ_channel,
                        uint8_t IRQ_priority)
{
    /* Configure the Priority Group to 2 bits */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

    NVIC_InitTypeDef NVIC_InitStructure;
    /* Enable the USARTx RX DMA Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = NVIC_IRQ_channel;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = IRQ_priority;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* DMA configuration ------------------------------------------------------ */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph, ENABLE);
    DMA_InitTypeDef DMA_InitStructure;
    DMA_DeInit(DMA_stream);
    DMA_InitStructure.DMA_Channel = DMA_channel;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;	// Receive
    DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t) rxBuffer;
    DMA_InitStructure.DMA_BufferSize = rxBufferSize;
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) & USARTx->DR;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable;
    DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
    DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
    DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;

    DMA_Init(DMA_stream, &DMA_InitStructure);

    /* Enable the USART Rx DMA request */
    USART_DMACmd(USARTx, USART_DMAReq_Rx, ENABLE);

    /* Enable DMA Stream Half Transfer and Transfer Complete interrupt */
    DMA_ITConfig(DMA_stream, DMA_IT_TC, ENABLE);
    DMA_ITConfig(DMA_stream, DMA_IT_HT, ENABLE);

    /* Enable the DMA RX Stream */
    DMA_Cmd(DMA_stream, ENABLE);
}

static void enableRxTxIrq(USART_TypeDef * USARTx, uint8_t usartIrq,
                          uint8_t IRQ_priority, uart_irq_type_t irqType)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    /* Configure the NVIC Preemption Priority Bits */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

    /* Enable the USART Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = usartIrq;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = IRQ_priority;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    if (irqType | UART_RX_IRQ)
        USART_ITConfig(USARTx, USART_IT_RXNE, ENABLE);
    if (irqType | UART_TX_IRQ)
        USART_ITConfig(USARTx, USART_IT_TXE, ENABLE);
}

//Wireless port
void usart_device_init_0(unsigned int bits, unsigned int parity,
                         unsigned int stopBits, unsigned int baud)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    initGPIO(GPIOA, (GPIO_Pin_9 | GPIO_Pin_10));
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);

    enableRxTxIrq(USART1, USART1_IRQn, UART_WIRELESS_IRQ_PRIORITY,
                  (UART_RX_IRQ | UART_TX_IRQ));

    initUsart(USART1, bits, parity, stopBits, baud);
}

//Auxilary port
void usart_device_init_1(unsigned int bits, unsigned int parity,
                         unsigned int stopBits, unsigned int baud)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

    initGPIO(GPIOD, (GPIO_Pin_8 | GPIO_Pin_9));
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource8, GPIO_AF_USART3);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource9, GPIO_AF_USART3);

    enableRxTxIrq(USART3, USART3_IRQn, UART_AUX_IRQ_PRIORITY,
                  (UART_RX_IRQ | UART_TX_IRQ));

    initUsart(USART3, bits, parity, stopBits, baud);
}

//GPS port
void usart_device_init_2(unsigned int bits, unsigned int parity,
                         unsigned int stopBits, unsigned int baud)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

    initGPIO(GPIOD, (GPIO_Pin_5 | GPIO_Pin_6));
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource5, GPIO_AF_USART2);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource6, GPIO_AF_USART2);

    initUsart(USART2, bits, parity, stopBits, baud);
    enableRxTxIrq(USART2, USART2_IRQn, UART_GPS_IRQ_PRIORITY, UART_TX_IRQ);
    enableRxDMA(RCC_AHB1Periph_DMA1, DMA1_Stream5, DMA_Channel_4,
                gpsRxBuffer, GPS_BUFFER_SIZE, USART2, DMA1_Stream5_IRQn,
                UART_GPS_IRQ_PRIORITY);
}

//Telemetry port
void usart_device_init_3(unsigned int bits, unsigned int parity,
                         unsigned int stopBits, unsigned int baud)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    initGPIO(GPIOA, (GPIO_Pin_0 | GPIO_Pin_1));
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource0, GPIO_AF_UART4);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource1, GPIO_AF_UART4);

    enableRxTxIrq(UART4, UART4_IRQn, UART_TELEMETRY_IRQ_PRIORITY,
                  (UART_RX_IRQ | UART_TX_IRQ));

    initUsart(UART4, bits, parity, stopBits, baud);
}

////////////////////////////////////////////////////////////////////////////
// Communication functions
////////////////////////////////////////////////////////////////////////////

void usart0_flush(void)
{
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

int usart0_getcharWait(char *c, size_t delay)
{
    return xQueueReceive(xUsart0Rx, c, delay) == pdTRUE ? 1 : 0;
}

int usart1_getcharWait(char *c, size_t delay)
{
    return xQueueReceive(xUsart1Rx, c, delay) == pdTRUE ? 1 : 0;
}

int usart2_getcharWait(char *c, size_t delay)
{
    return xQueueReceive(xUsart2Rx, c, delay) == pdTRUE ? 1 : 0;
}

int usart3_getcharWait(char *c, size_t delay)
{
    return xQueueReceive(xUsart3Rx, c, delay) == pdTRUE ? 1 : 0;
}

char usart0_getchar()
{
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

void usart0_putchar(char c)
{
    if (TRACE_LEVEL) {
        char buf[2];
        buf[0] = c;
        buf[1] = '\0';
        pr_debug(buf);
    }

    xQueueSend(xUsart0Tx, &c, portMAX_DELAY);

    //Enable transmitter interrupt
    USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
}

void usart1_putchar(char c)
{
    if (TRACE_LEVEL) {
        char buf[2];
        buf[0] = c;
        buf[1] = '\0';
        pr_debug(buf);
    }

    xQueueSend(xUsart1Tx, &c, portMAX_DELAY);

    //Enable transmitter interrupt
    USART_ITConfig(USART3, USART_IT_TXE, ENABLE);
}

void usart2_putchar(char c)
{
    if (TRACE_LEVEL) {
        char buf[2];
        buf[0] = c;
        buf[1] = '\0';
        pr_debug(buf);
    }

    xQueueSend(xUsart2Tx, &c, portMAX_DELAY);

    //Enable transmitter interrupt
    USART_ITConfig(USART2, USART_IT_TXE, ENABLE);
}

void usart3_putchar(char c)
{
    if (TRACE_LEVEL) {
        char buf[2];
        buf[0] = c;
        buf[1] = '\0';
        pr_debug(buf);
    }

    xQueueSend(xUsart3Tx, &c, portMAX_DELAY);

    //Enable transmitter interrupt
    USART_ITConfig(UART4, USART_IT_TXE, ENABLE);
}

void usart0_puts(const char *s)
{
    while (*s)
        usart0_putchar(*s++);
}

void usart1_puts(const char *s)
{
    while (*s)
        usart1_putchar(*s++);
}

void usart2_puts(const char *s)
{
    while (*s)
        usart2_putchar(*s++);
}

void usart3_puts(const char *s)
{
    while (*s)
        usart3_putchar(*s++);
}

int usart0_readLineWait(char *s, int len, size_t delay)
{
    int count = 0;
    while (count < len - 1) {
        char c = 0;

        if (!usart0_getcharWait(&c, delay)) {
            break;
        }

        *s++ = c;
        count++;
        if (c == '\n')
            break;
    }
    *s = '\0';
    return count;
}

int usart1_readLineWait(char *s, int len, size_t delay)
{
    int count = 0;
    while (count < len - 1) {
        char c = 0;

        if (!usart1_getcharWait(&c, delay)) {
            break;
        }

        *s++ = c;
        count++;
        if (c == '\n')
            break;
    }
    *s = '\0';
    return count;
}

int usart2_readLineWait(char *s, int len, size_t delay)
{
    int count = 0;
    while (count < len - 1) {
        char c = 0;

        if (!usart2_getcharWait(&c, delay)) {
            break;
        }

        *s++ = c;
        count++;
        if (c == '\n')
            break;
    }
    *s = '\0';
    return count;
}

int usart3_readLineWait(char *s, int len, size_t delay)
{
    int count = 0;
    while (count < len - 1) {
        char c = 0;

        if (!usart3_getcharWait(&c, delay)) {
            break;
        }

        *s++ = c;
        count++;
        if (c == '\n')
            break;
    }
    *s = '\0';
    return count;
}

int usart0_readLine(char *s, int len)
{
    return usart0_readLineWait(s, len, portMAX_DELAY);
}

int usart1_readLine(char *s, int len)
{
    return usart1_readLineWait(s, len, portMAX_DELAY);
}

int usart2_readLine(char *s, int len)
{
    return usart2_readLineWait(s, len, portMAX_DELAY);
}

int usart3_readLine(char *s, int len)
{
    return usart3_readLineWait(s, len, portMAX_DELAY);
}

////////////////////////////////////////////////////////////////////////////
// Interrupt Handlers
////////////////////////////////////////////////////////////////////////////

static void handle_usart_overrun(USART_TypeDef* USARTx)
{
    uint32_t cChar;
    if (USART_GetITStatus(USARTx, USART_IT_ORE_RX) != SET)
    	return;
	/*
	 * Handle Overrun error
	 * This bit is set by hardware when the word currently being received in the shift register is
	 * ready to be transferred into the RDR register while RXNE=1. An interrupt is generated if
	 * RXNEIE=1 in the USART_CR1 register. It is cleared by a software sequence (an read to the
	 * USART_SR register followed by a read to the USART_DR register)
	 */
	cChar = USART1->SR;
	cChar = USART1->DR;
	/* Suppress compiler warning */
	cChar = cChar;
}

void DMA1_Stream5_IRQHandler(void)
{
    portBASE_TYPE xTaskWokenByPost = pdFALSE;
    signed portCHAR cChar;
    /* Test on DMA Stream Transfer Complete interrupt */
    if (DMA_GetITStatus(DMA1_Stream5, DMA_IT_TCIF5)) {
        /* Clear DMA Stream Transfer Complete interrupt pending bit */
        DMA_ClearITPendingBit(DMA1_Stream5, DMA_IT_TCIF5);
        for (size_t i = GPS_BUFFER_SIZE / 2; i < GPS_BUFFER_SIZE; i++) {
            cChar = gpsRxBuffer[i];
            xQueueSendFromISR(xUsart2Rx, &cChar, &xTaskWokenByPost);
        }
    }

    /* Test on DMA Stream Half Transfer interrupt */
    if (DMA_GetITStatus(DMA1_Stream5, DMA_IT_HTIF5)) {
        /* Clear DMA Stream Half Transfer interrupt pending bit */
        DMA_ClearITPendingBit(DMA1_Stream5, DMA_IT_HTIF5);
        for (size_t i = 0; i < GPS_BUFFER_SIZE / 2; i++) {
            cChar = gpsRxBuffer[i];
            xQueueSendFromISR(xUsart2Rx, &cChar, &xTaskWokenByPost);
        }
    }
    portEND_SWITCHING_ISR(xTaskWokenByPost);
}

void USART1_IRQHandler(void)
{
    portBASE_TYPE xTaskWokenByTx = pdFALSE, xTaskWokenByPost = pdFALSE;
    signed portCHAR cChar;

    if (USART_GetITStatus(USART1, USART_IT_TXE) != RESET) {
        /* The interrupt was caused by the TX becoming empty.  Are there any more characters to transmit? */
        if (xQueueReceiveFromISR(xUsart0Tx, &cChar, &xTaskWokenByTx) ==
            pdTRUE) {
            // A character was retrieved from the queue so can be sent to the USART
            USART_SendData(USART1, cChar);
        } else {
            /* Queue empty, nothing to send so turn off the Tx interrupt. */
            USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
        }
    }

    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
        /* The interrupt was caused by a character being received.  Grab the
           character from the rx and place it in the queue or received
           characters. */
        cChar = USART_ReceiveData(USART1);
        xQueueSendFromISR(xUsart0Rx, &cChar, &xTaskWokenByPost);
    }

    handle_usart_overrun(USART1);

    /* If a task was woken by either a character being received or a character
       being transmitted then we may need to switch to another task. */
    portEND_SWITCHING_ISR(xTaskWokenByPost || xTaskWokenByTx);
}

void USART2_IRQHandler(void)
{
    portBASE_TYPE xTaskWokenByTx = pdFALSE, xTaskWokenByPost = pdFALSE;
    signed portCHAR cChar;

    if (USART_GetITStatus(USART2, USART_IT_TXE) != RESET) {
        /* The interrupt was caused by the TX becoming empty.  Are there any more characters to transmit? */
        if (xQueueReceiveFromISR(xUsart2Tx, &cChar, &xTaskWokenByTx) ==
            pdTRUE) {
            // A character was retrieved from the queue so can be sent to the USART
            USART_SendData(USART2, cChar);
        } else {
            /* Queue empty, nothing to send so turn off the Tx interrupt. */
            USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
        }
    }

    if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) {
        /* The interrupt was caused by a character being received.  Grab the
        character from the rx and place it in the queue or received
        characters. */
//		cChar = USART_ReceiveData(USART2);
//		xQueueSendFromISR( xUsart2Rx, &cChar, &xTaskWokenByPost );
    }

    handle_usart_overrun(USART2);

    /* If a task was woken by either a character being received or a character
       being transmitted then we may need to switch to another task. */
    portEND_SWITCHING_ISR(xTaskWokenByPost || xTaskWokenByTx);
}

void USART3_IRQHandler(void)
{
    portBASE_TYPE xTaskWokenByTx = pdFALSE, xTaskWokenByPost = pdFALSE;
    signed portCHAR cChar;

    if (USART_GetITStatus(USART3, USART_IT_TXE) != RESET) {
        /* The interrupt was caused by the TX becoming empty.  Are there any more characters to transmit? */
        if (xQueueReceiveFromISR(xUsart1Tx, &cChar, &xTaskWokenByTx) ==
            pdTRUE) {
            // A character was retrieved from the queue so can be sent to the USART
            USART_SendData(USART3, cChar);
        } else {
            /* Queue empty, nothing to send so turn off the Tx interrupt. */
            USART_ITConfig(USART3, USART_IT_TXE, DISABLE);
        }
    }

    if (USART_GetITStatus(USART3, USART_IT_RXNE) != RESET) {
        /* The interrupt was caused by a character being received.  Grab the
           character from the rx and place it in the queue or received
           characters. */
        cChar = USART_ReceiveData(USART3);
        xQueueSendFromISR(xUsart1Rx, &cChar, &xTaskWokenByPost);
    }

    handle_usart_overrun(USART3);

    /* If a task was woken by either a character being received or a character
       being transmitted then we may need to switch to another task. */
    portEND_SWITCHING_ISR(xTaskWokenByPost || xTaskWokenByTx);
}

void UART4_IRQHandler(void)
{
    portBASE_TYPE xTaskWokenByTx = pdFALSE, xTaskWokenByPost = pdFALSE;
    signed portCHAR cChar;

    if (USART_GetITStatus(UART4, USART_IT_TXE) != RESET) {
        /* The interrupt was caused by the TX becoming empty.  Are there any more characters to transmit? */
        if (xQueueReceiveFromISR(xUsart3Tx, &cChar, &xTaskWokenByTx) ==
            pdTRUE) {
            // A character was retrieved from the queue so can be sent to the USART
            USART_SendData(UART4, cChar);
        } else {
            /* Queue empty, nothing to send so turn off the Tx interrupt. */
            USART_ITConfig(UART4, USART_IT_TXE, DISABLE);
        }
    }
    if (USART_GetITStatus(UART4, USART_IT_RXNE) != RESET) {
        /* The interrupt was caused by a character being received.  Grab the
           character from the rx and place it in the queue or received
           characters. */
        cChar = USART_ReceiveData(UART4);
        xQueueSendFromISR(xUsart3Rx, &cChar, &xTaskWokenByPost);
    }

    handle_usart_overrun(UART4);

    /* If a task was woken by either a character being received or a character
       being transmitted then we may need to switch to another task. */
    portEND_SWITCHING_ISR(xTaskWokenByPost || xTaskWokenByTx);
}
