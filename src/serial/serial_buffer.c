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
#include "string.h"
#include "taskUtil.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>

static void ctrl_char_strip(char *data)
{
        /* Strip all control characters from our messages */
        for(; *data >= 32; ++data);
        *data = 0;
}

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
        serial_buffer_clear(sb);

        if (NULL == sb->buffer)
                sb->buffer = (char*) portMalloc(size);

        if (NULL != sb->buffer)
                return true;

        /* Failure case if here */
        sb->length = 0;
        sb->buffer = NULL;
        return false;
}

int serial_buffer_rx(struct serial_buffer *sb,
                     const size_t ms_delay)
{
        const size_t available = sb->length - sb->curr_len;
        char *ptr = sb->buffer + sb->curr_len;
        size_t msg_len = 0;

        while (!msg_len) {
                const int read = serial_get_line_wait(
                        sb->serial, ptr, available, msToTicks(ms_delay));

                if (!read) {
                        pr_trace("[serial_buffer] msg timeout\r\n");
                        return 0;
                }

                ctrl_char_strip(ptr);
                msg_len = strlen(ptr);
        }

        pr_trace("[serial_buffer] Msg (offset = ");
        pr_trace_int(sb->curr_len);
        pr_trace("): \"");
        pr_trace(ptr);
        pr_trace("\"\r\n");

        sb->curr_len += msg_len + 1;
        return msg_len + 1;
}

size_t serial_buffer_rx_msgs(struct serial_buffer *sb, const size_t ms_delay,
                             const char *msgs[], size_t msgs_len)
{
        size_t rx_bytes = 0;
        size_t msgs_rx = 0;

        for (; msgs_len; --msgs_len, ++msgs_rx) {
                const size_t read = serial_buffer_rx(sb, ms_delay);

                if (0 == read)
                        break;

                *msgs++ = sb->buffer + rx_bytes;
                rx_bytes += read;
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
        serial_put_s(sb->serial, sb->buffer);
}

size_t serial_buffer_append(struct serial_buffer *sb, const char *buff)
{
        if (NULL == buff)
                return 0;

        if (0 < sb->curr_len) /* Account for \0 char from prev op */
                --(sb->curr_len);

        char *ptr = sb->buffer + sb->curr_len;
        const size_t max_len = sb->length - sb->curr_len;
        strncpy(ptr, buff, max_len);

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
