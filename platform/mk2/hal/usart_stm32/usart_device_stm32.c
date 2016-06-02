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
#include "stm32f4xx_tim.h"
#include "stm32f4xx_usart.h"
#include "task.h"
#include "timer.h"
#include "usart_device.h"

/*
 * DMA Channels (Rx Ch/Stream || Tx Ch/Stream)
 *
 * USART1 - DMA2 Ch 4 / Stream 2/5 || DMA2 Ch 4 / Stream 7
 * USART2 - DMA1 Ch 4 / Stream 5   || DMA1 Ch 4 / Stream 6
 * USART3 - DMA1 Ch 4 / Stream 1   || DMA1 Ch 4 / Stream 3
 * UART4  - DMA1 Ch 4 / Stream 2   || DMA1 Ch 4 / Stream 4
 *
 * NOTE: DMA2_Stream2 is in use by ADC2.  So must use DMA2_S5
 */

#define DEFAULT_AUX_BAUD_RATE		115200
#define DEFAULT_GPS_BAUD_RATE		921600
#define DEFAULT_TELEMETRY_BAUD_RATE	115200
#define DEFAULT_WIRELESS_BAUD_RATE	115200
#define DMA_RX_BUFF_SIZE		32
#define DMA_TX_BUFF_SIZE		32
#define DMA_IRQ_PRIORITY		5
#define LOG_PFX				"[USART] "
#define UART_AUX_IRQ_PRIORITY 		8
#define UART_GPS_IRQ_PRIORITY 		5
#define UART_QUEUE_LEN 			1024
#define UART_TELEMETRY_IRQ_PRIORITY 	6
#define UART_WIRELESS_IRQ_PRIORITY 	7

typedef enum {
    UART_RX_IRQ = 1,
    UART_TX_IRQ = 2
} uart_irq_type_t;

struct dma_info {
        DMA_Stream_TypeDef* stream;     /* Pointer to our DMA Stream */
        uint32_t channel;               /* Channel number for our stream */
        size_t buff_size;               /* Size of the buffer */
        volatile uint8_t* buff;         /* Base address of buffer */
        volatile uint8_t* volatile ptr; /* Tail/Head pointer of the buffer */
};

static volatile struct usart_info {
        struct Serial *serial;
        USART_TypeDef *usart;
        volatile struct dma_info dma_rx;
        volatile struct dma_info dma_tx;
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

static void enable_dma_rx(const uint32_t dma_periph,
                          const uint8_t irq_channel,
                          const uint8_t irq_priority,
                          const uint32_t dma_it_flags,
                          volatile struct usart_info* ui)
{
        const volatile struct dma_info* dma = &ui->dma_rx;
        RCC_AHB1PeriphClockCmd(dma_periph, ENABLE);

        DMA_DeInit(dma->stream);

        DMA_InitTypeDef DMA_InitStructure;
        DMA_StructInit(&DMA_InitStructure);

        DMA_InitStructure.DMA_Channel = dma->channel;
        DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t) dma->buff;
        DMA_InitStructure.DMA_BufferSize = dma->buff_size;
        DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) &ui->usart->DR;
        DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
        DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
        DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
        DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
        DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
        DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
        DMA_InitStructure.DMA_Priority = DMA_Priority_High;

        DMA_Init(dma->stream, &DMA_InitStructure);

        if (dma_it_flags) {
                NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

                NVIC_InitTypeDef NVIC_InitStructure;
                NVIC_InitStructure.NVIC_IRQChannel = irq_channel;
                NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = irq_priority;
                NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
                NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
                NVIC_Init(&NVIC_InitStructure);

                DMA_ITConfig(dma->stream, dma_it_flags, ENABLE);
        }

    /* Enable the USART Rx DMA request */
    USART_DMACmd(ui->usart, USART_DMAReq_Rx, ENABLE);

    /* Enable the DMA RX Stream */
    DMA_Cmd(dma->stream, ENABLE);
}

