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
#include "stm32f30x_dma.h"
#include "stm32f30x_gpio.h"
#include "stm32f30x_misc.h"
#include "stm32f30x_rcc.h"
#include "stm32f30x_usart.h"
#include "task.h"
#include "usart_device.h"

#define UART_RX_QUEUE_LEN 	256
#define UART_TX_QUEUE_LEN 	64
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

static uint8_t *gpsRxBuffer;

static volatile struct usart_info {
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
     NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = UART_GPS_IRQ_PRIORITY; //1; //IRQ_priority;
     NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;

     NVIC_Init(&NVIC_InitStruct);

}

static void enableRxTxIrq(USART_TypeDef * USARTx, uint8_t usartIrq,
                          uint8_t IRQ_priority, uart_irq_type_t irqType)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    /* Test to clear the Error Flags */
    USART_ITConfig(USARTx, USART_IT_WU, DISABLE);
    USART_ITConfig(USARTx, USART_IT_CM, DISABLE);
    USART_ITConfig(USARTx, USART_IT_EOB, DISABLE);
    USART_ITConfig(USARTx, USART_IT_RTO, DISABLE);
    USART_ITConfig(USARTx, USART_IT_CTS, DISABLE);
    USART_ITConfig(USARTx, USART_IT_LBD, DISABLE);
    USART_ITConfig(USARTx, USART_IT_TC, DISABLE);
    USART_ITConfig(USARTx, USART_IT_IDLE, DISABLE);
    USART_ITConfig(USARTx, USART_IT_PE, DISABLE);
    USART_ITConfig(USARTx, USART_IT_ERR, DISABLE);

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
static void usart_device_init_1(size_t bits, size_t parity,
                                size_t stopBits, size_t baud)
{

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

    initGPIO(GPIOA, (GPIO_Pin_9 | GPIO_Pin_10));
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_7);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_7);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    init_usart(USART1, bits, parity, stopBits, baud);

    enableRxTxIrq(USART1, USART1_IRQn, UART_WIRELESS_IRQ_PRIORITY,
                  (UART_RX_IRQ | UART_TX_IRQ));
}

/* GPS port */
void usart_device_init_2(size_t bits, size_t parity,
                         size_t stopBits, size_t baud)
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
    initGPIO(GPIOB, (GPIO_Pin_4 | GPIO_Pin_3));
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource4, GPIO_AF_7);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource3, GPIO_AF_7);

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    init_usart(USART2, bits, parity, stopBits, baud);

    /* Note, only transmit interrupt is enabled */
    enableRxTxIrq(USART2, USART2_IRQn, UART_GPS_IRQ_PRIORITY, (UART_TX_IRQ));

    enableRxDMA(RCC_AHBPeriph_DMA1, DMA1_Channel6,
                gpsRxBuffer, GPS_BUFFER_SIZE, USART2, DMA1_Channel6_IRQn,
                UART_GPS_IRQ_PRIORITY);
}

/* Telemetry port - Commented out for now b/c need RAM */
#if 0
void usart_device_init_3(size_t bits, size_t parity,
                         size_t stopBits, size_t baud)
{
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
    initGPIO(GPIOB, (GPIO_Pin_10 | GPIO_Pin_11));
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_7);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_7);

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
    init_usart(UART4, bits, parity, stopBits, baud);

    enableRxTxIrq(USART3, UART4_IRQn, UART_TELEMETRY_IRQ_PRIORITY,
                  (UART_RX_IRQ | UART_TX_IRQ));

}
#endif

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
                serial_create(name, UART_TX_QUEUE_LEN, UART_RX_QUEUE_LEN,
                              _config_cb, usart, _char_tx_cb, usart);
        if (!s) {
                pr_error("[USART] Serial Malloc failure!\r\n");
                return false;
        }

        /* Set the usart_info data */
        volatile struct usart_info *ui = usart_data + uart_id;
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
                init_usart_serial(UART_WIRELESS, USART1,
                                  SERIAL_WIRELESS, "WiFi");


        if (!mem_alloc_success) {
                pr_error("[USART] Failed to init\r\n");
                panic(PANIC_CAUSE_MALLOC);
        }

        usart_device_init_1(8, 0, 1, DEFAULT_WIRELESS_BAUD_RATE);
        usart_device_init_2(8, 0, 1, DEFAULT_GPS_BAUD_RATE);
        /* usart_device_init_3(8, 0, 1, DEFAULT_TELEMETRY_BAUD_RATE); */

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

        volatile struct usart_info *ui = usart_data + id;
        return ui->serial;
}

void usart_device_config(const uart_id_t id, const size_t bits,
                         const size_t parity, const size_t stop_bits,
                         const size_t baud)
{
        if (!usart_id_in_bounds(id))
                return;

        volatile struct usart_info *ui = usart_data + id;
        init_usart(ui->usart, bits, parity, stop_bits, baud);
}

/* *** Interrupt Handlers *** */

