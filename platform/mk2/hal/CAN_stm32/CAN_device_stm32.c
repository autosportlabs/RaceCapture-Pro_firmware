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

static xQueueHandle xCan1Rx = NULL;
static xQueueHandle xCan2Rx = NULL;

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

static int initQueues()
{
        if (!xCan1Rx)
                xCan1Rx = xQueueCreate(CAN_QUEUE_LENGTH, sizeof(CanRxMsg));

        if (!xCan2Rx)
                xCan2Rx = xQueueCreate(CAN_QUEUE_LENGTH, sizeof(CanRxMsg));

        return xCan1Rx && xCan2Rx;
}

static void initGPIO(GPIO_TypeDef * GPIOx, uint32_t gpioPins)
{
    /* Configure CAN RX and TX pins */
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = gpioPins;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOx, &GPIO_InitStructure);
}

static void initCAN(CAN_TypeDef * CANx, uint32_t baud)
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

    int baudIndex = -1;

    /* Select baud rate up to requested rate, except for below min, where min is selected */
    if (baud >= can_baud_rate[CAN_BAUD_COUNT - 1]) {
        /* round down to peak rate if >= peak rate */
        baudIndex = CAN_BAUD_COUNT - 1;
    } else {
        for (baudIndex = 0; baudIndex < CAN_BAUD_COUNT - 1; baudIndex++) {
            if (baud < can_baud_rate[baudIndex + 1]) {
                /* take current idx if next is too large */
                break;
            }
        }
    }

    CAN_InitStructure.CAN_SJW = can_baud_sjw[baudIndex];
    CAN_InitStructure.CAN_BS1 = can_baud_bs1[baudIndex];
    CAN_InitStructure.CAN_BS2 = can_baud_bs2[baudIndex];
    CAN_InitStructure.CAN_Prescaler = can_baud_pre[baudIndex];

    CAN_Init(CANx, &CAN_InitStructure);
}

static void initCANInterrupts(CAN_TypeDef * CANx, uint8_t irqNumber)
{

    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = irqNumber;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = CAN_IRQ_PRIORITY;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = CAN_IRQ_SUB_PRIORITY;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

static void CAN_device_init_1(int baud)
{
    CAN_DeInit(CAN1);

    /* CAN GPIOs configuration ************************************************* */
    /* Enable GPIO clock */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

    /* Connect CAN pins to Alternate Function */
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource0, GPIO_AF_CAN1);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource1, GPIO_AF_CAN1);
    initGPIO(GPIOD, GPIO_Pin_0 | GPIO_Pin_1);

    /* CAN configuration ******************************************************* */
    /* Enable CAN clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);

    initCAN(CAN1, baud);

    /* Enable FIFO 0 message pending Interrupt */
    CAN_ITConfig(CAN1, CAN_IT_FMP0, ENABLE);

    initCANInterrupts(CAN1, CAN1_RX0_IRQn);
}

static void CAN_device_init_2(int baud)
{
    CAN_DeInit(CAN2);

    /* CAN GPIOs configuration ************************************************* */
    /* Enable GPIO clock */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

    /* Connect CAN pins to Alternate Function */
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource12, GPIO_AF_CAN2);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_CAN2);
    initGPIO(GPIOB, GPIO_Pin_12 | GPIO_Pin_13);

    /* CAN configuration ******************************************************* */
    /* Enable CAN clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN2, ENABLE);

    initCAN(CAN2, baud);

    /* Enable FIFO 0 message pending Interrupt */
    CAN_ITConfig(CAN2, CAN_IT_FMP1, ENABLE);

    initCANInterrupts(CAN2, CAN2_RX1_IRQn);
}

