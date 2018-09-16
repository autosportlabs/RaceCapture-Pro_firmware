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

#include "USB-CDC_device.h"
#include "panic.h"
#include "printk.h"
#include "serial.h"
#include "serial_device.h"

#include <string.h>
#include <usbd_cdc_core.h>
#include <usbd_cdc_vcp.h>
#include <usbd_conf.h>
#include <usbd_desc.h>
#include <usbd_usr.h>

/* STIEG: Make this device support buffered Tx */
#define USB_TX_BUF_CAP	1
#define USB_RX_BUF_CAP	512

static struct Serial *usb_serial;
USB_OTG_CORE_HANDLE USB_OTG_dev __attribute__ ((aligned (4)));


/**
 * Called after the serial device transmits a character.  Since we don't support
 * buffered tx yet, we simply de-queue the character and send it on its way. Yes
 * its a hack but its the way it was.
 */
static void _post_tx(xQueueHandle q, void *arg)
{
        char c;

        while (xQueueReceive(q, &c, 0))
                vcp_tx((uint8_t*) &c, 1);
}

int USB_CDC_device_init(const int priority, usb_device_data_rx_isr_cb_t* cb)
{
        if (usb_serial) {
                pr_error("[USB] Re-initialized USB Serial!  No bueno\r\n");
                panic(PANIC_CAUSE_MALLOC);
        }

        usb_serial = serial_create("USB", USB_TX_BUF_CAP, USB_RX_BUF_CAP,
                                   NULL, NULL, _post_tx, NULL);
        if (!usb_serial) {
                pr_error("[USB] Serial Malloc failure!\r\n");
                panic(PANIC_CAUSE_MALLOC);
        }

        vcp_setup(usb_serial, cb);

        /* Initialize the USB hardware */
        USBD_Init(&USB_OTG_dev, USB_OTG_FS_CORE_ID,
                  &USR_desc, &USBD_CDC_cb, &USR_cb);

        return 0;
}

int USB_CDC_is_initialized()
{
        return usb_serial != NULL;
}

struct Serial* USB_CDC_get_serial()
{
        return usb_serial;
}
