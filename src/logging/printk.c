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
#include "macros.h"
#include <string.h>
#include "modp_numtoa.h"
#include "printk.h"
#include "ts_ring_buff.h"
#include "serial.h"

#include <stdbool.h>
#include <stddef.h>

#define IF_LEVEL_GT_CURR_LEVEL_RET_ZERO(l) if ((l) > curr_level) return 0

static enum log_level curr_level = INFO;
static struct ts_ring_buff *log_buff;

size_t read_log_to_serial(struct Serial *s, int escape)
{
        char buff[16];
        size_t read = 0;

        while(true) {
                size_t bytes = ts_ring_buff_get(log_buff, &buff,
                                                ARRAY_LEN(buff) - 1);
                if (0 == bytes)
                        break;

                buff[bytes] = 0;
                read += bytes;
                if (escape) {
                        put_escapedString(s, buff, bytes);
                } else {
                        serial_write_s(s, buff);
                }
        }

        return read;
}

int writek(const char *msg)
{
        if (!log_buff)
                log_buff = ts_ring_buff_create(LOG_BUFFER_SIZE);

        if (NULL == msg)
                return 0;

        return ts_ring_buff_put(log_buff, msg, strlen(msg));
}

int writek_crlf()
{
        return writek("\r\n");
}

int writek_char(char c)
{
        return ts_ring_buff_put(log_buff, &c, 1);
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

int printk_bool_msg(enum log_level level, const char *msg, const bool value)
{
        IF_LEVEL_GT_CURR_LEVEL_RET_ZERO(level);
        const char* bool_value = value ? "true" : "false";
        return writek(msg) + writek(bool_value) + writek_crlf();
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
