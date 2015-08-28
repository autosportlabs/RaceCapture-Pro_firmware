#include "usart_device.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "stm32f30x_usart.h"
#include "stm32f30x_gpio.h"
#include "stm32f30x_misc.h"
#include "stm32f30x_rcc.h"
#include "stm32f30x_dma.h"
#include "printk.h"
#include "mem_mang.h"
#include "LED.h"

#define UART_QUEUE_LENGTH 	1024
#define GPS_BUFFER_SIZE		132

#define UART_WIRELESS_IRQ_PRIORITY 	7
#define UART_GPS_IRQ_PRIORITY 		5
#define UART_TELEMETRY_IRQ_PRIORITY 	6

#define DEFAULT_WIRELESS_BAUD_RATE	9600
#define DEFAULT_TELEMETRY_BAUD_RATE	115200
#define DEFAULT_GPS_BAUD_RATE		9600

typedef enum {
    UART_RX_IRQ = 1,
    UART_TX_IRQ = 2
} uart_irq_type_t;

xQueueHandle xUsart0Tx;
xQueueHandle xUsart0Rx;

xQueueHandle xUsart2Tx;
xQueueHandle xUsart2Rx;

xQueueHandle xUsart3Tx;
xQueueHandle xUsart3Rx;

static uint8_t gpsRxBuffer[GPS_BUFFER_SIZE];

static int initQueues()
{
    //gpsRxBuffer = (uint8_t *) portMalloc(sizeof(uint8_t) * GPS_BUFFER_SIZE);

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


static void enableRxDMA(uint32_t RCC_AHBPeriph,
                        DMA_Channel_TypeDef* DMA_channel,
                        uint8_t * rxBuffer, uint32_t rxBufferSize,
                        USART_TypeDef * USARTx, uint8_t NVIC_IRQ_channel,
                        uint8_t IRQ_priority)
{

    // Enable RX DMA Transfers from USARTx.
    USART_DMACmd(USARTx, USART_DMAReq_Rx, ENABLE);

    // Enable DMAx Controller.
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph, ENABLE);

    // Clear USARTx RX DMA Channel config.
    DMA_DeInit(DMA_channel);

    // Initialize USART2 RX DMA Channel:
    DMA_InitTypeDef DMA_InitStruct;
    DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t) & USARTx->RDR;         // USART2 RX Data Register.
    DMA_InitStruct.DMA_MemoryBaseAddr = (uint32_t)rxBuffer;                 // Copy data to RxBuffer.
    DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralSRC;                         // Peripheral as source, memory as destination.
    DMA_InitStruct.DMA_BufferSize = rxBufferSize;                           // Defined above.
    DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;           // No increment on RDR address.
    DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;                    // Increment memory address.
    DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;    // Byte-wise copy.
    DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;            // Byte-wise copy.
    DMA_InitStruct.DMA_Mode = DMA_Mode_Circular;                            // Ring buffer - don't interrupt when at end of memory region.
    DMA_InitStruct.DMA_Priority = DMA_Priority_High;                        // High priority.
    DMA_InitStruct.DMA_M2M = DMA_M2M_Disable;                               // Peripheral to memory, not M2M.

    // Initialize USART2 RX DMA Channel.
    DMA_Init(DMA_channel, &DMA_InitStruct);

    // Enable Transfer Complete, Half Transfer and Transfer Error interrupts.
    DMA_ITConfig(DMA_channel, DMA_IT_TC | DMA_IT_HT, ENABLE);

    // Enable USART2 RX DMA Channel.
    DMA_Cmd(DMA_channel, ENABLE);


    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

     // Configure USART2 RX DMA Channel Interrupts.
     NVIC_InitTypeDef NVIC_InitStruct;
     NVIC_InitStruct.NVIC_IRQChannel = NVIC_IRQ_channel;
     NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
     NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1; //IRQ_priority;
     NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;

     NVIC_Init(&NVIC_InitStruct);

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

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

    initGPIO(GPIOA, (GPIO_Pin_9 | GPIO_Pin_10));
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_7);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_7);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    initUsart(USART1, bits, parity, stopBits, baud);

    enableRxTxIrq(USART1, USART1_IRQn, UART_WIRELESS_IRQ_PRIORITY,
                  (UART_RX_IRQ | UART_TX_IRQ));
}

//GPS port
void usart_device_init_2(unsigned int bits, unsigned int parity,
                         unsigned int stopBits, unsigned int baud)
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
    initGPIO(GPIOB, (GPIO_Pin_4 | GPIO_Pin_3));
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource4, GPIO_AF_7);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource3, GPIO_AF_7);

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    initUsart(USART2, bits, parity, stopBits, baud);

    /* Note, only transmit interrupt is enabled */
    enableRxTxIrq(USART2, USART2_IRQn, UART_GPS_IRQ_PRIORITY, (UART_TX_IRQ));

    enableRxDMA(RCC_AHBPeriph_DMA1, DMA1_Channel6,
                gpsRxBuffer, GPS_BUFFER_SIZE, USART2, DMA1_Channel6_IRQn,
                UART_GPS_IRQ_PRIORITY);
}

