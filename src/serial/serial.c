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
#include "loggerConfig.h"
#include "macros.h"
#include "mem_mang.h"
#include "modp_numtoa.h"
#include "printk.h"
#include "projdefs.h"
#include "serial.h"
#include "str_util.h"
#include "usart.h"
#include "usb_comm.h"

#include <stddef.h>
#include <stdarg.h>
#include <stdio.h>

struct Serial {
        const char *name;
        xQueueHandle tx_queue;
        xQueueHandle rx_queue;

        config_func_t *config_cb;
        void *config_cb_arg;
        post_tx_func_t *post_tx_cb;
        void *post_tx_cb_arg;

        bool log;
        bool log_tx_pfx;
        bool log_rx_pfx;
};


void serial_destroy(struct Serial *s)
{
        if (s->tx_queue)
                vQueueDelete(s->tx_queue);

        if (s->rx_queue)
                vQueueDelete(s->rx_queue);

        portFree(s);
}

struct Serial* serial_create(const char *name, const size_t tx_cap,
                             const size_t rx_cap, config_func_t *cfg_cb,
                             void *cfg_cb_arg, post_tx_func_t *post_tx_cb,
                             void *post_tx_cb_arg)
{
        struct Serial *s = portMalloc(sizeof(struct Serial));
        if (!s)
                return NULL;

        memset(s, 0, sizeof(struct Serial));

        s->name = name;
        s->config_cb = cfg_cb;
        s->config_cb_arg = cfg_cb_arg;
        s->post_tx_cb = post_tx_cb;
        s->post_tx_cb_arg = post_tx_cb_arg;

        const unsigned portBASE_TYPE c_size =
                (unsigned portBASE_TYPE) sizeof(signed portCHAR);
        s->tx_queue = xQueueCreate(tx_cap, c_size);
        s->rx_queue = xQueueCreate(rx_cap, c_size);

        /* If one of these is NULL, then alloc failure.  Handle */
        if (!s->tx_queue || !s->rx_queue) {
                serial_destroy(s);
                return NULL;
        }

        /* If here, all good.  Return the new object */
        return s;
}

static void _log(const struct Serial *s, const char *action,
                 bool *pfx, const char data)
{
        if (!s->log)
                return;

        if (*pfx) {
                if (s->name) {
                        pr_info("[serial (");
                        pr_info(s->name);
                        pr_info(")] ");
                } else {
                        pr_info("[serial] ");
                }
                pr_info(action);
                *pfx = false;
        }

        switch(data) {
        case('\r'):
                pr_info("(\\r)");
                break;
        case('\n'):
                pr_info("(\\n)\r\n");
                *pfx = true;
                break;
        default:
                pr_info_char(data);
                break;
        }
}

static void log_rx(struct Serial *s, const char data)
{
        _log(s, "RX: ", &s->log_rx_pfx, data);
}

static void log_tx(struct Serial *s, const char data)
{
        _log(s, "TX: ", &s->log_tx_pfx, data);
}

bool serial_logging(struct Serial *s, const bool enable)
{
        const bool prev = s->log;
        s->log = enable;
        return prev;
}

bool serial_config(const struct Serial *s, const size_t bits,
                   const size_t parity, const size_t stop_bits,
                   const size_t baud)
{
        if (!s->config_cb)
                return true; /* No-op always succeeds */

        return s->config_cb(s->config_cb_arg, bits, parity, stop_bits, baud);
}

static void purge_queue(xQueueHandle q)
{
        for(char c; pdTRUE == xQueueReceive(q, &c, 0););
}

void serial_purge_rx_queue(struct Serial* s)
{
        purge_queue(s->rx_queue);
}

void serial_purge_tx_queue(struct Serial* s)
{
        purge_queue(s->tx_queue);
}

/**
 * Clears the contents of the rx and tx queues.
 */
void serial_clear(struct Serial *s)
{
        serial_purge_rx_queue(s);
        serial_purge_tx_queue(s);
}

void serial_flush(struct Serial *s)
{
        /* Legacy Behavior.  Don't log the chars being dumped. */
        serial_purge_rx_queue(s);

        /* STIEG: TODO Figure out how to flush Tx sanely */
}

