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
#include "timers.h"
#include "usart_device.h"

#define UART_RX_QUEUE_LEN 	256
#define UART_TX_QUEUE_LEN 	64
#define GPS_BUFFER_SIZE		132
#define LOG_PFX		"[USART] "
#define UART_WIRELESS_IRQ_PRIORITY 	7
#define UART_GPS_IRQ_PRIORITY 		5
#define UART_TELEMETRY_IRQ_PRIORITY 	6

#define DEFAULT_WIRELESS_BAUD_RATE	9600
#define DEFAULT_TELEMETRY_BAUD_RATE	115200
#define DEFAULT_GPS_BAUD_RATE		9600
#define WIFI_DMA_RX_BUFF_SIZE	32

typedef enum {
    UART_RX_IRQ = 1,
    UART_TX_IRQ = 2
} uart_irq_type_t;

struct dma_info {
        size_t buff_size;               /* Size of the buffer */
        volatile uint8_t* buff;         /* Base address of buffer */
        volatile uint8_t* volatile ptr; /* Tail/Head pointer of the buffer */
        DMA_Channel_TypeDef* chan;      /* Pointer to our DMA channel */
};

static volatile struct usart_info {
        struct Serial *serial;
        USART_TypeDef *usart;
        struct dma_info dma_rx;
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
                        const uint8_t irq_channel,
                        const uint8_t irq_priority,
                        const uint32_t dma_it_flags,
                        volatile struct usart_info* ui)
{
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph, ENABLE);
        DMA_DeInit(ui->dma_rx.chan);

        /*  Initialize USART2 RX DMA Channel */
        DMA_InitTypeDef DMA_InitStruct;
        DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t) &ui->usart->RDR;
        DMA_InitStruct.DMA_MemoryBaseAddr = (uint32_t) ui->dma_rx.buff;
        DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralSRC;
        DMA_InitStruct.DMA_BufferSize = ui->dma_rx.buff_size;
        DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
        DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
        DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
        DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
        DMA_InitStruct.DMA_Mode = DMA_Mode_Circular;
        DMA_InitStruct.DMA_Priority = DMA_Priority_High;
        DMA_InitStruct.DMA_M2M = DMA_M2M_Disable;

        /*  Configure RX DMA Channel Interrupts. */
        if (dma_it_flags) {
                NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

                NVIC_InitTypeDef NVIC_InitStruct;
                NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
                NVIC_InitStruct.NVIC_IRQChannel = irq_channel;
                NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = irq_priority;
                NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
                NVIC_Init(&NVIC_InitStruct);

                DMA_ITConfig(ui->dma_rx.chan, dma_it_flags, ENABLE);
        }

        /* Initialize RX DMA Channel. */
        USART_DMACmd(ui->usart, USART_DMAReq_Rx, ENABLE);
        DMA_Init(ui->dma_rx.chan, &DMA_InitStruct);

        // Enable USART2 RX DMA Channel.
        DMA_Cmd(ui->dma_rx.chan, ENABLE);
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

        if (irqType & UART_RX_IRQ)
                USART_ITConfig(USARTx, USART_IT_RXNE, ENABLE);

        if (irqType & UART_TX_IRQ)
                USART_ITConfig(USARTx, USART_IT_TXE, ENABLE);
}

/* Wireless port */
static void usart_device_init_1(size_t bits, size_t parity,
                                size_t stopBits, size_t baud)
{
        volatile struct usart_info* ui = usart_data + UART_WIRELESS;

        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
        initGPIO(GPIOA, (GPIO_Pin_9 | GPIO_Pin_10));
        GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_7);
        GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_7);

        RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
        init_usart(ui->usart, bits, parity, stopBits, baud);

        /* Note, only transmit interrupt is enabled */
        enableRxTxIrq(ui->usart, USART1_IRQn,
                      UART_WIRELESS_IRQ_PRIORITY, UART_TX_IRQ);

        enableRxDMA(RCC_AHBPeriph_DMA1, DMA1_Channel5_IRQn,
                    UART_WIRELESS_IRQ_PRIORITY,
                    DMA_IT_TC | DMA_IT_HT, ui);
}

