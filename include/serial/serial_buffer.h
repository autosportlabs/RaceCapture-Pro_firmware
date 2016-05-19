/*
 * Race Capture Pro Firmware
 *
 * Copyright (C) 2015 Autosport Labs
 *
 * This file is part of the Race Capture Pro fimrware suite
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

#ifndef _SERIAL_BUFFER_H_
#define _SERIAL_BUFFER_H_

#include "cpp_guard.h"
#include "serial.h"

#include <stdbool.h>

CPP_GUARD_BEGIN

struct serial_buffer {
        struct Serial *serial;
        size_t length;
        char *buffer;
        size_t curr_len;
};

bool serial_buffer_create(struct serial_buffer *sb,
                          struct Serial *serial,
                          const size_t size,
                          char *buffer);

char* serial_buffer_rx(struct serial_buffer *sb,
                       const size_t ms_delay);

size_t serial_buffer_rx_msgs(struct serial_buffer *sb,
                             const size_t ms_delay,
                             const char *msgs[], size_t msgs_len);

void serial_buffer_flush(struct serial_buffer *sb);

void serial_buffer_clear(struct serial_buffer *sb);

void serial_buffer_reset(struct serial_buffer *sb);

/**
 * Puts the contents of the serial buffer onto the wire.
 */
void serial_buffer_tx(struct serial_buffer *sb);

/**
 * Appends the contents of buff to the buffer.
 */
size_t serial_buffer_append(struct serial_buffer *sb, const char *buff);

/**
 * Performs sprintf operation and appends result to the buffer.
 */
size_t serial_buffer_printf_append(struct serial_buffer *sb,
                                   const char *format_str, ...);

CPP_GUARD_END

#endif /* _SERIAL_BUFFER_H_ */