bool serial_get_c_wait(struct Serial *s, char *c, const size_t delay)
{
        if (pdFALSE == xQueueReceive(s->rx_queue, c, delay))
                return false;

        log_rx(s, *c);
        return true;
}

char serial_get_c(struct Serial *s)
{
        char c;
        return serial_get_c_wait(s, &c, portMAX_DELAY) ? c : 0;
}

/**
 * Reads in a line from a serial device delimeted by \n.  The data is
 * written to buff BUT MAY NOT BE NULL TERMINATED.  NULL termination is the
 * responsibility of the caller.
 * @param s The Serial device to read from.
 * @param buff The buffer to put the data into.
 * @param len The length of the buffer.
 * @param delay The number of ticks to wait.
 * @return Number of characters read.
 */
int serial_get_line_wait(struct Serial *s, char *buff, const size_t len,
                         const size_t delay)
{
        size_t i = 0;

        for (; i < len; ++i) {
                if (!serial_get_c_wait(s, buff + i, delay))
                        return i;

                switch(buff[i]) {
                case '\n':
                        return ++i;
                }
        }

        return i;
}

int serial_get_line(struct Serial *s, char *l, const size_t len)
{
        return serial_get_line_wait(s, l, len, portMAX_DELAY);
}

bool serial_put_c_wait(struct Serial *s, const char c, const size_t delay)
{
        if (pdFALSE == xQueueSend(s->tx_queue, &c, delay))
                return false;

        log_tx(s, c);

        if (s->post_tx_cb)
                s->post_tx_cb(s->tx_queue, s->post_tx_cb_arg);

        return true;
}

bool serial_put_c(struct Serial *s, const char c)
{
        return serial_put_c_wait(s, c, portMAX_DELAY);
}

int serial_put_buff_wait(struct Serial *s, const char *buf, const size_t len,
                         const size_t delay)
{
        int i;

        for (i = 0; i < len; ++i)
                serial_put_c_wait(s, buf[i], delay);

        return i;
}

int serial_put_buff(struct Serial *s, const char *buf, const size_t len)
{
        return serial_put_buff_wait(s, buf, len, portMAX_DELAY);
}

int serial_put_s_wait(struct Serial *s, const char *l, const size_t delay)
{
        int i;

        for (i = 0; *l; ++i, ++l)
                serial_put_c_wait(s, *l, delay);

        return i;
}

int serial_put_s(struct Serial *s, const char *l)
{
        return serial_put_s_wait(s, l, portMAX_DELAY);
}

bool serial_read_byte(struct Serial *serial, uint8_t *b, const size_t delay)
{
        return serial_get_c_wait(serial, (char*) b, delay);
}

xQueueHandle serial_get_rx_queue(struct Serial *s)
{
        return s->rx_queue;
}

xQueueHandle serial_get_tx_queue(struct Serial *s)
{
        return s->tx_queue;
}

/* STIEG: Replace vsnprintf with vfprintf if you can make a stream */
/* typedef ssize_t cookie_read_function_t(void *__cookie, char *__buf, size_t __n); */
/* static ssize_t _read(void *cookie, char *buf, size_t n) */
/* { */
/*         struct Serial *s = cookie; */
/*         return (ssize_t) serial_get_line(s, buf, n); */
/* } */

/* typedef ssize_t cookie_write_function_t(void *__cookie, const char *__buf, */
/*                                         size_t __n); */
/* static ssize_t _write(void *cookie, const char *buf, size_t n) */
/* { */
/*         struct Serial *s = cookie; */
/*         return (ssize_t) serial_put_buff(s, buf, n); */
/* } */

/* typedef struct */
/* { */
/*         /\* These four struct member names are dictated by Linux; hopefully, */
/*            they don't conflict with any macros.  *\/ */
/*         cookie_read_function_t  *read; */
/*         cookie_write_function_t *write; */
/*         cookie_seek_function_t  *seek; */
/*         cookie_close_function_t *close; */
/* } cookie_io_functions_t; */
/* static const cookie_io_functions_t serial_cookie = { */
/*         .read = _read, */
/*         .write = _write, */
/*         .seek = NULL, */
/*         .close = NULL, */
/* }; */

