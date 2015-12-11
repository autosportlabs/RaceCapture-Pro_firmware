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

#include "mem_mang.h"
#include "printk.h"
#include "serial.h"
#include "serial_buffer.h"
#include "taskUtil.h"

#include <stdbool.h>

bool serial_buffer_create(struct serial_buffer *sb,
                          Serial *serial,
                          const size_t size,
                          char *buffer)
{
        if (NULL == sb || NULL == serial)
                return false;

        sb->serial = serial;
        sb->length = size;
        sb->buffer = buffer;

        if (NULL == sb->buffer)
                sb->buffer = (char*) portMalloc(size);

        if (NULL != sb->buffer)
                return true;

        /* Failure case if here */
        sb->length = 0;
        sb->buffer = NULL;
        return false;
}

int serial_buffer_read_wait(struct serial_buffer *sb,
                            const size_t ms_delay)
{
        return serial_get_line_wait(sb->serial,sb->buffer, sb->length,
                                    msToTicks(ms_delay));
}

void serial_buffer_flush(struct serial_buffer *sb)
{
        sb->buffer[0] = '\0';
        serial_flush(sb->serial);
}

void serial_buffer_puts(struct serial_buffer *sb, const char *s)
{
        serial_put_s(sb->serial, s);
}