int CAN_device_init(uint8_t channel, uint32_t baud)
{
	pr_info("Initializing CAN");
	pr_info_int(channel);
	pr_info_int_msg(" with baud rate ", baud);

	if (!initQueues()) {
		pr_info("CAN init queues failed\r\n");
		return 0;
	}

	switch (channel) {
	case 0:
		CAN_device_init_1(baud);
		break;
	case 1:
		CAN_device_init_2(baud);
		break;
	default:
		pr_info("CAN init device failed\r\n");
		return 0;
	}

	/* Clear out all filter values except 0.  It accepts all. */
	CAN_device_set_filter(channel, 0, 1, 0, 0, true);
	for (size_t i = 1; i < CAN_FILTER_COUNT; ++i)
		CAN_device_set_filter(channel, i, 0, 0, 0, false);

	pr_info("CAN init success!\r\n");
	return 1;
}

int CAN_device_set_filter(uint8_t channel, uint8_t id, uint8_t extended,
			  uint32_t filter, uint32_t mask, const bool enabled)
{
	if (channel > 1)
		return 0;

	if (id > 13)
		return 0;

	/* CAN2 filters start at 14 by default */
	if (channel == 1)
		id += 14;

	/*
	 * The mapping for these filters/masks is wonkey.  See page
	 * 1080 of the stm32f4 reference guide for details.
	 * - STD CAN ID -> Bits [31:21]
	 * - EXT CAN ID -> Bits [31:03]
	 */

	CAN_FilterInitTypeDef CAN_FilterInitStructure;
	CAN_FilterInitStructure.CAN_FilterNumber = id;
	CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;
	CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment =
		(channel == 0 ? CAN_FIFO0 : CAN_FIFO1);
	CAN_FilterInitStructure.CAN_FilterActivation =
		enabled ? ENABLE : DISABLE;

	const size_t shift = extended ? 3 : 21;
	filter <<= shift;
	mask <<= shift;
	CAN_FilterInitStructure.CAN_FilterIdHigh = filter >> 16;
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh = mask >> 16;
	CAN_FilterInitStructure.CAN_FilterIdLow = (uint16_t) filter;
	CAN_FilterInitStructure.CAN_FilterMaskIdLow = (uint16_t) mask;

	CAN_FilterInit(&CAN_FilterInitStructure);

	return 1;
}

int CAN_device_tx_msg(uint8_t channel, CAN_msg * msg, unsigned int timeoutMs)
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
        if (0 == timeoutMs)
                return mailbox != CAN_TxStatus_NoMailBox;

        /* Using ticks avoids a race-condition */
        size_t ticks = getCurrentTicks();
        const size_t trigger = ticks + msToTicks(timeoutMs);
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

int CAN_device_rx_msg(uint8_t channel, CAN_msg * msg, unsigned int timeoutMs)
{
    CanRxMsg rxMsg;
    if (xQueueReceive(channel == 0 ? xCan1Rx : xCan2Rx, &rxMsg, msToTicks(timeoutMs)) == pdTRUE) {
        msg->isExtendedAddress = rxMsg.IDE == CAN_ID_EXT ? 1 : 0;
        msg->addressValue = msg->isExtendedAddress ? rxMsg.ExtId : rxMsg.StdId;
        memcpy(msg->data, rxMsg.Data, rxMsg.DLC);
        msg->dataLength = rxMsg.DLC;
        return 1;
    } else {
        pr_debug("timeout rx CAN msg\r\n");
        return 0;
    }
}

void CAN1_RX0_IRQHandler(void)
{
    portBASE_TYPE xTaskWokenByRx = pdFALSE;
    CanRxMsg rxMsg;
    CAN_Receive(CAN1, CAN_FIFO0, &rxMsg);
    xQueueSendFromISR(xCan1Rx, &rxMsg, &xTaskWokenByRx);
    portEND_SWITCHING_ISR(xTaskWokenByRx);
}

void CAN2_RX1_IRQHandler(void)
{
    portBASE_TYPE xTaskWokenByRx = pdFALSE;
    CanRxMsg rxMsg;
    CAN_Receive(CAN2, CAN_FIFO1, &rxMsg);
    xQueueSendFromISR(xCan2Rx, &rxMsg, &xTaskWokenByRx);
    portEND_SWITCHING_ISR(xTaskWokenByRx);
}