/* STIEG: Add "Serial_" prefix to these methods without them */
/* STIEG: Migrate to _put_numeric_val once figure out float printf issue */
/* static int _put_numeric_val(struct Serial *s, const char *fmt, ...) */
/* { */
/*         char buf[32]; */
/*         va_list ap; */

/*         va_start(ap, fmt); */
/*         vsnprintf(buf, ARRAY_LEN(buf), fmt, ap); */
/*         va_end(ap); */

/*         return serial_put_s(s, str_util_rstrip_zeros_inline(buf)); */
/* } */

int put_int(struct Serial *s, const int n)
{
        char buf[12];
        modp_itoa10(n, buf);
        return serial_put_s(s, buf);
}

int put_ll(struct Serial *s, const long long l)
{
        char buf[22];
        modp_ltoa10(l, buf);
        return serial_put_s(s, buf);
}

int put_hex(struct Serial *s, const int i)
{
        char buf[30];
        modp_itoaX(i, buf, 16);
        return serial_put_s(s, buf);
}

int put_uint(struct Serial *s, const unsigned int n)
{
        char buf[20];
        modp_uitoa10(n, buf);
        return serial_put_s(s, buf);
}

int put_float(struct Serial *s, const float f, const int precision)
{
        char buf[20];
        modp_ftoa(f, buf, precision);
        return serial_put_s(s, buf);
}

int put_double(struct Serial *s, const double f, const int precision)
{
        char buf[30];
        modp_dtoa(f, buf, precision);
        return serial_put_s(s, buf);
}

void put_nameIndexUint(struct Serial *serial, const char *s, int i, unsigned int n)
{
        serial_put_s(serial, s);
        serial_put_s(serial, "_");
        put_uint(serial, i);
        serial_put_s(serial, "=");
        put_uint(serial, n);
        serial_put_s(serial, ";");
}

void put_nameSuffixUint(struct Serial *serial, const char *s, const char *suf, unsigned int n)
{
        serial_put_s(serial, s);
        serial_put_s(serial, "_");
        serial_put_s(serial, suf);
        serial_put_s(serial, "=");
        put_uint(serial, n);
        serial_put_s(serial, ";");
}

void put_nameUint(struct Serial *serial, const char *s, unsigned int n)
{
        serial_put_s(serial, s);
        serial_put_s(serial, "=");
        put_uint(serial, n);
        serial_put_s(serial, ";");
}

void put_nameIndexInt(struct Serial *serial, const char *s, int i, int n)
{
        serial_put_s(serial, s);
        serial_put_s(serial, "_");
        put_uint(serial, i);
        serial_put_s(serial, "=");
        put_int(serial, n);
        serial_put_s(serial, ";");
}

void put_nameSuffixInt(struct Serial *serial, const char *s, const char *suf, int n)
{
        serial_put_s(serial, s);
        serial_put_s(serial, "_");
        serial_put_s(serial, suf);
        serial_put_s(serial, "=");
        put_int(serial, n);
        serial_put_s(serial, ";");
}

void put_nameInt(struct Serial *serial, const char *s, int n)
{
        serial_put_s(serial, s);
        serial_put_s(serial, "=");
        put_int(serial, n);
        serial_put_s(serial, ";");
}

void put_nameIndexDouble(struct Serial *serial, const char *s, int i, double n, int precision)
{
        serial_put_s(serial, s);
        serial_put_s(serial, "_");
        put_uint(serial, i);
        serial_put_s(serial, "=");
        put_double(serial, n,precision);
        serial_put_s(serial, ";");
}

void put_nameSuffixDouble(struct Serial *serial, const char *s, const char *suf, double n, int precision)
{
        serial_put_s(serial, s);
        serial_put_s(serial, "_");
        serial_put_s(serial, suf);
        serial_put_s(serial, "=");
        put_double(serial, n,precision);
        serial_put_s(serial, ";");
}

void put_nameDouble(struct Serial *serial, const char *s, double n, int precision)
{
        serial_put_s(serial, s);
        serial_put_s(serial, "=");
        put_double(serial, n, precision);
        serial_put_s(serial, ";");
}

