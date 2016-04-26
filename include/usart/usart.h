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

#ifndef USART_H
#define USART_H

#include "cpp_guard.h"
#include "serial.h"

#include <stdint.h>

CPP_GUARD_BEGIN

/*
 * This must remain a 0 based enum as its values are used to
 * retrieve data from an internal array.
 */
typedef enum {
    UART_GPS = 0,
    UART_TELEMETRY,
    UART_WIRELESS,
    UART_AUX,
    __UART_COUNT, /* Maximum number of UART devices */
} uart_id_t;

int usart_init();

struct Serial* get_serial(uart_id_t id);

void usart_config(const uart_id_t id, const size_t bits,
                  const size_t parity, const size_t stop_bits,
                  const size_t baud);

CPP_GUARD_END

#endif //#ifndef USART_H
