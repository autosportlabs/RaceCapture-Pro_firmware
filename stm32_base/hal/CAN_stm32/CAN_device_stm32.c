#include "CAN_device.h"
#include <stdint.h>
#include <mod_string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "stm32f4xx_can.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_misc.h"
#include "taskUtil.h"
#include "printk.h"
#include "LED.h"

static xQueueHandle xCan1Tx = NULL;
static xQueueHandle xCan1Rx = NULL;

static xQueueHandle xCan2Tx = NULL;
static xQueueHandle xCan2Rx = NULL;

#define CAN_QUEUE_LENGTH 10

#define CAN_IRQ_PRIORITY 		5
#define CAN_IRQ_SUB_PRIORITY 	0

//For 168MHz clock
/*       BS1 BS2 SJW Pre
 1M:      12  8   1   2
 500k:    8   5   1   6
 250k:    8   5   1   12
 125k:    12  8   1   16
 100k:    12  8   1   20 */

#define CAN_BAUD_COUNT 5
static const u8 can_baud_bs1[] = { CAN_BS1_12tq, CAN_BS1_12tq, CAN_BS1_8tq,	CAN_BS1_8tq, CAN_BS1_12tq };
static const u8 can_baud_bs2[] = { CAN_BS1_8tq, CAN_BS1_8tq, CAN_BS1_5tq, CAN_BS1_5tq, CAN_BS1_8tq };
static const u8 can_baud_sjw[] = { CAN_SJW_1tq, CAN_SJW_1tq, CAN_SJW_1tq, CAN_SJW_1tq, CAN_SJW_1tq };
static const u8 can_baud_pre[] = { 20, 16, 12, 6, 2 };
static const u32 can_baud_rate[] = { 100000, 125000, 250000, 500000, 1000000 };

static int initQueues() {
	int success = 1;

	if (! (xCan1Rx && xCan1Tx && xCan2Rx && xCan2Tx)){
		xCan1Rx = xQueueCreate(CAN_QUEUE_LENGTH,
				( unsigned portBASE_TYPE ) sizeof( CanRxMsg ));
		xCan1Tx = xQueueCreate(CAN_QUEUE_LENGTH + 1,
				( unsigned portBASE_TYPE ) sizeof( CanRxMsg ));

		xCan2Rx = xQueueCreate(CAN_QUEUE_LENGTH,
				( unsigned portBASE_TYPE ) sizeof( CanRxMsg ));
		xCan2Tx = xQueueCreate(CAN_QUEUE_LENGTH + 1,
				( unsigned portBASE_TYPE ) sizeof( CanRxMsg ));
	}

	if (xCan1Rx == NULL || xCan1Rx == NULL || xCan2Rx == NULL || xCan2Rx == NULL) {
		success = 0;
	}
	return success;
}

static void initGPIO(GPIO_TypeDef* GPIOx, uint32_t gpioPins){
	/* Configure CAN RX and TX pins */
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = gpioPins;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOx, &GPIO_InitStructure);
}

static void initCAN(CAN_TypeDef* CANx, int baud){

	CAN_InitTypeDef CAN_InitStructure;
	/* CAN cell init */
	CAN_InitStructure.CAN_TTCM = DISABLE;
	CAN_InitStructure.CAN_ABOM = DISABLE;
	CAN_InitStructure.CAN_AWUM = DISABLE;
	CAN_InitStructure.CAN_NART = DISABLE;
	CAN_InitStructure.CAN_RFLM = DISABLE;
	CAN_InitStructure.CAN_TXFP = DISABLE;
	CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;

	int baudIndex = -1;
	// Select baud rate up to requested rate, except for below min, where min is selected
	if (baud >= can_baud_rate[ CAN_BAUD_COUNT - 1]) // round down to peak rate if >= peak rate
		baudIndex = CAN_BAUD_COUNT - 1;
	else {
		for (baudIndex = 0; baudIndex < CAN_BAUD_COUNT - 1; baudIndex++) {
			if (baud < can_baud_rate[baudIndex + 1]) // take current idx if next is too large
				break;
		}
	}

	CAN_InitStructure.CAN_SJW = can_baud_sjw[baudIndex];
	CAN_InitStructure.CAN_BS1 = can_baud_bs1[baudIndex];
	CAN_InitStructure.CAN_BS2 = can_baud_bs2[baudIndex];
	CAN_InitStructure.CAN_Prescaler = can_baud_pre[baudIndex];

	CAN_Init(CANx, &CAN_InitStructure);

}

static void initCANInterrupts(CAN_TypeDef* CANx, uint8_t irqNumber){

	NVIC_InitTypeDef  NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = irqNumber;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = CAN_IRQ_PRIORITY;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = CAN_IRQ_SUB_PRIORITY;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

static void CAN_device_init_1(int baud) {
	CAN_DeInit(CAN1);

	/* CAN GPIOs configuration **************************************************/
	/* Enable GPIO clock */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

	/* Connect CAN pins to Alternate Function */
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource0, GPIO_AF_CAN1);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource1, GPIO_AF_CAN1);
	initGPIO(GPIOD, GPIO_Pin_0 | GPIO_Pin_1);

	/* CAN configuration ********************************************************/
	/* Enable CAN clock */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);

	initCAN(CAN1, baud);

	//set default filter
	CAN_device_set_filter(0, 0, 0, 0, 0);

	/* Enable FIFO 0 message pending Interrupt */
	CAN_ITConfig(CAN1, CAN_IT_FMP0, ENABLE);

	initCANInterrupts(CAN1, CAN1_RX0_IRQn);
}

