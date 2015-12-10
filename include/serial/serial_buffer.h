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

#include "serial.h"

#include <stdbool.h>

struct serial_buffer {
        Serial *serial;
        size_t length;
        char *buffer;
};

bool serial_buffer_create(struct serial_buffer *sb,
                          Serial *serial,
                          const size_t size,
                          char *buffer);

int serial_buffer_read_wait(struct serial_buffer *sb,
                            const size_t ms_delay);

void serial_buffer_flush(struct serial_buffer *sb);

void serial_buffer_puts(struct serial_buffer *sb, const char *s);

#endif /* _SERIAL_BUFFER_H_ */