void DMA1_Channel6_IRQHandler(void)
{
        volatile struct usart_info *ui = usart_data + UART_GPS;
        xQueueHandle rx_queue = serial_get_rx_queue(ui->serial);

        portBASE_TYPE xTaskWokenByPost = pdFALSE;
        signed portCHAR cChar;

        /* Test on DMA Stream Transfer Complete interrupt */
        if (DMA_GetITStatus(DMA1_IT_TC6)) {
                /* Clear DMA Stream Transfer Complete interrupt pending bit */
                DMA_ClearITPendingBit(DMA1_IT_TC6);
                for (size_t i = GPS_BUFFER_SIZE / 2; i < GPS_BUFFER_SIZE; i++) {
                        cChar = gpsRxBuffer[i];
                        xQueueSendFromISR(rx_queue, &cChar, &xTaskWokenByPost);
                }
        }

        /* Test on DMA Stream Half Transfer interrupt */
        if (DMA_GetITStatus(DMA1_IT_HT6)) {
                /* Clear DMA Stream Half Transfer interrupt pending bit */
                DMA_ClearITPendingBit(DMA1_IT_HT6);
                for (size_t i = 0; i < GPS_BUFFER_SIZE / 2; i++) {
                        cChar = gpsRxBuffer[i];
                        xQueueSendFromISR(rx_queue, &cChar, &xTaskWokenByPost);
                }
        }

        portEND_SWITCHING_ISR(xTaskWokenByPost);
}

static void usart_generic_irq_handler(volatile struct usart_info *ui)
{
        USART_TypeDef *usart = ui->usart;
        signed portCHAR cChar;
        portBASE_TYPE xTaskWokenByTx = pdFALSE;
        bool done_work = false;

        if (SET == USART_GetITStatus(usart, USART_IT_TXE)) {
                /*
                 * The interrupt was caused by the TX becoming empty.
                 * Are there any more characters to transmit?
                 */
                xQueueHandle tx_queue = serial_get_tx_queue(ui->serial);
                if (pdTRUE == xQueueReceiveFromISR(tx_queue, &cChar,
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

                done_work = true;
        }

        portBASE_TYPE xTaskWokenByPost = pdFALSE;
        if (SET == USART_GetITStatus(usart, USART_IT_RXNE)) {
                /*
                 * The interrupt was caused by a character being received.
                 * Grab the character from the rx and place it in the queue
                 * or received characters.
                 */
                cChar = USART_ReceiveData(usart);
                xQueueHandle rx_queue = serial_get_rx_queue(ui->serial);
                xQueueSendFromISR(rx_queue, &cChar, &xTaskWokenByPost);
                done_work = true;
        }

        if (!done_work) {
                /* Testing hack to try to eliminate interrupts */
                USART_ClearITPendingBit(usart, USART_IT_WU);
                USART_ClearITPendingBit(usart, USART_IT_CM);
                USART_ClearITPendingBit(usart, USART_IT_EOB);
                USART_ClearITPendingBit(usart, USART_IT_RTO);
                USART_ClearITPendingBit(usart, USART_IT_CTS);
                USART_ClearITPendingBit(usart, USART_IT_LBD);
                USART_ClearITPendingBit(usart, USART_IT_TC);
                USART_ClearITPendingBit(usart, USART_IT_IDLE);
                USART_ClearITPendingBit(usart, USART_IT_ORE);
                USART_ClearITPendingBit(usart, USART_IT_NE);
                USART_ClearITPendingBit(usart, USART_IT_FE);
                USART_ClearITPendingBit(usart, USART_IT_PE);
        }

        /*
         * If a task was woken by either a character being received or a
         * character being transmitted then we may need to switch to
         * another task.
         */
        portEND_SWITCHING_ISR(xTaskWokenByPost || xTaskWokenByTx);
}


void USART1_IRQHandler(void)
{
        volatile struct usart_info *ui = usart_data + UART_WIRELESS;
        USART_TypeDef *usart = ui->usart;

        ITStatus wu_status = USART_GetITStatus(usart, USART_FLAG_WU);
        ITStatus cm_status = USART_GetITStatus(usart, USART_FLAG_CM);
        ITStatus eob_status = USART_GetITStatus(usart, USART_FLAG_EOB);
        ITStatus rto_status = USART_GetITStatus(usart, USART_FLAG_RTO);
        ITStatus cts_status = USART_GetITStatus(usart, USART_FLAG_CTS);
        ITStatus lbd_status = USART_GetITStatus(usart, USART_FLAG_LBD);
        ITStatus txe_status = USART_GetITStatus(usart, USART_FLAG_TXE);
        ITStatus tc_status = USART_GetITStatus(usart, USART_FLAG_TC);
        ITStatus rxne_status = USART_GetITStatus(usart, USART_FLAG_RXNE);
        ITStatus idle_status = USART_GetITStatus(usart, USART_FLAG_IDLE);
        ITStatus ore_status = USART_GetITStatus(usart, USART_FLAG_ORE);
        ITStatus ne_status = USART_GetITStatus(usart, USART_FLAG_NE);
        ITStatus fe_status = USART_GetITStatus(usart, USART_FLAG_FE);
        ITStatus pe_status = USART_GetITStatus(usart, USART_FLAG_PE);

        (void) wu_status;
        (void) cm_status;
        (void) eob_status;
        (void) rto_status;
        (void) cts_status;
        (void) lbd_status;
        (void) txe_status;
        (void) tc_status;
        (void) rxne_status;
        (void) idle_status;
        (void) ore_status;
        (void) ne_status;
        (void) fe_status;
        (void) pe_status;

        usart_generic_irq_handler(usart_data + UART_WIRELESS);
}

void USART2_IRQHandler(void)
{
        usart_generic_irq_handler(usart_data + UART_GPS);
}

void USART3_IRQHandler(void)
{
        usart_generic_irq_handler(usart_data + UART_AUX);
}
