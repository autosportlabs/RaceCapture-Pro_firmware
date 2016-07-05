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
#include <USB-CDC_device.h>
#include <hw_config.h>
#include <portmacro.h>
#include <queue.h>
#include <semphr.h>
#include <stdbool.h>
#include <string.h>
#include <task.h>
#include <timers.h>
#include <usb_desc.h>
#include <usb_istr.h>
#include <usb_lib.h>
#include <usb_mem.h>
#include <usb_prop.h>
#include <usb_pwr.h>

/* STIEG: Make this device support buffered Tx */
#define USB_TX_BUF_CAP	128
#define USB_RX_BUF_CAP	512
#define USB_BUF_ELTS(in, out, bufsize) ((in - out + bufsize) % bufsize)

static struct {
	uint8_t USB_Rx_Buffer[VIRTUAL_COM_PORT_DATA_SIZE];
	uint8_t USB_Tx_Buffer[VIRTUAL_COM_PORT_DATA_SIZE];
        struct Serial *serial;
        usb_device_data_rx_isr_cb_t* rx_isr_cb;
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
        return 0;
}

/* Public API */
int USB_CDC_device_init(const int priority, usb_device_data_rx_isr_cb_t* cb)
{
	/* Perform a full USB reset */
	USB_CDC_force_reenumeration();

	/* Create read/write mutexes */
        usb_state.serial = serial_create("USB", USB_TX_BUF_CAP, USB_RX_BUF_CAP,
                                         NULL, NULL, NULL, NULL);
	if (NULL == usb_state.serial)
		return -1;
        usb_state.rx_isr_cb = cb;

	Set_System();
	Set_USBClock();

	USB_Interrupts_Config();
	USB_Init();

	return 0;
}

struct Serial* USB_CDC_get_serial()
{
        return (struct Serial*) usb_state.serial;
}

int USB_CDC_is_initialized()
{
        return NULL != usb_state.serial;
}

/* Private USB Stack callbacks */

/*
 * This code has been adapted from the ST Microelectronics CDC
 * Example, which is covered under the V2 Liberty License:
 * http://www.st.com/software_license_agreement_liberty_v2
 */
static void usb_handle_transfer(void)
{
        portBASE_TYPE hpta = false;
        xQueueHandle queue = serial_get_tx_queue(usb_state.serial);
        uint8_t *buff = usb_state.USB_Tx_Buffer;
        size_t len = 0;

        for (; len < VIRTUAL_COM_PORT_DATA_SIZE; ++len)
                if (!xQueueReceiveFromISR(queue, buff + len, &hpta))
                        break;

        /* Check if we actually have something to send */
	if (len) {
                UserToPMABufferCopy(usb_state.USB_Tx_Buffer,
                                    ENDP1_TXADDR, len);
                SetEPTxCount(ENDP1, len);
                SetEPTxValid(ENDP1);
        }

        portEND_SWITCHING_ISR(hpta);
}

void EP1_IN_Callback (void)
{
	usb_handle_transfer();
}

void EP3_OUT_Callback(void)
{
        portBASE_TYPE hpta = false;
        xQueueHandle queue = serial_get_rx_queue(usb_state.serial);
        uint8_t *buff = usb_state.USB_Rx_Buffer;

	/* Get the received data buffer and clear the counter */
	const size_t len = USB_SIL_Read(EP3_OUT, buff);

        for (size_t i = 0; i < len; ++i)
                xQueueSendFromISR(queue, buff + i, &hpta);

	/*
         * STIEG HACK
         * For now just assume that all the data made it into the queue.  This
         * is what we do in MK2.  Probably shouldn't go out the door like this.
	 */
        SetEPRxValid(ENDP3);

        if (usb_state.rx_isr_cb)
                usb_state.rx_isr_cb();

        portEND_SWITCHING_ISR(hpta);
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
