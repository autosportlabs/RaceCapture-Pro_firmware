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
#include "stdutil.h"
#include "str_util.h"
#include "taskUtil.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define LOG_PFX	"[serial buffer]"

bool serial_buffer_create(struct serial_buffer *sb,
                          struct Serial *serial,
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

        serial_buffer_clear(sb);
        if (NULL != sb->buffer)
                return true;

        /* Failure case if here */
        sb->length = 0;
        sb->buffer = NULL;
        return false;
}

char* serial_buffer_rx(struct serial_buffer *sb,
                       const size_t ms_delay)
{
        const size_t available = sb->length - sb->curr_len;
	if (!available)
		return NULL;

        char *ptr = sb->buffer + sb->curr_len;
        size_t msg_len = 0;
        int read;

        while (!msg_len) {
                read  = serial_read_line_wait(sb->serial, ptr, available - 1,
					      msToTicks(ms_delay));

                if (read < 1)
                        return NULL;

                msg_len = serial_msg_strlen(ptr);
        }

	/* Check for overflow and warn */
	if (read == available)
		pr_warning(LOG_PFX "Serial buffer overflow!\r\n");

	/* If here, got a non-empty msg.  Terminate it and add the length */
        ptr[read] = 0;
        sb->curr_len += ++read;
        return ptr;
}

size_t serial_buffer_rx_msgs(struct serial_buffer *sb, const size_t ms_delay,
                             const char *msgs[], size_t msgs_len)
{
        size_t msgs_rx = 0;

        for (; msgs_len; --msgs_len, ++msgs_rx) {
                const char* msg = serial_buffer_rx(sb, ms_delay);

                if (!msg)
                        break;

                *msgs++ = msg;
        }

        return msgs_rx;
}

void serial_buffer_flush(struct serial_buffer *sb)
{
        serial_flush(sb->serial);
}

void serial_buffer_clear(struct serial_buffer *sb)
{
        sb->buffer[0] = '\0';
        sb->curr_len = 0;
}

void serial_buffer_reset(struct serial_buffer *sb)
{
        serial_buffer_flush(sb);
        serial_buffer_clear(sb);
}

void serial_buffer_tx(struct serial_buffer *sb)
{
        serial_write_s(sb->serial, sb->buffer);
}

size_t serial_buffer_append(struct serial_buffer *sb, const char *buff)
{
        if (NULL == buff)
                return 0;

        if (0 < sb->curr_len) /* Account for \0 char from prev op */
                --(sb->curr_len);

        char *ptr = sb->buffer + sb->curr_len;
        const size_t max_len = sb->length - sb->curr_len;
        strntcpy(ptr, buff, max_len);

        const size_t buff_len = strlen(buff) + 1;
        const size_t cpy_len = MIN(buff_len, max_len);
        sb->curr_len += cpy_len;

        return buff_len;
}

size_t serial_buffer_printf_append(struct serial_buffer *sb,
                                   const char *format_str, ...)
{
        va_list a_list;
        va_start(a_list, format_str);

        if (0 < sb->curr_len) /* Account for \0 char from prev op */
                --(sb->curr_len);

        char *ptr = sb->buffer + sb->curr_len;
        const size_t max_len = sb->length - sb->curr_len;
        const size_t chars = vsnprintf(ptr, max_len, format_str, a_list);

        va_end(a_list);

        sb->curr_len += chars + 1;
        return chars + 1;
}
