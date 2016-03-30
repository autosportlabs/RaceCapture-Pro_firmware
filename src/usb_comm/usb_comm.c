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
#include "USB-CDC_device.h"
#include "messaging.h"
#include "serial.h"
#include "task.h"
#include "usb_comm.h"

#define BUFFER_SIZE	1025
#define mainUSB_COMM_STACK	1000

static char lineBuffer[BUFFER_SIZE];
static struct Serial *serial;

static void usb_comm_task(void *pvParameters)
{
        while (!USB_CDC_is_initialized());

        while (1)
                process_msg(serial, lineBuffer, BUFFER_SIZE);
}

void startUSBCommTask(int priority)
{
        if (serial)
                return;

        USB_CDC_device_init(priority);
        serial = USB_CDC_get_serial();

        xTaskCreate(usb_comm_task, (signed portCHAR *) "USB_Comm",
                    mainUSB_COMM_STACK, NULL, priority, NULL);
}