void put_nameIndexFloat(struct Serial *serial, const char *s, int i, float n, int precision)
{
        serial_put_s(serial, s);
        serial_put_s(serial, "_");
        put_uint(serial, i);
        serial_put_s(serial, "=");
        put_float(serial, n, precision);
        serial_put_s(serial, ";");
}

void put_nameSuffixFloat(struct Serial *serial, const char *s, const char *suf, float n, int precision)
{
        serial_put_s(serial, s);
        serial_put_s(serial, "_");
        serial_put_s(serial, suf);
        serial_put_s(serial, "=");
        put_float(serial, n, precision);
        serial_put_s(serial, ";");
}

void put_nameFloat(struct Serial *serial, const char *s, float n, int precision)
{
        serial_put_s(serial, s);
        serial_put_s(serial, "=");
        put_float(serial, n, precision);
        serial_put_s(serial, ";");
}

void put_nameString(struct Serial *serial, const char *s, const char *v)
{
        serial_put_s(serial, s);
        serial_put_s(serial, "=\"");
        serial_put_s(serial, v);
        serial_put_s(serial, "\";");
}

void put_nameSuffixString(struct Serial *serial, const char *s, const char *suf, const char *v)
{
        serial_put_s(serial, s);
        serial_put_s(serial, "_");
        serial_put_s(serial, suf);
        serial_put_s(serial, "=\"");
        serial_put_s(serial, v);
        serial_put_s(serial, "\";");
}

void put_nameIndexString(struct Serial *serial, const char *s, int i, const char *v)
{
        serial_put_s(serial, s);
        serial_put_s(serial, "_");
        put_uint(serial, i);
        serial_put_s(serial, "=\"");
        serial_put_s(serial, v);
        serial_put_s(serial, "\";");
}

void put_escapedString(struct Serial * serial, const char *v, int length)
{
        const char *value = v;
        while (value - v < length) {
                switch(*value) {
                case '\n':
                        serial_put_s(serial, "\\n");
                        break;
                case '\r':
                        serial_put_s(serial, "\\r");
                        break;
                case '"':
                        serial_put_s(serial, "\\\"");
                        break;
                default:
                        serial_put_c(serial, *value);
                        break;
                }
                value++;
        }
}

void put_nameEscapedString(struct Serial *serial, const char *s, const char *v, int length)
{
        serial_put_s(serial, s);
        serial_put_s(serial, "=\"");
        const char *value = v;
        while (value - v < length) {
                switch(*value) {
                case ' ':
                        serial_put_s(serial, "\\_");
                        break;
                case '\n':
                        serial_put_s(serial, "\\n");
                        break;
                case '\r':
                        serial_put_s(serial, "\\r");
                        break;
                case '"':
                        serial_put_s(serial, "\\\"");
                        break;
                default:
                        serial_put_c(serial, *value);
                        break;
                }
                value++;
        }
        serial_put_s(serial, "\";");
}


void put_bytes(struct Serial *serial, char *data, unsigned int length)
{
        while (length > 0) {
                serial_put_c(serial, *data);
                data++;
                length--;
        }
}

void put_crlf(struct Serial *serial)
{
        serial_put_s(serial, "\r\n");
}

void read_line(struct Serial *serial, char *buffer, size_t bufferSize)
{
        size_t bufIndex = 0;
        char c;
        while(bufIndex < bufferSize - 1) {
                c = serial_get_c(serial);
                if (c) {
                        if ('\r' == c) {
                                break;
                        } else {
                                buffer[bufIndex++] = c;
                        }
                }
        }
        buffer[bufIndex]='\0';
}

void interactive_read_line(struct Serial *serial, char * buffer, size_t bufferSize)
{
        size_t bufIndex = 0;
        char c;
        while(bufIndex < bufferSize - 1) {
                c = serial_get_c(serial);
                if (c) {
                        if ('\r' == c) {
                                break;
                        } else if ('\b' == c) {
                                if (bufIndex > 0) {
                                        bufIndex--;
                                        serial_put_c(serial, c);
                                }
                        } else {
                                serial_put_c(serial, c);
                                buffer[bufIndex++] = c;
                        }
                }
        }
        serial_put_s(serial, "\r\n");
        buffer[bufIndex]='\0';
}
