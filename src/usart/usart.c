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

#include "serial.h"
#include "usart.h"
#include "usart_device.h"

#include <stdint.h>

int usart_init()
{
        return usart_device_init();
}

struct Serial* usart_get_serial(const uart_id_t port)
{
        return usart_device_get_serial(port);
}

void usart_config(const uart_id_t id, const size_t bits,
                  const size_t parity, const size_t stop_bits,
                  const size_t baud)
{
        usart_device_config(id, bits, parity, stop_bits, baud);
}
