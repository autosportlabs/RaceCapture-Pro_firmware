/*
 * Race Capture Firmware
 *
 * Copyright (C) 2016 Autosport Labs
 *
 * This file is part of the Race Capture firmware suite
 *
 * This is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details. You should
 * have received a copy of the GNU General Public License along with
 * this code. If not, see <http://www.gnu.org/licenses/>.
 */

#include "FreeRTOS.h"
#include "led.h"
#include "mem_mang.h"
#include "panic.h"
#include "printk.h"
#include "queue.h"
#include "serial.h"
#include "serial_device.h"
#include "stm32f4xx_dma.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_misc.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_usart.h"
#include "task.h"
#include "usart_device.h"

#define UART_QUEUE_LEN 	1024
#define GPS_BUFFER_SIZE	132

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

static uint8_t *gpsRxBuffer;

static struct usart_info {
        xQueueHandle tx;
        xQueueHandle rx;
        struct Serial *serial;
        USART_TypeDef *usart;
} usart_data[__UART_COUNT];


static void init_usart(USART_TypeDef *USARTx, const size_t bits,
                       const size_t parity, const size_t stop_bits,
                       const size_t baud)
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
        switch (stop_bits) {
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

/* Wireless port */
static void usart_device_init_0(size_t bits, size_t parity,
                                size_t stopBits, size_t baud)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    initGPIO(GPIOA, (GPIO_Pin_9 | GPIO_Pin_10));
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);

    enableRxTxIrq(USART1, USART1_IRQn, UART_WIRELESS_IRQ_PRIORITY,
                  (UART_RX_IRQ | UART_TX_IRQ));

    init_usart(USART1, bits, parity, stopBits, baud);
}

/* Auxilary port */
static void usart_device_init_1(size_t bits, size_t parity,
                                size_t stopBits, size_t baud)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

    initGPIO(GPIOD, (GPIO_Pin_8 | GPIO_Pin_9));
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource8, GPIO_AF_USART3);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource9, GPIO_AF_USART3);

    enableRxTxIrq(USART3, USART3_IRQn, UART_AUX_IRQ_PRIORITY,
                  (UART_RX_IRQ | UART_TX_IRQ));

    init_usart(USART3, bits, parity, stopBits, baud);
}

/* GPS port */
static void usart_device_init_2(size_t bits, size_t parity,
                                size_t stopBits, size_t baud)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

    initGPIO(GPIOD, (GPIO_Pin_5 | GPIO_Pin_6));
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource5, GPIO_AF_USART2);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource6, GPIO_AF_USART2);

    init_usart(USART2, bits, parity, stopBits, baud);
    enableRxTxIrq(USART2, USART2_IRQn, UART_GPS_IRQ_PRIORITY, UART_TX_IRQ);
    enableRxDMA(RCC_AHB1Periph_DMA1, DMA1_Stream5, DMA_Channel_4,
                gpsRxBuffer, GPS_BUFFER_SIZE, USART2, DMA1_Stream5_IRQn,
                UART_GPS_IRQ_PRIORITY);
}

/* Telemetry port */
static void usart_device_init_3(size_t bits, size_t parity,
                                size_t stopBits, size_t baud)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    initGPIO(GPIOA, (GPIO_Pin_0 | GPIO_Pin_1));
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource0, GPIO_AF_UART4);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource1, GPIO_AF_UART4);

    enableRxTxIrq(UART4, UART4_IRQn, UART_TELEMETRY_IRQ_PRIORITY,
                  (UART_RX_IRQ | UART_TX_IRQ));

    init_usart(UART4, bits, parity, stopBits, baud);
}

static bool _config_cb(void *cfg_cb_arg, const size_t bits,
                       const size_t parity, const size_t stop_bits,
                       const size_t baud)
{
        USART_TypeDef *usart = cfg_cb_arg;
        init_usart(usart, bits, parity, stop_bits, baud);
        return true;
}

