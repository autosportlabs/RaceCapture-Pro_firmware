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
#include "loggerConfig.h"
#include "serial.h"
#include "serial_device.h"
#include "usart.h"
#include "usart_device.h"

#include <stddef.h>
#include <stdbool.h>

struct Serial* serial_device_get(const serial_id_t port)
{
        struct Serial *s = NULL;
        switch(port) {
        case SERIAL_USB:
#if USB_SERIAL_SUPPORT
                s =  USB_CDC_get_serial();
#endif
                break;
        case SERIAL_GPS:
                s = usart_device_get_serial(UART_GPS);
                break;
        case SERIAL_TELEMETRY:
                s = usart_device_get_serial(UART_TELEMETRY);
                break;
        case SERIAL_BLUETOOTH:
                s = usart_device_get_serial(UART_WIRELESS);
                break;
        case SERIAL_WIFI:
        case SERIAL_AUX:
                s = usart_device_get_serial(UART_AUX);
                break;
        default:
                s = NULL;
        };

        if (!s)
                return NULL;

        /* Hack to get serial logging working temporarily */
        const bool enable_logs =
                getWorkingLoggerConfig()->logging_cfg.serial[port];
        serial_logging(s, enable_logs);

        return s;
}
