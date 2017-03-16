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

#include "CAN_device.h"
#include "FreeRTOS.h"
#include "printk.h"
#include "queue.h"
#include "stm32f4xx_can.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_misc.h"
#include "stm32f4xx_rcc.h"
#include "task.h"
#include "taskUtil.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#define _LOG_PFX  "[CAN device] "

static xQueueHandle can_rx_queue = NULL;

#define CAN_FILTER_COUNT	13
#define CAN_IRQ_PRIORITY	5
#define CAN_IRQ_SUB_PRIORITY	0
#define CAN_QUEUE_LENGTH	10

//For 168MHz clock
/*       BS1 BS2 SJW Pre
 1M:      12  8   1   2
 500k:    8   5   1   6
 250k:    8   5   1   12
 125k:    12  8   1   16
 100k:    12  8   1   20 */

#define CAN_BAUD_COUNT 5
static const u8 can_baud_bs1[] =
{ CAN_BS1_12tq, CAN_BS1_12tq, CAN_BS1_8tq, CAN_BS1_8tq, CAN_BS1_12tq };

static const u8 can_baud_bs2[] =
{ CAN_BS1_8tq, CAN_BS1_8tq, CAN_BS1_5tq, CAN_BS1_5tq, CAN_BS1_8tq };

static const u8 can_baud_sjw[] =
{ CAN_SJW_1tq, CAN_SJW_1tq, CAN_SJW_1tq, CAN_SJW_1tq, CAN_SJW_1tq };

static const u8 can_baud_pre[] = { 20, 16, 12, 6, 2 };
static const u32 can_baud_rate[] = { 100000, 125000, 250000, 500000, 1000000 };

static bool init_queue()
{
        if (!can_rx_queue)
                can_rx_queue = xQueueCreate(CAN_QUEUE_LENGTH, sizeof(CAN_msg));
        return can_rx_queue != NULL;
}

static void init_GPIO_CAN(GPIO_TypeDef * GPIOx, uint32_t gpio_pins)
{
        /* Configure CAN RX and TX pins */
        GPIO_InitTypeDef GPIO_InitStructure;
        GPIO_InitStructure.GPIO_Pin = gpio_pins;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
        GPIO_Init(GPIOx, &GPIO_InitStructure);
}

static void init_GPIO_port(GPIO_TypeDef * GPIOx, uint32_t gpio_pins)
{
        GPIO_InitTypeDef GPIO_InitStructure;
        GPIO_InitStructure.GPIO_Pin = gpio_pins;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
        GPIO_Init(GPIOx, &GPIO_InitStructure);
}


static void init_CAN(CAN_TypeDef * CANx, uint32_t baud)
{
        CAN_InitTypeDef CAN_InitStructure;
        /* CAN cell init */
        CAN_InitStructure.CAN_TTCM = DISABLE;
        CAN_InitStructure.CAN_ABOM = ENABLE;
        CAN_InitStructure.CAN_AWUM = DISABLE;
        CAN_InitStructure.CAN_NART = DISABLE;
        CAN_InitStructure.CAN_RFLM = DISABLE;
        CAN_InitStructure.CAN_TXFP = DISABLE;
        CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;

        int baud_index = -1;

        /* Select baud rate up to requested rate, except for below min, where min is selected */
        if (baud >= can_baud_rate[CAN_BAUD_COUNT - 1]) {
            /* round down to peak rate if >= peak rate */
            baud_index = CAN_BAUD_COUNT - 1;
        } else {
            for (baud_index = 0; baud_index < CAN_BAUD_COUNT - 1; baud_index++) {
                if (baud < can_baud_rate[baud_index + 1]) {
                    /* take current idx if next is too large */
                    break;
                }
            }
        }
        CAN_InitStructure.CAN_SJW = can_baud_sjw[baud_index];
        CAN_InitStructure.CAN_BS1 = can_baud_bs1[baud_index];
        CAN_InitStructure.CAN_BS2 = can_baud_bs2[baud_index];
        CAN_InitStructure.CAN_Prescaler = can_baud_pre[baud_index];
        CAN_Init(CANx, &CAN_InitStructure);
}

static void init_CAN_interrupts(CAN_TypeDef * CANx, uint8_t irq_number)
{
        NVIC_InitTypeDef NVIC_InitStructure;
        NVIC_InitStructure.NVIC_IRQChannel = irq_number;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = CAN_IRQ_PRIORITY;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = CAN_IRQ_SUB_PRIORITY;
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStructure);
}

