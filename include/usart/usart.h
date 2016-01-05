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

typedef enum {
    UART_GPS = 0,
    UART_TELEMETRY,
    UART_WIRELESS,
    UART_AUX
} uart_id_t;

#define SERIAL_COUNT 5

int usart_init();
int usart_init_serial(Serial *serial, uart_id_t port);
void usart_config(uart_id_t port, uint8_t bits, uint8_t parity, uint8_t stopbits, uint32_t baud);

CPP_GUARD_END

#endif //#ifndef USART_H