static void CAN_device_init_2(int baud) {
	CAN_DeInit(CAN2);

	/* CAN GPIOs configuration **************************************************/
	/* Enable GPIO clock */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

	/* Connect CAN pins to Alternate Function */
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource12, GPIO_AF_CAN2);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_CAN2);
	initGPIO(GPIOB, GPIO_Pin_12 | GPIO_Pin_13);

	/* CAN configuration ********************************************************/
	/* Enable CAN clock */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN2, ENABLE);

	initCAN(CAN2, baud);

	//set default filter
	CAN_device_set_filter(1, 0, 0, 0, 0);

	/* Enable FIFO 0 message pending Interrupt */
	CAN_ITConfig(CAN2, CAN_IT_FMP1, ENABLE);

	initCANInterrupts(CAN2, CAN2_RX1_IRQn);
}

int CAN_device_init(uint8_t channel, uint32_t baud){

	pr_info("CAN");
	pr_info_int(channel);
	pr_info(" init @ ");
	pr_info_int(baud);
	if (initQueues()){
		switch(channel){
		case 0:
			CAN_device_init_1(baud);
			break;
		case 1:
			CAN_device_init_2(baud);
			break;
		}
		pr_info(" win\r\n");
		return 1;
	}
	else{
		pr_info(" fail\r\n");
		return 0;
	}
}

int CAN_device_set_filter(uint8_t channel, uint8_t id, uint8_t extended, uint32_t filter, uint32_t mask) {

	if (channel > 1) return 0;
	if (id > 13) return 0;

	if (channel == 1) id += 14; //CAN2 filters start at 14 by default

	CAN_FilterInitTypeDef CAN_FilterInitStructure;
	CAN_FilterInitStructure.CAN_FilterNumber = id;
	CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;
	CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;
	CAN_FilterInitStructure.CAN_FilterIdHigh = filter >> 16;
	CAN_FilterInitStructure.CAN_FilterIdLow = filter & 0xFFFF;
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh = mask >> 16;
	CAN_FilterInitStructure.CAN_FilterMaskIdLow = mask & 0xFFFF;
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment = channel == 0 ? CAN_FIFO0 : CAN_FIFO1;
	CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
	CAN_FilterInit(&CAN_FilterInitStructure);
	return 1;
}

int CAN_device_tx_msg(uint8_t channel, CAN_msg *msg, unsigned int timeoutMs) {
	CanTxMsg TxMessage;
	/* Transmit Structure preparation */
	TxMessage.StdId = msg->addressValue;
	TxMessage.ExtId = 0x00;
	TxMessage.RTR = CAN_RTR_DATA;
	TxMessage.IDE = (msg->isExtendedAddress ? CAN_ID_EXT : CAN_ID_STD);
	TxMessage.DLC = msg->dataLength;

	memcpy(TxMessage.Data, msg->data, msg->dataLength);
	CAN_Transmit(channel == 0 ? CAN1 : CAN2, &TxMessage);
	return 1;
}

int CAN_device_rx_msg(uint8_t channel, CAN_msg *msg, unsigned int timeoutMs) {
	CanRxMsg rxMsg;
	if (xQueueReceive(channel == 0 ? xCan1Rx : xCan2Rx, &rxMsg, msToTicks(timeoutMs)) == pdTRUE) {
		msg->isExtendedAddress = rxMsg.IDE == CAN_ID_EXT ? 1 : 0;
		uint32_t address = rxMsg.StdId;
		if (msg->isExtendedAddress) {
			address = (address << 18) | rxMsg.ExtId;
		}
		msg->addressValue = 0x1FFFFFFF & address; // mask out extra bits
		memcpy(msg->data, rxMsg.Data, rxMsg.DLC);
		msg->dataLength = rxMsg.DLC;
		return 1;
	} else {
		pr_debug("timeout rx CAN msg\r\n");
		return 0;
	}
}

void CAN1_RX0_IRQHandler(void) {
	portBASE_TYPE xTaskWokenByRx = pdFALSE;
	CanRxMsg rxMsg;
	CAN_Receive(CAN1, CAN_FIFO0, &rxMsg);
	xQueueSendFromISR(xCan1Rx, &rxMsg, &xTaskWokenByRx);
	portEND_SWITCHING_ISR(xTaskWokenByRx);
}

void CAN2_RX1_IRQHandler(void) {
	portBASE_TYPE xTaskWokenByRx = pdFALSE;
	CanRxMsg rxMsg;
	CAN_Receive(CAN2, CAN_FIFO1, &rxMsg);
	xQueueSendFromISR(xCan2Rx, &rxMsg, &xTaskWokenByRx);
	portEND_SWITCHING_ISR(xTaskWokenByRx);
}