static void CAN_device_init_1(int baud, bool termination_enabled)
{
        CAN_DeInit(CAN1);

        /* CAN GPIOs configuration ************************************************* */
        /* Enable GPIO clock */
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

        /* Connect CAN pins to Alternate Function */
        GPIO_PinAFConfig(GPIOD, GPIO_PinSource0, GPIO_AF_CAN1);
        GPIO_PinAFConfig(GPIOD, GPIO_PinSource1, GPIO_AF_CAN1);
        init_GPIO_CAN(GPIOD, GPIO_Pin_0 | GPIO_Pin_1);

        /* CAN configuration ******************************************************* */
        /* Enable CAN clock */
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);

        init_CAN(CAN1, baud);

        /* Enable FIFO 0 message pending Interrupt */
        CAN_ITConfig(CAN1, CAN_IT_FMP0, ENABLE);

        init_CAN_interrupts(CAN1, CAN1_RX0_IRQn);

        /* Configure soft termination */
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
        init_GPIO_port(GPIOB, GPIO_Pin_10);
        if (termination_enabled) {
            GPIO_SetBits(GPIOB, GPIO_Pin_10);
        }
        else {
            GPIO_ResetBits(GPIOB, GPIO_Pin_10);
        }
}

static void CAN_device_init_2(int baud, bool termination_enabled)
{
        CAN_DeInit(CAN2);

        /* CAN GPIOs configuration ************************************************* */
        /* Enable GPIO clock */
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

        /* Connect CAN pins to Alternate Function */
        GPIO_PinAFConfig(GPIOB, GPIO_PinSource12, GPIO_AF_CAN2);
        GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_CAN2);
        init_GPIO_CAN(GPIOB, GPIO_Pin_12 | GPIO_Pin_13);

        /* CAN configuration ******************************************************* */
        /* Enable CAN clock */
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN2, ENABLE);

        init_CAN(CAN2, baud);

        /* Enable FIFO 0 message pending Interrupt */
        CAN_ITConfig(CAN2, CAN_IT_FMP1, ENABLE);

        init_CAN_interrupts(CAN2, CAN2_RX1_IRQn);

        /* Configure soft termination */
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
        init_GPIO_port(GPIOB, GPIO_Pin_11);
        if (termination_enabled) {
            GPIO_SetBits(GPIOB, GPIO_Pin_11);
        }
        else {
            GPIO_ResetBits(GPIOB, GPIO_Pin_11);
        }
}

int CAN_device_init(const uint8_t channel, const uint32_t baud, const bool termination_enabled)
{
        pr_info(_LOG_PFX "Initializing CAN");
        pr_info_int(channel);
        pr_info_int_msg(" with baud rate ", baud);

        if (!init_queue()) {
            pr_info(_LOG_PFX "CAN init queue failed\r\n");
            return 0;
        }

        switch (channel) {
        case 0:
            CAN_device_init_1(baud, termination_enabled);
            break;
        case 1:
            CAN_device_init_2(baud, termination_enabled);
            break;
        default:
            pr_info(_LOG_PFX "CAN init device failed\r\n");
            return 0;
        }

        /* Clear out all filter values except 0.  It accepts all. */
        CAN_device_set_filter(channel, 0, 1, 0, 0, true);
        for (size_t i = 1; i < CAN_FILTER_COUNT; ++i)
            CAN_device_set_filter(channel, i, 0, 0, 0, false);

        pr_info(_LOG_PFX "CAN init success!\r\n");
        return 1;
}