static void _char_tx_cb(xQueueHandle queue, void *post_tx_arg)
{
        USART_TypeDef *usart = (USART_TypeDef*) post_tx_arg;
        /* Set the interrupt Tx Flag */
        USART_ITConfig(usart, USART_IT_TXE, ENABLE);
}

static bool init_usart_serial(const uart_id_t uart_id, USART_TypeDef *usart,
                              const serial_id_t serial_id, const char *name)
{
        struct Serial *s =
                serial_create(name, UART_QUEUE_LEN, UART_QUEUE_LEN,
                              _config_cb, usart, _char_tx_cb, usart);
        if (!s) {
                pr_error("[USART] Serial Malloc failure!\r\n");
                return false;
        }

        /* Set the usart_info data */
        struct usart_info *ui = usart_data + uart_id;
        ui->rx = serial_get_rx_queue(s);
        ui->tx = serial_get_tx_queue(s);
        ui->usart = usart;
        ui->serial = s;

        return true;
}

int usart_device_init()
{
        /* Must be malloc b/c will be used for DMA.  DMA requires heap mem */
        gpsRxBuffer = (uint8_t *) portMalloc(GPS_BUFFER_SIZE);

        /*
         * NOTE: Careful with the mappings here.  If you change them
         *       ensure that you also update the values in the IRQ
         *       handlers below.
         */
        const bool mem_alloc_success =  gpsRxBuffer &&
                init_usart_serial(UART_GPS, USART2,
                                  SERIAL_GPS, "GPS") &&
                init_usart_serial(UART_TELEMETRY, UART4,
                                  SERIAL_TELEMETRY, "Cell") &&
                init_usart_serial(UART_WIRELESS, USART1,
                                  SERIAL_WIRELESS, "BT") &&
                init_usart_serial(UART_AUX, USART3, SERIAL_AUX, "Aux");

        if (!mem_alloc_success) {
                pr_error("[USART] Failed to init\r\n");
                panic(PANIC_CAUSE_MALLOC);
        }

        usart_device_init_0(8, 0, 1, DEFAULT_WIRELESS_BAUD_RATE);
        usart_device_init_1(8, 0, 1, DEFAULT_AUX_BAUD_RATE);
        usart_device_init_2(8, 0, 1, DEFAULT_GPS_BAUD_RATE);
        usart_device_init_3(8, 0, 1, DEFAULT_TELEMETRY_BAUD_RATE);

        return 1;
}

static bool usart_id_in_bounds(const uart_id_t id)
{
        return ((size_t) id) < __UART_COUNT;
}

struct Serial* usart_device_get_serial(const uart_id_t id)
{
        if (!usart_id_in_bounds(id))
                return NULL;

        struct usart_info *ui = usart_data + id;
        return ui->serial;
}

void usart_device_config(const uart_id_t id, const size_t bits,
                         const size_t parity, const size_t stop_bits,
                         const size_t baud)
{
        if (!usart_id_in_bounds(id))
                return;

        struct usart_info *ui = usart_data + id;
        init_usart(ui->usart, bits, parity, stop_bits, baud);
}

/* *** Interrupt Handlers *** */

static void handle_usart_overrun(USART_TypeDef* USARTx)
{
        if (USART_GetITStatus(USARTx, USART_IT_ORE_RX) != SET)
                return;

        /*
	 * Handle Overrun error
         *
	 * This bit is set by hardware when the word currently being received in
         * the shift register is ready to be transferred into the RDR register
         * while RXNE=1. An interrupt is generated if RXNEIE=1 in the USART_CR1
         * register. It is cleared by a software sequence (an read to the
         * USART_SR register followed by a read to the USART_DR register)
	 */
        uint32_t cChar;
	cChar = USART1->SR;
	cChar = USART1->DR;

	/* Suppress compiler warning */
	(void) cChar;
}

