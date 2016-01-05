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


#include "usart.h"
#include "usart_device.h"
#include <stdint.h>

int usart_init()
{
    return usart_device_init();
}

int usart_init_serial(Serial *serial, uart_id_t port)
{
    return usart_device_init_serial(serial, port);
}

void usart_config(uart_id_t port, uint8_t bits, uint8_t parity, uint8_t stopbits, uint32_t baud)
{
    usart_device_config(port, bits, parity, stopbits, baud);
}