int CAN_device_set_filter(const uint8_t channel, const uint8_t id, const uint8_t extended,
			  const uint32_t filter, const uint32_t mask, const bool enabled)
{
        if (channel > 1)
            return 0;

        if (id > 13)
            return 0;

        /*
         * The mapping for these filters/masks is wonkey.  See page
         * 1080 of the stm32f4 reference guide for details.
         * - STD CAN ID -> Bits [31:21]
         * - EXT CAN ID -> Bits [31:03]
         */

        CAN_FilterInitTypeDef CAN_filter_init_structure;
        /* CAN2 filters start at 14 by default */
        CAN_filter_init_structure.CAN_FilterNumber = (channel == 1) ? id + 14 : id;
        CAN_filter_init_structure.CAN_FilterMode = CAN_FilterMode_IdMask;
        CAN_filter_init_structure.CAN_FilterScale = CAN_FilterScale_32bit;
        CAN_filter_init_structure.CAN_FilterFIFOAssignment =
            (channel == 0 ? CAN_FIFO0 : CAN_FIFO1);
        CAN_filter_init_structure.CAN_FilterActivation =
            enabled ? ENABLE : DISABLE;

        const size_t shift = extended ? 3 : 21;
        CAN_filter_init_structure.CAN_FilterIdHigh = (filter << shift) >> 16;
        CAN_filter_init_structure.CAN_FilterMaskIdHigh = (mask << shift) >> 16;
        CAN_filter_init_structure.CAN_FilterIdLow = (uint16_t) filter;
        CAN_filter_init_structure.CAN_FilterMaskIdLow = (uint16_t) mask;

        CAN_FilterInit(&CAN_filter_init_structure);

        return 1;
}

int CAN_device_tx_msg(const uint8_t channel, const CAN_msg * msg, const unsigned int timeout_ms)
{
        CanTxMsg TxMessage;

        /* Transmit Structure preparation */
        if (msg->isExtendedAddress) {
                TxMessage.ExtId = msg->addressValue;
                TxMessage.IDE = CAN_ID_EXT;
        } else {
                TxMessage.StdId = msg->addressValue;
                TxMessage.IDE = CAN_ID_STD;
        }

        TxMessage.RTR = CAN_RTR_DATA;
        TxMessage.DLC = msg->dataLength;
        memcpy(TxMessage.Data, msg->data, msg->dataLength);

        CAN_TypeDef* chan = channel == 0 ? CAN1 : CAN2;
        const uint8_t mailbox = CAN_Transmit(chan, &TxMessage);

        /*
         * Then they don't want to wait.  Ok.  Let caller know if they
         * got a mailbox then.  If not, message was unable to be sent.
         */
        if (0 == timeout_ms)
                return mailbox != CAN_TxStatus_NoMailBox;

        /* Using ticks avoids a race-condition */
        size_t ticks = getCurrentTicks();
        const size_t trigger = ticks + msToTicks(timeout_ms);
        uint8_t status = CAN_TxStatus_Failed;

        while(ticks <= trigger) {
                status = CAN_TransmitStatus(chan, mailbox);
                if (CAN_TxStatus_Pending != status)
                        break;

                /*
                 * Not using yield here as it will cause lower priority tasks
                 * to starve.  Yield only allows tasks of equal or greater
                 * priority to run.
                 */
                delayTicks(1);

                ticks = getCurrentTicks();
        }

        if (CAN_TxStatus_Pending == status)
                CAN_CancelTransmit(chan, mailbox);

        return status == CAN_TxStatus_Ok;
}

int CAN_device_rx_msg(CAN_msg * msg, const unsigned int timeout_ms)
{
        if (pdTRUE == xQueueReceive(can_rx_queue, msg, msToTicks(timeout_ms))) {
            return 1;
        } else {
            pr_debug(_LOG_PFX "timeout rx CAN msg\r\n");
            return 0;
        }
}

static void process_can_irq_rx(uint8_t can_bus, CAN_TypeDef* can_x, uint8_t fifo_number)
{
        portBASE_TYPE task_woken_by_rx = pdFALSE;
        CanRxMsg rx_msg;
        CAN_Receive(can_x, fifo_number, &rx_msg);

        /* translate into a higher level CAN message */
        CAN_msg can_msg;
        can_msg.can_bus = can_bus;
        can_msg.isExtendedAddress = rx_msg.IDE == CAN_ID_EXT;
        can_msg.addressValue = can_msg.isExtendedAddress ? rx_msg.ExtId : rx_msg.StdId;
        memcpy(can_msg.data, rx_msg.Data, rx_msg.DLC);
        can_msg.dataLength = rx_msg.DLC;

        xQueueSendFromISR(can_rx_queue, &can_msg, &task_woken_by_rx);
        portEND_SWITCHING_ISR(task_woken_by_rx);
}

void CAN1_RX0_IRQHandler(void)
{
        process_can_irq_rx(0, CAN1, CAN_FIFO0);
}

void CAN2_RX1_IRQHandler(void)
{
        process_can_irq_rx(1, CAN2, CAN_FIFO1);
}
