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

#include <FreeRTOS.h>
#include <portmacro.h>
#include <queue.h>
#include <semphr.h>
#include <task.h>
#include <timers.h>

#include <hw_config.h>
#include <USB-CDC_device.h>
#include <usb_desc.h>
#include <usb_istr.h>
#include <usb_lib.h>
#include <usb_mem.h>
#include <usb_prop.h>
#include <usb_pwr.h>

#include <stdbool.h>
#include <string.h>


#define USB_BUF_ELTS(in, out, bufsize) ((in - out + bufsize) % bufsize)

static struct {
	uint8_t USB_Rx_Buffer[VIRTUAL_COM_PORT_DATA_SIZE];
	uint8_t USB_Tx_Buffer[VIRTUAL_COM_PORT_DATA_SIZE];
	volatile uint32_t USB_Tx_ptr_in;
	volatile uint32_t USB_Tx_ptr_out;
	volatile uint32_t USB_Rx_ptr_in;
	volatile uint32_t USB_Rx_ptr_out;
	volatile int _init_flag;
	xSemaphoreHandle _rlock;
	xSemaphoreHandle _wlock;
} usb_state;

/*
 * ST's USB hardware doesn't include an internal pullup resistor on
 * USB_DP for some odd reason and instead relies on an externally
 * gated (via a set of transistors) pull up attached to a
 * USB_DISCONNECT pin. As this is only used during startup (we don't
 * do any dynamic re-enumeration), we can shortcut this by having a
 * static pullup, and simply causing a glitch on the line by forcing
 * USB_DP low before we initialize the rest of the USB hardware. It's
 * a little hacky, but doesn't seem to cause any ill effects.
 */
static int USB_CDC_force_reenumeration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

 	GPIO_ResetBits(GPIOA, GPIO_Pin_12);

	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

}

/* Public API */
int USB_CDC_device_init(const int priority)
{
	/* Perform a full USB reset */
	USB_CDC_force_reenumeration();

	/* Create read/write mutexes */
	usb_state._rlock = xSemaphoreCreateMutex();
	usb_state._wlock = xSemaphoreCreateMutex();

	if (NULL == usb_state._rlock || NULL == usb_state._wlock) {
		return -1;
	}

	Set_System();
	Set_USBClock();

	USB_Interrupts_Config();

	USB_Init();

	usb_state._init_flag = 1;

	return 0;
}

void USB_CDC_Write(portCHAR *cByte, int len)
{
	xSemaphoreTake(usb_state._wlock, portMAX_DELAY);
	while (len--) {
		/*
		 * If the transmit buffer is full, wait until the USB
		 * hardware has had time to flush it out
		 */
		uint16_t usb_buf_elements = USB_BUF_ELTS(usb_state.USB_Tx_ptr_in,
							 usb_state.USB_Tx_ptr_out,
							 VIRTUAL_COM_PORT_DATA_SIZE);

		while (VIRTUAL_COM_PORT_DATA_SIZE - 1 == usb_buf_elements) {
			portYIELD();
			usb_buf_elements = USB_BUF_ELTS(usb_state.USB_Tx_ptr_in,
							usb_state.USB_Tx_ptr_out,
							VIRTUAL_COM_PORT_DATA_SIZE);
		}

		usb_state.USB_Tx_Buffer[usb_state.USB_Tx_ptr_in] = *cByte++;

		/* Handle wrapping */
		if(VIRTUAL_COM_PORT_DATA_SIZE - 1 == usb_state.USB_Tx_ptr_in) {
			usb_state.USB_Tx_ptr_in = 0;
		} else {
			usb_state.USB_Tx_ptr_in++;
		}

	}
	xSemaphoreGive(usb_state._wlock);
}

void USB_CDC_SendByte( portCHAR cByte )
{
	USB_CDC_Write(&cByte, 1);
}

portBASE_TYPE USB_CDC_ReceiveByte(portCHAR *data)
{
	return USB_CDC_ReceiveByteDelay(data, 0);
}

