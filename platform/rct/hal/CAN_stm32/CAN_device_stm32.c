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
#include "led.h"
#include "mod_string.h"
#include "printk.h"
#include "queue.h"
#include "stm32f30x.h"
#include "stm32f30x_can.h"
#include "stm32f30x_gpio.h"
#include "stm32f30x_misc.h"
#include "stm32f30x_rcc.h"
#include "task.h"
#include "taskUtil.h"
#include <stdint.h>

static xQueueHandle xCan1Rx;

#define CAN_FILTER_COUNT	13
#define CAN_IRQ_PRIORITY 	5
#define CAN_IRQ_SUB_PRIORITY 	0
#define CAN_QUEUE_LENGTH	10

//For 36MHz clock
/*       BS1 BS2 SJW Pre
 1M:      15  2   1   2
 500k:    6   1   1   9
 250k:    13  2   1   9
 125k:    13  2   1   18
 100k:    15  2   1   20 */

#define CAN_BAUD_COUNT 5
static const u8 can_baud_bs1[] =
{ CAN_BS1_15tq, CAN_BS1_13tq, CAN_BS1_13tq, CAN_BS1_6tq, CAN_BS1_15tq };

static const u8 can_baud_bs2[] =
{ CAN_BS1_2tq, CAN_BS1_2tq, CAN_BS1_2tq, CAN_BS1_1tq, CAN_BS1_2tq };

static const u8 can_baud_sjw[] =
{ CAN_SJW_1tq, CAN_SJW_1tq, CAN_SJW_1tq, CAN_SJW_1tq, CAN_SJW_1tq };

static const u8 can_baud_pre[] = { 20, 18, 9, 9, 2 };
static const u32 can_baud_rate[] = { 100000, 125000, 250000, 500000, 1000000 };

static int initQueues()
{
        if (!xCan1Rx) {
                xCan1Rx = xQueueCreate(CAN_QUEUE_LENGTH,
                                       (unsigned portBASE_TYPE)
                                       sizeof(CanRxMsg));
                if (!xCan1Rx)
                        return 0;
        }

        return 1;
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
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);

    /* Connect CAN pins to Alternate Function */
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource8, GPIO_AF_9);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource9, GPIO_AF_9);
    initGPIO(GPIOB, GPIO_Pin_8 | GPIO_Pin_9);

    /* CAN configuration ******************************************************* */
    /* Enable CAN clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);

    initCAN(CAN1, baud);

    /* Enable FIFO 0 message pending Interrupt */
    CAN_ITConfig(CAN1, CAN_IT_FMP0, ENABLE);

    initCANInterrupts(CAN1, USB_LP_CAN1_RX0_IRQn);
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

	/*
	 * The mapping for these filters/masks is wonkey.  See page
	 * 1026 of the stm32f3 reference guide for details.
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
	if (channel != 0)
		return 0;

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
	const uint8_t mailbox = CAN_Transmit(CAN1, &TxMessage);

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
                status = CAN_TransmitStatus(CAN1, mailbox);
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
                CAN_CancelTransmit(CAN1, mailbox);

        return status == CAN_TxStatus_Ok;

}

int CAN_device_rx_msg(uint8_t channel, CAN_msg * msg, unsigned int timeoutMs)
{
	if (channel > 0) {
		return 0;
	}

    CanRxMsg rxMsg;
    if (xQueueReceive(xCan1Rx, &rxMsg, msToTicks(timeoutMs)) == pdTRUE) {
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

void CAN_device_isr(void)
{
    if (CAN_GetITStatus(CAN1, CAN_IT_FMP0) != RESET)
    {
        portBASE_TYPE xTaskWokenByRx = pdFALSE;
        CanRxMsg rxMsg;
        CAN_Receive(CAN1, CAN_FIFO0, &rxMsg);
        xQueueSendFromISR(xCan1Rx, &rxMsg, &xTaskWokenByRx);
        portEND_SWITCHING_ISR(xTaskWokenByRx);
    }
}