void DMA1_Stream5_IRQHandler(void)
{
        struct usart_info *ui = usart_data + UART_GPS;
        portBASE_TYPE xTaskWokenByPost = pdFALSE;
        signed portCHAR cChar;

        /* Test on DMA Stream Transfer Complete interrupt */
        if (DMA_GetITStatus(DMA1_Stream5, DMA_IT_TCIF5)) {
                /* Clear DMA Stream Transfer Complete interrupt pending bit */
                DMA_ClearITPendingBit(DMA1_Stream5, DMA_IT_TCIF5);
                for (size_t i = GPS_BUFFER_SIZE / 2; i < GPS_BUFFER_SIZE; i++) {
                        cChar = gpsRxBuffer[i];
                        xQueueSendFromISR(ui->rx, &cChar, &xTaskWokenByPost);
                }
        }

        /* Test on DMA Stream Half Transfer interrupt */
        if (DMA_GetITStatus(DMA1_Stream5, DMA_IT_HTIF5)) {
                /* Clear DMA Stream Half Transfer interrupt pending bit */
                DMA_ClearITPendingBit(DMA1_Stream5, DMA_IT_HTIF5);
                for (size_t i = 0; i < GPS_BUFFER_SIZE / 2; i++) {
                        cChar = gpsRxBuffer[i];
                        xQueueSendFromISR(ui->rx, &cChar, &xTaskWokenByPost);
                }
        }

        portEND_SWITCHING_ISR(xTaskWokenByPost);
}

static void usart_generic_irq_handler(USART_TypeDef *usart,
                                      xQueueHandle rx,
                                      xQueueHandle tx)
{
        portBASE_TYPE xTaskWokenByTx = pdFALSE, xTaskWokenByPost = pdFALSE;
        signed portCHAR cChar;

        if (USART_GetITStatus(usart, USART_IT_TXE) != RESET) {
                /*
                 * The interrupt was caused by the TX becoming empty.
                 * Are there any more characters to transmit?
                 */
                if (tx != NULL &&
                    pdTRUE == xQueueReceiveFromISR(tx, &cChar,
                                                   &xTaskWokenByTx)) {
                        /*
                         * A character was retrieved from the queue so
                         * can be sent to the USART.
                         */
                        USART_SendData(usart, cChar);
                } else {
                        /*
                         * Queue empty, nothing to send so turn off the
                         * Tx interrupt.
                         */
                        USART_ITConfig(usart, USART_IT_TXE, DISABLE);
                }
        }

        if (USART_GetITStatus(usart, USART_IT_RXNE) != RESET) {
                /*
                 * The interrupt was caused by a character being received.
                 * Grab the character from the rx and place it in the queue
                 * or received characters.
                 */
                cChar = USART_ReceiveData(usart);
                if (rx)
                        xQueueSendFromISR(rx, &cChar, &xTaskWokenByPost);
        }

        handle_usart_overrun(usart);

        /*
         * If a task was woken by either a character being received or a
         * character being transmitted then we may need to switch to
         * another task.
         */
        portEND_SWITCHING_ISR(xTaskWokenByPost || xTaskWokenByTx);
}


void USART1_IRQHandler(void)
{
        struct usart_info *ui = usart_data + UART_WIRELESS;
        usart_generic_irq_handler(USART1, ui->rx, ui->tx);
}

void USART2_IRQHandler(void)
{
        struct usart_info *ui = usart_data + UART_GPS;
        /*
         * rx is NULL here b/c its incomming data is handled by DMA.
         * Setting NULL just prevents us from inserting a character into
         * the serial buffer.  It will still receive and clear the flag
         * if it gets called, but it shouldn't be called since we do not
         * enable it.
         */
        usart_generic_irq_handler(USART2, NULL, ui->tx);
}

void USART3_IRQHandler(void)
{
        struct usart_info *ui = usart_data + UART_AUX;
        usart_generic_irq_handler(USART3, ui->rx, ui->tx);
}

void UART4_IRQHandler(void)
{
        struct usart_info *ui = usart_data + UART_TELEMETRY;
        usart_generic_irq_handler(UART4, ui->rx, ui->tx);
}
