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

#include "capabilities.h"
#include "mod_string.h"
#include "modp_numtoa.h"
#include "printk.h"
#include "ring_buffer.h"
#include "serial.h"

#include <stddef.h>

#define IF_LEVEL_GT_CURR_LEVEL_RET_ZERO(l) if ((l) > curr_level) return 0

static enum log_level curr_level = INFO;
static char _log_buffer[LOG_BUFFER_SIZE];

static struct ring_buff log_buff = {
        .buf = _log_buffer,
        .size = LOG_BUFFER_SIZE,
        .head = _log_buffer,
        .tail = _log_buffer
};
static struct ring_buff * const lbp = &log_buff;

size_t read_log_to_serial(struct Serial *s, int escape)
{
        char buff[16];
        size_t to_read = get_used(lbp);

        while(has_data(lbp)) {
                int read = get_data(lbp, &buff, sizeof(buff));
                for(int i = 0; i < read; i++)
                        if (escape) {
                                put_escapedString(s, &buff[i],1);
                        } else {
                                serial_put_c(s, buff[i]);
                        }
        }

        return to_read;
}

size_t write_to_log_buff(const char *msg, const size_t size)
{
        if (0 == size)
                return 0;

        size_t data_written = put_data(lbp, msg, size);

        if (data_written == size)
                return data_written;

        // else if here we need to dump some log data.
        // XXX: Log this?
        int size_diff = size - data_written;
        dump_data(&log_buff, size_diff);
        data_written += put_data(lbp, msg + data_written, size_diff);

        return data_written;
}

int writek(const char *msg)
{
        if (NULL == msg)
                return 0;

        return write_to_log_buff(msg, strlen(msg));
}

int writek_crlf()
{
        return writek("\r\n");
}

int writek_char(char c)
{
        return write_to_log_buff(&c, 1);
}

int writek_int(int value)
{
        char buf[12];
        modp_itoa10(value, buf);
        return writek(buf);
}

int writek_float(float value)
{
        char buf[20];
        modp_ftoa(value, buf, 6);
        return writek(buf);
}

int printk(enum log_level level, const char *msg)
{
        IF_LEVEL_GT_CURR_LEVEL_RET_ZERO(level);
        return writek(msg);
}

int printk_char(enum log_level level, const char c)
{
        IF_LEVEL_GT_CURR_LEVEL_RET_ZERO(level);
        return writek_char(c);
}

int printk_crlf(enum log_level level)
{
        IF_LEVEL_GT_CURR_LEVEL_RET_ZERO(level);
        return writek_crlf();
}

int printk_int(enum log_level level, int value)
{
        IF_LEVEL_GT_CURR_LEVEL_RET_ZERO(level);
        return writek_int(value);
}

int printk_int_msg(enum log_level level, const char *msg, int value)
{
        IF_LEVEL_GT_CURR_LEVEL_RET_ZERO(level);
        return writek(msg) + writek_int(value) + writek_crlf();
}

int printk_float(enum log_level level, float value)
{
        IF_LEVEL_GT_CURR_LEVEL_RET_ZERO(level);
        return writek_float(value);
}

int printk_float_msg(enum log_level level, const char *msg, float value)
{
        IF_LEVEL_GT_CURR_LEVEL_RET_ZERO(level);
        return writek(msg) + writek_float(value) + writek_crlf();
}

int printk_str_msg(enum log_level level, const char *msg, const char *value)
{
        IF_LEVEL_GT_CURR_LEVEL_RET_ZERO(level);
        return writek(msg) + writek(value) + writek_crlf();
}

enum log_level get_log_level()
{
        return curr_level;
}

enum log_level set_log_level(enum log_level level)
{
        if (level <= TRACE)
                curr_level = level;

        return curr_level;
}
