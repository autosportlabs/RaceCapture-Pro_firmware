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
#include "panic.h"
#include "printk.h"
#include "projdefs.h"
#include "serial.h"
#include "str_util.h"
#include "queue.h"
#include "usart.h"
#include "usb_comm.h"
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

static const char invalid_char = INVALID_CHAR;

enum data_dir {
        DATA_DIR_RX,
        DATA_DIR_TX,
};

struct Serial {
	const char *name;
	xQueueHandle tx_queue;
	xQueueHandle rx_queue;
	bool closed;

	config_func_t *config_cb;
	void *config_cb_arg;
	post_tx_func_t *post_tx_cb;
	void *post_tx_cb_arg;
	serial_ioctl_cb_t *ioctl_cb;

	enum serial_log_type log_type;
	size_t log_rx_cntr;
	size_t log_tx_cntr;

	struct serial_cfg cfg;
};

void serial_purge_rx_queue(struct Serial* s)
{
	xQueueReset(s->rx_queue);
}

void serial_purge_tx_queue(struct Serial* s)
{
	xQueueReset(s->tx_queue);
}

/**
 * Clears the contents of the rx and tx queues.
 */
void serial_clear(struct Serial *s)
{
        serial_purge_rx_queue(s);
        serial_purge_tx_queue(s);
}

/**
 * Unblocks our Seral rx queue by putting an invalid character on the
 * front of the queue. This will unblock any task that is waiting on
 * the queue.  Its up to the rx handler below to ensure that we return
 * the correct status code.
 */
static void unblock_rx_queue(struct Serial *s)
{
	xQueueSendToFront(s->rx_queue, &invalid_char, 0);
}

/**
 * Closes the Serial device and releases the associated buffer resources
 * to ensure that we use as few resources as necessary.
 */
void serial_close(struct Serial* s)
{
	s->closed = true;
	serial_clear(s);
	unblock_rx_queue(s);
}

/**
 * Re-opens a closed serial object.  This exists because there are inherent
 * race conditions when using serial objects directly and closing/destroying
 * them.  We need an intermediary step like what glibc does with file handles
 * and streams.  That is tracked by issue #542.
 */
void serial_reopen(struct Serial* s)
{
	serial_clear(s);
	s->closed = false;
}