#if 0
static void enable_dma_tx(uint32_t RCC_AHBPeriph,
                          const uint8_t irq_channel,
                          const uint8_t irq_priority,
                          const uint32_t dma_it_flags,
                          volatile struct usart_info* ui)
{
        const volatile struct dma_info* dma = &ui->dma_tx;
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph, ENABLE);
        DMA_DeInit(dma->chan);

        DMA_InitTypeDef DMA_InitStructure;
        DMA_InitStructure.DMA_Channel = dma->chan;
        DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t) dma->buff;
        DMA_InitStructure.DMA_BufferSize = 0;
        DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) &ui->usart->DR;
        DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
        DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
        DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
        DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
        DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
        DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
        DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
        DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable;
        DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
        DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
        DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;

        DMA_Init(DMA_stream, &DMA_InitStructure);

        if (dma_it_flags) {
                NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

                NVIC_InitTypeDef NVIC_InitStructure;
                NVIC_InitStructure.NVIC_IRQChannel = irq_channel;
                NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = irq_priority;
                NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
                NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
                NVIC_Init(&NVIC_InitStructure);

                DMA_ITConfig(DMA_stream, dma_it_flags, ENABLE);
        }

    /* Enable the USART Rx DMA request */
    USART_DMACmd(ui->usart, USART_DMAReq_Tx, ENABLE);
}
#endif

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

        if (irqType & UART_RX_IRQ)
                USART_ITConfig(USARTx, USART_IT_RXNE, ENABLE);

        if (irqType & UART_TX_IRQ)
                USART_ITConfig(USARTx, USART_IT_TXE, ENABLE);
}

/* Wireless port */
static void usart_device_init_0(size_t bits, size_t parity,
                                size_t stopBits, size_t baud)
{
        volatile struct usart_info* ui = usart_data + UART_WIRELESS;

        RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

        initGPIO(GPIOA, (GPIO_Pin_9 | GPIO_Pin_10));
        GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
        GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);

        enableRxTxIrq(ui->usart, USART1_IRQn,
                      UART_WIRELESS_IRQ_PRIORITY,
                      UART_TX_IRQ);

        enable_dma_rx(RCC_AHB1Periph_DMA2, DMA2_Stream5_IRQn,
                      DMA_IRQ_PRIORITY, DMA_IT_TC | DMA_IT_HT, ui);

        init_usart(ui->usart, bits, parity, stopBits, baud);
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
        volatile struct usart_info* ui = usart_data + UART_GPS;

        RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

        initGPIO(GPIOD, (GPIO_Pin_5 | GPIO_Pin_6));
        GPIO_PinAFConfig(GPIOD, GPIO_PinSource5, GPIO_AF_USART2);
        GPIO_PinAFConfig(GPIOD, GPIO_PinSource6, GPIO_AF_USART2);

        init_usart(ui->usart, bits, parity, stopBits, baud);
        enableRxTxIrq(ui->usart, USART2_IRQn, UART_GPS_IRQ_PRIORITY,
                      UART_TX_IRQ);

        enable_dma_rx(RCC_AHB1Periph_DMA1, DMA1_Stream5_IRQn,
                      DMA_IRQ_PRIORITY, DMA_IT_TC | DMA_IT_HT, ui);
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
        volatile struct usart_info *ui = post_tx_arg;
        if (!ui->dma_tx.stream)
                USART_ITConfig(ui->usart, USART_IT_TXE, ENABLE);
}

static bool init_usart_serial(const uart_id_t uart_id, USART_TypeDef *usart,
                              const serial_id_t serial_id, const char *name,
                              const size_t dma_rx_buff_size,
                              DMA_Stream_TypeDef* dma_rx_stream,
                              const uint32_t dma_rx_channel,
                              const size_t dma_tx_buff_size,
                              DMA_Stream_TypeDef* dma_tx_stream,
                              const uint32_t dma_tx_channel)
{
        volatile struct usart_info *ui = usart_data + uart_id;
        struct Serial *s =
                serial_create(name, UART_QUEUE_LEN, UART_QUEUE_LEN,
                              _config_cb, usart, _char_tx_cb,
                              (void*) ui);
        if (!s) {
                pr_error(LOG_PFX "Serial Malloc failure!\r\n");
                return false;
        }

        ui->usart = usart;
        ui->serial = s;

        uint8_t* dma_rx_buff = NULL;
        if (dma_rx_stream && dma_rx_buff_size) {
                dma_rx_buff = malloc(dma_rx_buff_size);
                if (!dma_rx_buff) {
                        pr_error(LOG_PFX "DMA Rx Buff Malloc failure!\r\n");
                        return false;
                }

                ui->dma_rx.buff_size = dma_rx_buff_size;
                ui->dma_rx.buff = dma_rx_buff;
                ui->dma_rx.ptr = dma_rx_buff;
                ui->dma_rx.channel = dma_rx_channel;
                ui->dma_rx.stream = dma_rx_stream;
        }

        uint8_t* dma_tx_buff = NULL;
        if (dma_tx_stream && dma_tx_buff_size) {
                dma_tx_buff = malloc(dma_tx_buff_size);
                if (!dma_tx_buff) {
                        pr_error(LOG_PFX "DMA Tx Buff Malloc failure!\r\n");
                        return false;
                }

                ui->dma_tx.buff_size = dma_tx_buff_size;
                ui->dma_tx.buff = dma_tx_buff;
                ui->dma_tx.ptr = dma_tx_buff;
                ui->dma_tx.channel = dma_tx_channel;
                ui->dma_tx.stream = dma_tx_stream;
        }

        return true;
}

