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
#include "sdcard.h"
#include "led.h"

#include <stdbool.h>
#include <stddef.h>

#define IF_LEVEL_GT_CURR_LEVEL_RET_ZERO(l) if ((l) > curr_level) return 0

static enum log_level curr_level = INFO;
static struct ts_ring_buff *log_buff;

#define SD_PR_FILE
#ifdef SD_PR_FILE
#define PR_FILENAME "pr_log.txt"
static FIL pr_file_data;
static FIL* pr_file = &pr_file_data;
static bool pr_file_open = false;
static bool pr_open_attempt = false;

FRESULT pr_init_sd_file()
{
	led_enable(LED_LOGGER);
        FRESULT ret = FR_OK;
        if ( ! pr_open_attempt && ! pr_file_open )
        {
		led_enable(LED_CAN);
                pr_open_attempt = true;
                if ( sdcard_fs_mounted() )
                {
			led_enable(LED_GPS);
                        pr_info( "mount sd" );
                        if( FR_OK == sd_open( pr_file, PR_FILENAME,
                                FA_READ | FA_WRITE | FA_CREATE_ALWAYS ) )
                        {
                                pr_info( "open sd" );
                                if ( FR_OK == sd_truncate( pr_file ) )
                                {
                                        pr_info( "truncate sd" );
                                        pr_file_open = true;
                                }
                                else
                                {
                                        pr_info( "close sd" );
                                        sd_close( pr_file );
                                }
                        }
                }
        }
	return ret;
}
#endif

size_t read_log_to_serial(struct Serial *s, int escape)
{
        char buff[16];
        size_t read = 0;

        pr_init_sd_file();
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
		if ( pr_file_open )
		{
			unsigned int written = 0;
			sd_write( pr_file, buff, bytes, &written );
		}
        }
	if ( pr_file_open )
	{
		led_enable(LED_LOGGER);
		sd_sync( pr_file );
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
	if ( value != value )
	{
		strcpy( buf, "nan" );
	}
	else
	{
		modp_ftoa(value, buf, 6);
	}
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