void serial_destroy(struct Serial *s)
{
	vQueueDelete(s->tx_queue);
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

static void log_header_if_necessary(struct Serial *s,
                                    const enum data_dir dir)
{
        size_t count = 0;
        switch (dir) {
        case DATA_DIR_RX:
                count = s->log_rx_cntr;
                break;
        case DATA_DIR_TX:
                count = s->log_tx_cntr;
                break;
        }

        /* If not at 0'th position, do not print header */
        if (count)
                return;

        pr_info("\r\n");
        if (s->name) {
                pr_info("[serial (");
                pr_info(s->name);
                pr_info(")] ");
        } else {
                pr_info("[serial] ");
        }

        switch (dir) {
        case DATA_DIR_RX:
                pr_info("RX: ");
                s->log_tx_cntr = 0;
                break;
        case DATA_DIR_TX:
                pr_info("TX: ");
                s->log_rx_cntr = 0;
                break;
        }
}

static void log_ascii(size_t* cntr, const char data)
{
        switch(data) {
        case('\r'):
                pr_info("(\\r)");
                break;
        case('\n'):
                pr_info("(\\n)");
                *cntr = 0;
                break;
        default:
                pr_info_char(data);
                break;
        }
}

static void log_binary(size_t* cntr, const char data)
{
        char buf[12];
        /* Only print printable characters in data column */
        if (32 <= data && data <= 126) {
                sprintf(buf, "0x%02x(%c)", data, data);
        } else {
                sprintf(buf, "0x%02x(.)", data);
        }
        pr_info(buf);

        if (*cntr < 16) {
                pr_info_char(',');
        } else {
                *cntr = 0;
        }
}

static void _log(struct Serial *s,
                 const enum data_dir dir,
                 const char data)
{
        if (SERIAL_LOG_TYPE_NONE == s->log_type)
                return;

        log_header_if_necessary(s, dir);
        size_t* cntr = NULL;
        switch (dir) {
        case DATA_DIR_RX:
                cntr = &s->log_rx_cntr;
                break;
        case DATA_DIR_TX:
                cntr = &s->log_tx_cntr;
                break;
        }
        *cntr += 1;

        switch(s->log_type) {
        case SERIAL_LOG_TYPE_ASCII:
                log_ascii(cntr, data);
                break;
        case SERIAL_LOG_TYPE_BINARY:
                log_binary(cntr, data);
                break;
        default:
                break;
        }
}

static void log_rx(struct Serial *s, const char data)
{
        _log(s, DATA_DIR_RX, data);
}

static void log_tx(struct Serial *s, const char data)
{
        _log(s, DATA_DIR_TX, data);
}

enum serial_log_type serial_logging(struct Serial *s,
                                    const enum serial_log_type type)
{
        const enum serial_log_type prev = s->log_type;
        s->log_type = type;
        return prev;
}

bool serial_config(struct Serial *s, const size_t bits,
		   const size_t parity, const size_t stop_bits,
		   const size_t baud)
{
	s->cfg.data_bits = bits;
	s->cfg.parity_bits = parity;
	s->cfg.stop_bits = stop_bits;
	s->cfg.baud = baud;

	if (!s->config_cb)
		return true; /* No-op always succeeds */

	return s->config_cb(s->config_cb_arg, bits, parity, stop_bits, baud);
}

bool serial_is_connected(const struct Serial* s)
{
	return s && !s->closed;
}

/**
 * @return The serial config as set by the #serial_config method.
 */
const struct serial_cfg* serial_get_config(const struct Serial* s)
{
	return &s->cfg;
}

void serial_flush(struct Serial *s)
{
        /* Legacy Behavior.  Don't log the chars being dumped. */
        serial_purge_rx_queue(s);

        /* STIEG: TODO Figure out how to flush Tx sanely */
}

int serial_read_c_wait(struct Serial *s, char *c, const size_t delay)
{
	if (s->closed)
		return -1;

        if (pdFALSE == xQueueReceive(s->rx_queue, c, delay))
                return 0;

	/* Check & handle closure of serial device here */
	if (*c == invalid_char && s->closed) {
		/* Unblock the queue for other waiting tasks */
		unblock_rx_queue(s);
		return -1;
	}

        log_rx(s, *c);
        return 1;
}

int serial_read_c(struct Serial *s, char* c)
{
        return serial_read_c_wait(s, c, portMAX_DELAY);
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
int serial_read_line_wait(struct Serial *s, char *buff, const size_t len,
                         const size_t delay)
{
        int i = 0;
        for (; i < len; ++i) {
                switch(serial_read_c_wait(s, buff + i, delay)) {
		default:
			panic(PANIC_CAUSE_UNREACHABLE);
			break;
		case -1:
			/* If partially read, return what was read. */
			return i == 0 ? -1 : i;
		case 0:
			return i;
		case 1:
			if ('\n' == buff[i])
				return ++i;
		}
	}

        return i;
}

int serial_read_line(struct Serial *s, char *l, const size_t len)
{
        return serial_read_line_wait(s, l, len, portMAX_DELAY);
}

int serial_write_c_wait(struct Serial *s, const char c, const size_t delay)
{
	if (s->closed)
		return -1;

        if (pdFALSE == xQueueSend(s->tx_queue, &c, delay))
                return 0;

	/* Handle case where closing queue unblocks xQueueSend */
	if (s->closed)
		return -1;

        log_tx(s, c);

        if (s->post_tx_cb)
                s->post_tx_cb(s->tx_queue, s->post_tx_cb_arg);

        return 1;
}

int serial_write_c(struct Serial *s, const char c)
{
        return serial_write_c_wait(s, c, portMAX_DELAY);
}

int serial_write_buff_wait(struct Serial *s, const char *buf, const size_t len,
                         const size_t delay)
{
        int i = 0;
        for (; i < len; ++i) {
                switch(serial_write_c_wait(s, buf[i], delay)) {
		default:
			panic(PANIC_CAUSE_UNREACHABLE);
			break;
		case -1:
			/* If partially sent, then return what was sent. */
			return i == 0 ? -1 : i;
		case 0:
			return i;
		case 1:
			break;
		}
	}

        return i;
}

int serial_write_buff(struct Serial *s, const char *buf, const size_t len)
{
        return serial_write_buff_wait(s, buf, len, portMAX_DELAY);
}

int serial_write_s_wait(struct Serial *s, const char *l, const size_t delay)
{
	return serial_write_buff_wait(s, l, strlen(l), delay);
}

int serial_write_s(struct Serial *s, const char *l)
{
        return serial_write_s_wait(s, l, portMAX_DELAY);
}

int serial_read_byte(struct Serial *serial, uint8_t *b, const size_t delay)
{
        return serial_read_c_wait(serial, (char*) b, delay);
}

xQueueHandle serial_get_rx_queue(struct Serial *s)
{
        return s->rx_queue;
}

xQueueHandle serial_get_tx_queue(struct Serial *s)
{
        return s->tx_queue;
}

void serial_set_name(struct Serial *s, const char *name)
{
        s->name = name;
}

const char* serial_get_name(struct Serial *s)
{
        return s->name;
}


/**
 * Used to set the callback handler for Serial based IOCTL devices. This
 * should only be used by the task that is responsible for managing a
 * serial device. In effect this allows the task to handle Serial specific
 * requests.  This interface intentionally mimics socket ioctl calls so that
 * we can more easily replace Serial with Sockets at a later date.
 * @param s The serial device to alter.
 * @param cb The callback address.
 * @note Ideally this would be better placed in the _init method for seial.
 * But that is not pratical as of this writing because not all Serial
 * devices are created in the tasks that will actually manage them.
 * We should fix that at a later date.
 */
void serial_set_ioctl_cb(struct Serial *s, serial_ioctl_cb_t* cb)
{
        s->ioctl_cb = cb;
}

/**
 * Invoke an ioctl request on a Serial device.
 * @param s The Serial device.
 * @param req The request to enact.
 * @param argp Memory address that may be useful for these commands.
 */
enum serial_ioctl_status serial_ioctl(struct Serial *s,
				      unsigned long req, void* argp)
{
	if(!s->ioctl_cb)
		return SERIAL_IOCTL_STATUS_UNSUPPORTED;

	return s->ioctl_cb(s, req, argp);
}

/* STIEG: Replace vsnprintf with vfprintf if you can make a stream */
/* typedef ssize_t cookie_read_function_t(void *__cookie, char *__buf, size_t __n); */
/* static ssize_t _read(void *cookie, char *buf, size_t n) */
/* { */
/*         struct Serial *s = cookie; */
/*         return (ssize_t) serial_read_line(s, buf, n); */
/* } */

/* typedef ssize_t cookie_write_function_t(void *__cookie, const char *__buf, */
/*                                         size_t __n); */
/* static ssize_t _write(void *cookie, const char *buf, size_t n) */
/* { */
/*         struct Serial *s = cookie; */
/*         return (ssize_t) serial_write_buff(s, buf, n); */
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
/* STIEG: Migrate to _write_numeric_val once figure out float printf issue */
/* static int _write_numeric_val(struct Serial *s, const char *fmt, ...) */
/* { */
/*         char buf[32]; */
/*         va_list ap; */

/*         va_start(ap, fmt); */
/*         vsnprintf(buf, ARRAY_LEN(buf), fmt, ap); */
/*         va_end(ap); */

/*         return serial_write_s(s, str_util_rstrip_zeros_inline(buf)); */
/* } */

int put_int(struct Serial *s, const int n)
{
        char buf[12];
        modp_itoa10(n, buf);
        return serial_write_s(s, buf);
}

int put_ll(struct Serial *s, const long long l)
{
        char buf[22];
        modp_ltoa10(l, buf);
        return serial_write_s(s, buf);
}

int put_hex(struct Serial *s, const int i)
{
        char buf[30];
        modp_itoaX(i, buf, 16);
        return serial_write_s(s, buf);
}

int put_uint(struct Serial *s, const unsigned int n)
{
        char buf[20];
        modp_uitoa10(n, buf);
        return serial_write_s(s, buf);
}

int put_float(struct Serial *s, const float f, const int precision)
{
        char buf[20];
        modp_ftoa(f, buf, precision);
        return serial_write_s(s, buf);
}

int put_double(struct Serial *s, const double f, const int precision)
{
        char buf[30];
        modp_dtoa(f, buf, precision);
        return serial_write_s(s, buf);
}

void put_nameIndexUint(struct Serial *serial, const char *s, int i, unsigned int n)
{
        serial_write_s(serial, s);
        serial_write_s(serial, "_");
        put_uint(serial, i);
        serial_write_s(serial, "=");
        put_uint(serial, n);
        serial_write_s(serial, ";");
}

void put_nameSuffixUint(struct Serial *serial, const char *s, const char *suf, unsigned int n)
{
        serial_write_s(serial, s);
        serial_write_s(serial, "_");
        serial_write_s(serial, suf);
        serial_write_s(serial, "=");
        put_uint(serial, n);
        serial_write_s(serial, ";");
}

void put_nameUint(struct Serial *serial, const char *s, unsigned int n)
{
        serial_write_s(serial, s);
        serial_write_s(serial, "=");
        put_uint(serial, n);
        serial_write_s(serial, ";");
}

void put_nameIndexInt(struct Serial *serial, const char *s, int i, int n)
{
        serial_write_s(serial, s);
        serial_write_s(serial, "_");
        put_uint(serial, i);
        serial_write_s(serial, "=");
        put_int(serial, n);
        serial_write_s(serial, ";");
}

void put_nameSuffixInt(struct Serial *serial, const char *s, const char *suf, int n)
{
        serial_write_s(serial, s);
        serial_write_s(serial, "_");
        serial_write_s(serial, suf);
        serial_write_s(serial, "=");
        put_int(serial, n);
        serial_write_s(serial, ";");
}

void put_nameInt(struct Serial *serial, const char *s, int n)
{
        serial_write_s(serial, s);
        serial_write_s(serial, "=");
        put_int(serial, n);
        serial_write_s(serial, ";");
}

void put_nameIndexDouble(struct Serial *serial, const char *s, int i, double n, int precision)
{
        serial_write_s(serial, s);
        serial_write_s(serial, "_");
        put_uint(serial, i);
        serial_write_s(serial, "=");
        put_double(serial, n,precision);
        serial_write_s(serial, ";");
}

void put_nameSuffixDouble(struct Serial *serial, const char *s, const char *suf, double n, int precision)
{
        serial_write_s(serial, s);
        serial_write_s(serial, "_");
        serial_write_s(serial, suf);
        serial_write_s(serial, "=");
        put_double(serial, n,precision);
        serial_write_s(serial, ";");
}

void put_nameDouble(struct Serial *serial, const char *s, double n, int precision)
{
        serial_write_s(serial, s);
        serial_write_s(serial, "=");
        put_double(serial, n, precision);
        serial_write_s(serial, ";");
}

void put_nameIndexFloat(struct Serial *serial, const char *s, int i, float n, int precision)
{
        serial_write_s(serial, s);
        serial_write_s(serial, "_");
        put_uint(serial, i);
        serial_write_s(serial, "=");
        put_float(serial, n, precision);
        serial_write_s(serial, ";");
}

void put_nameSuffixFloat(struct Serial *serial, const char *s, const char *suf, float n, int precision)
{
        serial_write_s(serial, s);
        serial_write_s(serial, "_");
        serial_write_s(serial, suf);
        serial_write_s(serial, "=");
        put_float(serial, n, precision);
        serial_write_s(serial, ";");
}

void put_nameFloat(struct Serial *serial, const char *s, float n, int precision)
{
        serial_write_s(serial, s);
        serial_write_s(serial, "=");
        put_float(serial, n, precision);
        serial_write_s(serial, ";");
}

void put_nameString(struct Serial *serial, const char *s, const char *v)
{
        serial_write_s(serial, s);
        serial_write_s(serial, "=\"");
        serial_write_s(serial, v);
        serial_write_s(serial, "\";");
}

void put_nameSuffixString(struct Serial *serial, const char *s, const char *suf, const char *v)
{
        serial_write_s(serial, s);
        serial_write_s(serial, "_");
        serial_write_s(serial, suf);
        serial_write_s(serial, "=\"");
        serial_write_s(serial, v);
        serial_write_s(serial, "\";");
}

void put_nameIndexString(struct Serial *serial, const char *s, int i, const char *v)
{
        serial_write_s(serial, s);
        serial_write_s(serial, "_");
        put_uint(serial, i);
        serial_write_s(serial, "=\"");
        serial_write_s(serial, v);
        serial_write_s(serial, "\";");
}

void put_escapedString(struct Serial * serial, const char *v, int length)
{
        const char *value = v;
        while (value - v < length) {
                switch(*value) {
                case '\n':
                        serial_write_s(serial, "\\n");
                        break;
                case '\r':
                        serial_write_s(serial, "\\r");
                        break;
                case '"':
                        serial_write_s(serial, "\\\"");
                        break;
                default:
                        serial_write_c(serial, *value);
                        break;
                }
                value++;
        }
}

void put_nameEscapedString(struct Serial *serial, const char *s, const char *v, int length)
{
        serial_write_s(serial, s);
        serial_write_s(serial, "=\"");
        const char *value = v;
        while (value - v < length) {
                switch(*value) {
                case ' ':
                        serial_write_s(serial, "\\_");
                        break;
                case '\n':
                        serial_write_s(serial, "\\n");
                        break;
                case '\r':
                        serial_write_s(serial, "\\r");
                        break;
                case '"':
                        serial_write_s(serial, "\\\"");
                        break;
                default:
                        serial_write_c(serial, *value);
                        break;
                }
                value++;
        }
        serial_write_s(serial, "\";");
}


void put_bytes(struct Serial *serial, char *data, unsigned int length)
{
        while (length > 0) {
                serial_write_c(serial, *data);
                data++;
                length--;
        }
}

void put_crlf(struct Serial *serial)
{
        serial_write_s(serial, "\r\n");
}

void read_line(struct Serial *serial, char *buffer, size_t bufferSize)
{
        size_t bufIndex = 0;
        while(bufIndex < bufferSize - 1) {
		char c;
                if (1 == serial_read_c(serial, &c)) {
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
        while(bufIndex < bufferSize - 1) {
		char c;
                if (1 == serial_read_c(serial, &c)) {
                        if ('\r' == c) {
                                break;
                        } else if ('\b' == c) {
                                if (bufIndex > 0) {
                                        bufIndex--;
                                        serial_write_c(serial, c);
                                }
                        } else {
                                serial_write_c(serial, c);
                                buffer[bufIndex++] = c;
                        }
                }
        }
        serial_write_s(serial, "\r\n");
        buffer[bufIndex]='\0';
}