//Telemetry port
void usart_device_init_3(unsigned int bits, unsigned int parity,
                         unsigned int stopBits, unsigned int baud)
{
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
    initGPIO(GPIOB, (GPIO_Pin_10 | GPIO_Pin_11));
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_7);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_7);

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
    initUsart(UART4, bits, parity, stopBits, baud);

    enableRxTxIrq(USART3, UART4_IRQn, UART_TELEMETRY_IRQ_PRIORITY,
                  (UART_RX_IRQ | UART_TX_IRQ));

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

void DMA1_Channel6_IRQHandler(void)
{
    portBASE_TYPE xTaskWokenByPost = pdFALSE;
    signed portCHAR cChar;
    /* Test on DMA Stream Transfer Complete interrupt */
    if (DMA_GetITStatus(DMA1_IT_TC6)) {
        /* Clear DMA Stream Transfer Complete interrupt pending bit */
        DMA_ClearITPendingBit(DMA1_IT_TC6);
        for (size_t i = GPS_BUFFER_SIZE / 2; i < GPS_BUFFER_SIZE; i++) {
            cChar = gpsRxBuffer[i];
            xQueueSendFromISR(xUsart2Rx, &cChar, &xTaskWokenByPost);
        }
    }

    /* Test on DMA Stream Half Transfer interrupt */
    if (DMA_GetITStatus(DMA1_IT_HT6)) {
        /* Clear DMA Stream Half Transfer interrupt pending bit */
        DMA_ClearITPendingBit(DMA1_IT_HT6);
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

    unsigned int ISR = USART1->ISR;

    if (ISR & USART_FLAG_TXE) {
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

    if (ISR & USART_FLAG_RXNE) {
        /* The interrupt was caused by a character being received.  Grab the
           character from the rx and place it in the queue or received
           characters. */
        cChar = USART_ReceiveData(USART1);
        xQueueSendFromISR(xUsart0Rx, &cChar, &xTaskWokenByPost);
    }

    if (ISR & USART_FLAG_ORE) {
            USART_ClearITPendingBit (USART1, USART_IT_ORE);
    }

    /* If a task was woken by either a character being received or a character
       being transmitted then we may need to switch to another task. */
    portEND_SWITCHING_ISR(xTaskWokenByPost || xTaskWokenByTx);
}

void USART2_IRQHandler(void)
{
    portBASE_TYPE xTaskWokenByTx = pdFALSE, xTaskWokenByPost = pdFALSE;
    signed portCHAR cChar;

    unsigned int ISR = USART2->ISR;

    if (ISR & USART_FLAG_TXE) {
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

    if (ISR & USART_FLAG_RXNE) {
        /* The interrupt was caused by a character being received.  Grab the
        character from the rx and place it in the queue or received
        characters. */
		cChar = USART_ReceiveData(USART2);
		xQueueSendFromISR( xUsart2Rx, &cChar, &xTaskWokenByPost );
    }

    if (ISR & USART_FLAG_ORE) {
            USART_ClearITPendingBit (USART1, USART_IT_ORE);
    }

    /* If a task was woken by either a character being received or a character
       being transmitted then we may need to switch to another task. */
    portEND_SWITCHING_ISR(xTaskWokenByPost || xTaskWokenByTx);
}

void USART3_IRQHandler(void)
{
    portBASE_TYPE xTaskWokenByTx = pdFALSE, xTaskWokenByPost = pdFALSE;
    signed portCHAR cChar;

    unsigned int ISR = USART3->ISR;

    if (ISR & USART_FLAG_TXE) {
        /* The interrupt was caused by the TX becoming empty.  Are there any more characters to transmit? */
        if (xQueueReceiveFromISR(xUsart3Tx, &cChar, &xTaskWokenByTx) ==
            pdTRUE) {
            // A character was retrieved from the queue so can be sent to the USART
            USART_SendData(USART3, cChar);
        } else {
            /* Queue empty, nothing to send so turn off the Tx interrupt. */
            USART_ITConfig(USART3, USART_IT_TXE, DISABLE);
        }
    }

    if (ISR & USART_FLAG_RXNE) {
        /* The interrupt was caused by a character being received.  Grab the
           character from the rx and place it in the queue or received
           characters. */
        cChar = USART_ReceiveData(USART3);
        xQueueSendFromISR(xUsart3Rx, &cChar, &xTaskWokenByPost);
    }

    if (ISR & USART_FLAG_ORE) {
               USART_ClearITPendingBit (USART1, USART_IT_ORE);
    }

    /* If a task was woken by either a character being received or a character
       being transmitted then we may need to switch to another task. */
    portEND_SWITCHING_ISR(xTaskWokenByPost || xTaskWokenByTx);
}