portBASE_TYPE USB_CDC_ReceiveByteDelay(portCHAR *data, portTickType delay )
{
	xSemaphoreTake(usb_state._rlock, portMAX_DELAY);
	int ret = 1;
	bool byte_received = false;

	while (!byte_received) {
		if (usb_state.USB_Rx_ptr_out != usb_state.USB_Rx_ptr_in) {
			*data = usb_state.USB_Rx_Buffer[usb_state.USB_Rx_ptr_out++];
			/*
			 * If we've cleared the buffer, send a signal to the
			 * USB hardware to stop NAKing packets and to
			 * continue receiving
			 */
			if (usb_state.USB_Rx_ptr_out == usb_state.USB_Rx_ptr_in) {
				/* Enable the receive of data on EP3 */
				SetEPRxValid(ENDP3);
			}

			/*
			 * Note: We don't need to handle wrapping in
			 * the receive calls because we always fill
			 * the rx buffer from the start and drain to
			 * completion
			 */
			byte_received = true;
		} else {
			if (delay == portMAX_DELAY) {
				portYIELD();
			} else if (delay) {
				portYIELD();
				delay--;
			} else {
				ret = 0;
				goto unlock_and_return;
			}
		}
	}

unlock_and_return:
	xSemaphoreGive(usb_state._rlock);

	return ret;
}

int USB_CDC_is_initialized()
{
	return usb_state._init_flag;
}

/* Private USB Stack callbacks */

/*
 * This code has been adapted from the ST Microelectronics CDC
 * Example, which is covered under the V2 Liberty License:
 * http://www.st.com/software_license_agreement_liberty_v2
 */
void usb_handle_transfer(void)
{
	uint16_t USB_Tx_length = USB_BUF_ELTS(usb_state.USB_Tx_ptr_in,
					      usb_state.USB_Tx_ptr_out,
					      VIRTUAL_COM_PORT_DATA_SIZE);

	if(0 == USB_Tx_length) {
		return;
	}

	/*
	 * Handle the situation where we can only transmit to the end
	 * of the buffer in this packet
	 */
	if(usb_state.USB_Tx_ptr_out > usb_state.USB_Tx_ptr_in) {
		USB_Tx_length = (VIRTUAL_COM_PORT_DATA_SIZE -
				 usb_state.USB_Tx_ptr_out);
	}

	UserToPMABufferCopy(&usb_state.USB_Tx_Buffer[usb_state.USB_Tx_ptr_out],
			    ENDP1_TXADDR, USB_Tx_length);
	usb_state.USB_Tx_ptr_out += USB_Tx_length;

	/* Handle wrapping */
	if (VIRTUAL_COM_PORT_DATA_SIZE == usb_state.USB_Tx_ptr_out) {
		usb_state.USB_Tx_ptr_out = 0;
	}

	SetEPTxCount(ENDP1, USB_Tx_length);
	SetEPTxValid(ENDP1);
}

void EP1_IN_Callback (void)
{
	usb_handle_transfer();
}

void EP3_OUT_Callback(void)
{
	/* Get the received data buffer and clear the counter */
	usb_state.USB_Rx_ptr_in = USB_SIL_Read(EP3_OUT, usb_state.USB_Rx_Buffer);
	usb_state.USB_Rx_ptr_out = 0;

	/*
	 * USB data will be processed by handler threads, we will
	 * continue to NAK packets until such a time as all of the
	 * prior data has been handled
	 */
}

void SOF_Callback(void)
{
	static uint32_t FrameCount = 0;

	if(bDeviceState == CONFIGURED) {
		if (VIRTUAL_COM_PORT_IN_FRAME_INTERVAL == FrameCount++) {
			/* Reset the frame counter */
			FrameCount = 0;

			/* Check the data to be sent through IN pipe */
			usb_handle_transfer();
		}
         }
}