/* GPS port */
void usart_device_init_2(size_t bits, size_t parity,
                         size_t stopBits, size_t baud)
{
        volatile struct usart_info* ui = usart_data + UART_GPS;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
        initGPIO(GPIOB, (GPIO_Pin_4 | GPIO_Pin_3));
        GPIO_PinAFConfig(GPIOB, GPIO_PinSource4, GPIO_AF_7);
        GPIO_PinAFConfig(GPIOB, GPIO_PinSource3, GPIO_AF_7);

        RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
        init_usart(ui->usart, bits, parity, stopBits, baud);

        /* Note, only transmit interrupt is enabled */
        enableRxTxIrq(ui->usart, USART2_IRQn,
                      UART_GPS_IRQ_PRIORITY, UART_TX_IRQ);

        enableRxDMA(RCC_AHBPeriph_DMA1, DMA1_Channel6_IRQn,
                    UART_GPS_IRQ_PRIORITY,
                    DMA_IT_TC | DMA_IT_HT, ui);
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
                              const serial_id_t serial_id, const char *name,
                              const size_t rx_buff_size,
                              DMA_Channel_TypeDef* dma_chan)
{
        struct Serial *s =
                serial_create(name, UART_TX_QUEUE_LEN, UART_RX_QUEUE_LEN,
                              _config_cb, usart, _char_tx_cb, usart);
        if (!s) {
                pr_error(LOG_PFX "Serial Malloc failure!\r\n");
                return false;
        }

        uint8_t* rx_buff = NULL;
        if (rx_buff_size) {
                rx_buff = malloc(rx_buff_size);
                if (!rx_buff) {
                        pr_error(LOG_PFX "Rx DMA Buff Malloc failure!\r\n");
                        return false;
                }
        }

        /* Set the usart_info data */
        volatile struct usart_info *ui = usart_data + uart_id;
        ui->usart = usart;
        ui->serial = s;
        ui->dma_rx.buff_size = rx_buff_size;
        ui->dma_rx.buff = rx_buff;
        ui->dma_rx.ptr = rx_buff;
        ui->dma_rx.chan = dma_chan;

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

void DMA1_Channel6_IRQHandler(void)
{
        volatile struct usart_info *ui = usart_data + UART_GPS;
        const size_t buff_size = ui->dma_rx.buff_size;
        volatile uint8_t* buff = ui->dma_rx.buff;
        xQueueHandle rx_queue = serial_get_rx_queue(ui->serial);
        /* We only ever expect one interrupt or the other, not both */
        portBASE_TYPE task_awoken = pdFALSE;

        /* Test on DMA Stream Transfer Complete interrupt */
        if (DMA_GetITStatus(DMA1_IT_TC6)) {
                /* Clear DMA Stream Transfer Complete interrupt pending bit */
                DMA_ClearITPendingBit(DMA1_IT_TC6);
                for (size_t i = buff_size / 2; i < buff_size; i++) {
                        const uint8_t val = buff[i];
                        xQueueSendFromISR(rx_queue, &val, &task_awoken);
                }
        }

        /* Test on DMA Stream Half Transfer interrupt */
        if (DMA_GetITStatus(DMA1_IT_HT6)) {
                /* Clear DMA Stream Half Transfer interrupt pending bit */
                DMA_ClearITPendingBit(DMA1_IT_HT6);
                for (size_t i = 0; i < buff_size / 2; i++) {
                        const uint8_t val = buff[i];
                        xQueueSendFromISR(rx_queue, &val, &task_awoken);
                }
        }

        portEND_SWITCHING_ISR(task_awoken);
}

static void usart_generic_irq_handler(volatile struct usart_info *ui)
{
        USART_TypeDef *usart = ui->usart;
        signed portCHAR cChar;
        portBASE_TYPE xTaskWokenByTx = pdFALSE;

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
        }

        /*
         * ORE interrupt can occur when USART_IT_RXNE interrupt is active
         * OR when USART_IT_ERR interrupt is active. See page 933 of the
         * STm32F3XX manual for a logic diagram.  If ORE triggers it won't
         * necessarily set the USART_IT_ORE interrupt flag because we
         * disable the USART_IT_ERR interrupt.  This breaks the check
         * provided by USART_GetITStatus(usart, USART_IT_ORE).  This bug has
         * been fixed in the STM32F4XX libs, but not this series :(.  Thus
         * we must check the ISR directly to see if this is what caused the
         * interrupt and clear it appropriately.
         */
        if (SET == USART_GetFlagStatus(usart, USART_FLAG_ORE))
                USART_ClearFlag(usart, USART_FLAG_ORE);

        /*
         * If a task was woken by either a character being received or a
         * character being transmitted then we may need to switch to
         * another task.
         */
        portEND_SWITCHING_ISR(xTaskWokenByPost || xTaskWokenByTx);
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

static void dma_data_to_queue(xQueueHandle queue,
                              const volatile uint8_t* tail,
                              const volatile uint8_t* head,
                              const bool in_isr,
                              portBASE_TYPE* task_awoke)
{
        for (uint8_t val; tail < head; ++tail) {
                val = *tail;
                if (in_isr) {
                        xQueueSendFromISR(queue, &val, task_awoke);
                } else {
                        xQueueSend(queue, &val, 0);
                }
        }
}


static bool dma_rx_to_queue_isr(volatile struct usart_info *ui)
{
        volatile uint8_t* buff = ui->dma_rx.buff;
        volatile uint8_t* const edge = buff + ui->dma_rx.buff_size;
        volatile uint8_t* volatile head = edge - ui->dma_rx.chan->CNDTR;
        volatile uint8_t* volatile tail = ui->dma_rx.ptr;
        xQueueHandle queue = serial_get_rx_queue(ui->serial);
        portBASE_TYPE task_awoke = pdFALSE;

        if (head < tail) {
                dma_data_to_queue(queue, tail, edge, true, &task_awoke);
                tail = buff;
        }

        if (tail < head) {
                dma_data_to_queue(queue, tail, head, true, &task_awoke);
                tail = head;
        }

        ui->dma_rx.ptr = tail;
        return task_awoke;
}

void DMA1_Channel5_IRQHandler(void)
{
        volatile struct usart_info *ui = usart_data + UART_WIRELESS;
        const bool task_awoken = dma_rx_to_queue_isr(ui);
        DMA_ClearITPendingBit(DMA1_IT_TC5);
        DMA_ClearITPendingBit(DMA1_IT_HT5);
        portEND_SWITCHING_ISR(task_awoken);
}

static void dma_rx_to_queue(volatile struct usart_info *ui)
{
        /* Disable DMA interrupts during this to prevent contention */
        static const uint32_t dma_it_mask =
                DMA_IT_TC | DMA_IT_HT | DMA_IT_TE;
        const volatile uint32_t dma_it_flags =
                ui->dma_rx.chan->CCR & dma_it_mask;
        if (dma_it_flags)
                DMA_ITConfig(ui->dma_rx.chan, dma_it_flags, DISABLE);

        volatile uint8_t* buff = ui->dma_rx.buff;
        volatile uint8_t* const edge = buff + ui->dma_rx.buff_size;
        volatile uint8_t* volatile head = edge - ui->dma_rx.chan->CNDTR;
        volatile uint8_t* volatile tail = ui->dma_rx.ptr;
        xQueueHandle queue = serial_get_rx_queue(ui->serial);

        if (head < tail) {
                dma_data_to_queue(queue, tail, edge, false, NULL);
                tail = buff;
        }

        if (tail < head) {
                dma_data_to_queue(queue, tail, head, false, NULL);
                tail = head;
        }

        ui->dma_rx.ptr = tail;

        /* Re-enable DMA interrupts if they were set originally. */
        if (dma_it_flags)
                DMA_ITConfig(ui->dma_rx.chan, dma_it_flags, ENABLE);
}

static void timer_dma_handler(xTimerHandle handle)
{
        dma_rx_to_queue(usart_data + UART_WIRELESS);
}


/* *** Public methods *** */

int usart_device_init()
{
        /*
         * NOTE: Careful with the mappings here.  If you change them
         *       ensure that you also update the values in the IRQ
         *       handlers.
         */
        const bool mem_alloc_success =
                init_usart_serial(UART_WIRELESS, USART1, SERIAL_WIFI, "WiFi",
                                  WIFI_DMA_RX_BUFF_SIZE, DMA1_Channel5) &&
                init_usart_serial(UART_GPS, USART2, SERIAL_GPS, "GPS",
                                  GPS_BUFFER_SIZE, DMA1_Channel6);

        if (!mem_alloc_success) {
                pr_error("[USART] Failed to init\r\n");
                panic(PANIC_CAUSE_MALLOC);
        }

        usart_device_init_1(8, 0, 1, DEFAULT_WIRELESS_BAUD_RATE);
        usart_device_init_2(8, 0, 1, DEFAULT_GPS_BAUD_RATE);
        /* usart_device_init_3(8, 0, 1, DEFAULT_TELEMETRY_BAUD_RATE); */

        /* Create a timer that fires every tick to handle DMA data */
        const signed char timer_name[] = "USART DMA Timer";
        const xTimerHandle dma_timer =
                xTimerCreate(timer_name, 1, pdTRUE, NULL, timer_dma_handler);
        xTimerStart(dma_timer, 0);

        return 1;
}

struct Serial* usart_device_get_serial(const uart_id_t id)
{
        if (!usart_id_in_bounds(id))
                return NULL;

        volatile struct usart_info *ui = usart_data + id;
        return ui->serial;
}