static bool usart_id_in_bounds(const uart_id_t id)
{
        return ((size_t) id) < __UART_COUNT;
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
static void usart_generic_irq_handler(volatile struct usart_info *ui)
{
        USART_TypeDef *usart = ui->usart;
        signed portCHAR cChar;
        portBASE_TYPE xTaskWoken = pdFALSE;

        if (SET == USART_GetITStatus(usart, USART_IT_TXE)) {
                /*
                 * The interrupt was caused by the TX becoming empty.
                 * Are there any more characters to transmit?
                 */
                xQueueHandle tx_queue = serial_get_tx_queue(ui->serial);
                if (pdTRUE == xQueueReceiveFromISR(tx_queue, &cChar,
                                                   &xTaskWoken)) {
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

        /*
         * Read the ORE status flag here because doing this along with
         * reading data from our UART will clear the ORE flag if it has
         * been set.  Thus we get a two for one and reduce the chance of
         * accidentially dumping a character like we could do with our
         * previous overrun handler. This is per the documentation in the
         * USART_ClearFlag method.
         */
        const bool ore_set =
                SET == USART_GetFlagStatus(usart, USART_FLAG_ORE);


        if (USART_GetITStatus(usart, USART_IT_RXNE) == SET) {
                /*
                 * The interrupt was caused by a character being received.
                 * Grab the character from the rx and place it in the queue
                 * or received characters.
                 */
                cChar = USART_ReceiveData(usart);
                xQueueHandle rx_queue = serial_get_rx_queue(ui->serial);
                xQueueSendFromISR(rx_queue, &cChar, &xTaskWoken);
        } else if (ore_set) {
                /*
                 * We will likely never get in here, but this is to
                 * be sure we cover all our bases.  See page 968 of the
                 * reference manual for why I did this.  In short its safe to dump
                 * the data here since we know there is nothing in the RDR
                 * register (since RXNE is not set).  Having the ORE set here
                 * indicates a race case where we read the data as the other
                 * data was lost in the shift register (thus leaving the ORE
                 * flag set but clearing the RXNE flag). In other words, this
                 * is done purely to clear the ORE flag knowing we are not losing
                 * data.
                 */
                USART_ReceiveData(usart);
        }

        /*
         * If a task was woken by either a character being received or a
         * character being transmitted then we may need to switch to
         * another task.
         */
        portEND_SWITCHING_ISR(xTaskWoken);
}

void USART1_IRQHandler(void)
{
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

void UART4_IRQHandler(void)
{
        usart_generic_irq_handler(usart_data + UART_TELEMETRY);
}

static bool dma_rx_isr(volatile struct usart_info *ui)
{
        volatile uint8_t* tail = ui->dma_rx.ptr;
        volatile uint8_t* const buff = ui->dma_rx.buff;
        volatile uint8_t* const edge = buff + ui->dma_rx.buff_size;
        volatile uint8_t* const head = edge - ui->dma_rx.stream->NDTR;
        xQueueHandle queue = serial_get_rx_queue(ui->serial);
        portBASE_TYPE task_awoke = pdFALSE;

        while (tail != head) {
                uint8_t val = *tail;
                xQueueSendFromISR(queue, &val, &task_awoke);

                if (++tail >= edge)
                        tail = buff;
        }

        ui->dma_rx.ptr = head;
        return task_awoke;
}


void DMA1_Stream5_IRQHandler(void)
{
        DMA_ClearITPendingBit(DMA1_Stream5, DMA_IT_HTIF5);
        DMA_ClearITPendingBit(DMA1_Stream5, DMA_IT_TCIF5);
        volatile struct usart_info *ui = usart_data + UART_GPS;
        portEND_SWITCHING_ISR(dma_rx_isr(ui));
}

void DMA2_Stream5_IRQHandler(void)
{
        DMA_ClearITPendingBit(DMA2_Stream5, DMA_IT_HTIF5);
        DMA_ClearITPendingBit(DMA2_Stream5, DMA_IT_TCIF5);
        volatile struct usart_info *ui = usart_data + UART_WIRELESS;
        portEND_SWITCHING_ISR(dma_rx_isr(ui));
}

/* *** Timer Methods *** */
/* Timer 7 IRQ Handler */
void TIM7_IRQHandler(void)
{
        TIM_ClearITPendingBit(TIM7, TIM_IT_Update);

        volatile struct usart_info *ui_wireless =
                usart_data + UART_WIRELESS;
        volatile struct usart_info *ui_gps = usart_data + UART_GPS;

        const bool gps_ta = dma_rx_isr(ui_gps);
        const bool wireless_ta = dma_rx_isr(ui_wireless);

        /* const bool ta_gps_tx = dma_tx_isr(ui_gps, false); */
        /* const bool ta_wifi_tx = dma_tx_isr(ui_wifi, false); */

        portEND_SWITCHING_ISR(gps_ta || wireless_ta);
}


void setup_dma_timer(void)
{
        /* We use Timer 7 */
        TIM_TypeDef* const timer = TIM7;

        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);
        TIM_DeInit(timer);

        TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
        TIM_TimeBaseStructInit(&TIM_TimeBaseInitStructure);
        /* Proc is 168MHz, so prescalar is 168 - 1 to make clock 1MHz */
        TIM_TimeBaseInitStructure.TIM_Prescaler = 167;
        /* With above clock, each tick is 1 us. We want interrupts every 1ms */
        TIM_TimeBaseInitStructure.TIM_Period = 1000;
        TIM_TimeBaseInit(timer, &TIM_TimeBaseInitStructure);

        TIM_UpdateDisableConfig(timer, DISABLE);
        TIM_UpdateRequestConfig(timer, TIM_UpdateSource_Global);

        NVIC_InitTypeDef NVIC_InitStructure;
        NVIC_InitStructure.NVIC_IRQChannel = TIM7_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = DMA_IRQ_PRIORITY;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStructure);
        TIM_ITConfig(timer, TIM_IT_Update, ENABLE);

        TIM_Cmd(timer, ENABLE);
}

/* *** Public Methods *** */
int usart_device_init()
{
        /*
         * NOTE: Careful with the mappings here.  If you change them
         *       ensure that you also update the values in the IRQ
         *       handlers below.
         */
        const bool mem_alloc_success =
                init_usart_serial(UART_GPS, USART2, SERIAL_GPS, "GPS",
                                  DMA_RX_BUFF_SIZE, DMA1_Stream5,
                                  DMA_Channel_4, 0, NULL, 0) &&
                init_usart_serial(UART_TELEMETRY, UART4, SERIAL_TELEMETRY,
                                  "Cell", 0, NULL, 0, 0, NULL, 0) &&
                init_usart_serial(UART_WIRELESS, USART1, SERIAL_BLUETOOTH,
                                  "BT", DMA_RX_BUFF_SIZE, DMA2_Stream5,
                                  DMA_Channel_4, 0, NULL, 0) &&
                init_usart_serial(UART_AUX, USART3, SERIAL_AUX, "Aux",
                                  0, NULL, 0, 0, NULL, 0);

        if (!mem_alloc_success) {
                pr_error(LOG_PFX "Failed to init\r\n");
                panic(PANIC_CAUSE_MALLOC);
        }

        usart_device_init_0(8, 0, 1, DEFAULT_WIRELESS_BAUD_RATE);
        usart_device_init_1(8, 0, 1, DEFAULT_AUX_BAUD_RATE);
        usart_device_init_2(8, 0, 1, DEFAULT_GPS_BAUD_RATE);
        usart_device_init_3(8, 0, 1, DEFAULT_TELEMETRY_BAUD_RATE);

        setup_dma_timer();

        return 1;
}

struct Serial* usart_device_get_serial(const uart_id_t id)
{
        if (!usart_id_in_bounds(id))
                return NULL;

        volatile struct usart_info *ui = usart_data + id;
        return ui->serial;
}
