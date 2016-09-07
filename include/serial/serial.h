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

#ifndef _SERIAL_H_
#define _SERIAL_H_

#include "FreeRTOS.h"
#include "cpp_guard.h"
#include "queue.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

CPP_GUARD_BEGIN

struct serial_cfg {
	size_t baud;
	size_t data_bits;
	size_t parity_bits;
	size_t stop_bits;
};

/**
 * The callback that gets fired when a user attempts to configure a
 * serial device.
 * @param cfg_cb_arg The argument as defined when you invoke serial_create.
 * @param bits # of bits per message (usually 8).
 * @param parity Parity of the data.  0 - None, 1 - Even, 2 - Odd.
 * @param stop_bits # of stop bits.
 * @param baud The baud rate of the serial device.
 */
typedef bool config_func_t(void *cfg_cb_arg, const size_t bits,
                           const size_t parity, const size_t stop_bits,
                           const size_t baud);

/**
 * The callback that gets fired when a user sends a character to the
 * serial buffer.  Typically used to set interrupt flags so that the
 * data can get sent out.
 * @param queue Handle for the queue with data to send.
 * @param post_tx_arg User provided argument as defined in serial_reate.
 */
typedef void post_tx_func_t(xQueueHandle queue, void *post_tx_arg);

enum serial_log_type {
        SERIAL_LOG_TYPE_NONE   = 0,
        SERIAL_LOG_TYPE_ASCII  = 1,
        SERIAL_LOG_TYPE_BINARY = 2,
};

struct Serial;

void serial_destroy(struct Serial *s);

void serial_close(struct Serial* s);

void serial_reopen(struct Serial* s);

bool serial_is_connected(const struct Serial* s);

struct Serial* serial_create(const char *name, const size_t tx_cap,
                             const size_t rx_cap, config_func_t *cfg_cb,
                             void *cfg_cb_arg, post_tx_func_t *post_tx_cb,
                             void *post_tx_cb_arg);

void serial_purge_rx_queue(struct Serial* s);

void serial_purge_tx_queue(struct Serial* s);

void serial_clear(struct Serial *s);

void serial_flush(struct Serial *s);

enum serial_log_type serial_logging(struct Serial *s,
                                    const enum serial_log_type type);

void serial_set_name(struct Serial *s, const char *name);

const char* serial_get_name(struct Serial *s);

void serial_init(struct Serial *s, unsigned int bits, unsigned int parity,
                 unsigned int stopBits, unsigned int baud);

bool serial_config(struct Serial *s, const size_t bits,
                   const size_t parity, const size_t stop_bits,
                   const size_t baud);

int serial_read_c_wait(struct Serial *s, char *c, const size_t delay);

int serial_read_c(struct Serial *s, char* c);

int serial_read_byte(struct Serial *serial, uint8_t *b, const size_t delay);

int serial_read_line(struct Serial *s, char *l, const size_t len);

int serial_read_line_wait(struct Serial *s, char *l, const size_t len,
			  const size_t delay);

int serial_write_c(struct Serial *s, const char c);

int serial_write_buff_wait(struct Serial *s, const char *buf,
			   const size_t len, const size_t delay);

int serial_write_buff(struct Serial *s, const char *buf, const size_t len);

int serial_write_s_wait(struct Serial *s, const char *l, const size_t delay);

int serial_write_s(struct Serial *s, const char *l);

xQueueHandle serial_get_rx_queue(struct Serial *s);

xQueueHandle serial_get_tx_queue(struct Serial *s);

enum serial_ioctl_status {
	SERIAL_IOCTL_STATUS_OK = 0,
	SERIAL_IOCTL_STATUS_ERR = -1,
	SERIAL_IOCTL_STATUS_UNSUPPORTED	= -2,
};


enum serial_ioctl {
        SERIAL_IOCTL_TELEMETRY = 1,
};

/**
 * The call back that gets fired when an ioctl is invoked on a Serial device.
 * @param req The request to be handled.
 * @param data Memory pointer that may be useful for the command.
 * @return Usually on success 0 is returned, but this is dependent on the
 * ioctl request.  The return value may be a parameter.  Values < 0 usually
 * indicate an error.
 */
typedef enum serial_ioctl_status serial_ioctl_cb_t(struct Serial *s,
						   unsigned long req, void* data);

void serial_set_ioctl_cb(struct Serial *s, serial_ioctl_cb_t* cb);

enum serial_ioctl_status serial_ioctl(struct Serial *s,
				      unsigned long req, void* argp);

const struct serial_cfg* serial_get_config(const struct Serial* s);

int put_int(struct Serial * serial, int n);

int put_ll(struct Serial *serial, long long l);

int put_hex(struct Serial * serial, int n);

int put_float(struct Serial * serial, float f, int precision);

int put_double(struct Serial * serial, double f, int precision);

int put_uint(struct Serial * serial, unsigned int n);

void put_escapedString(struct Serial * serial, const char *v, int length);

void put_nameUint(struct Serial * serial, const char *s, unsigned int n);

void put_nameSuffixUint(struct Serial * serial, const char *s, const char *suf,
                        unsigned int n);

void put_nameIndexUint(struct Serial * serial, const char *s, int i,
                       unsigned int n);

void put_nameInt(struct Serial * serial, const char *s, int n);

void put_nameSuffixInt(struct Serial * serial, const char *s, const char *suf,
                       int n);

void put_nameIndexInt(struct Serial * serial, const char *s, int i, int n);

void put_nameDouble(struct Serial * serial, const char *s, double n, int precision);

void put_nameSuffixDouble(struct Serial * serial, const char *s, const char *suf,
                          double n, int precision);

void put_nameIndexDouble(struct Serial * serial, const char *s, int i, double n,
                         int precision);

void put_nameFloat(struct Serial * serial, const char *s, float n, int precision);

void put_nameSuffixFloat(struct Serial * serial, const char *s, const char *suf,
                         float n, int precision);

void put_nameIndexFloat(struct Serial * serial, const char *s, int i, float n,
                        int precision);

void put_nameString(struct Serial * serial, const char *s, const char *v);

void put_nameSuffixString(struct Serial * serial, const char *s, const char *suf,
                          const char *v);

void put_nameIndexString(struct Serial * serial, const char *s, int i, const char *v);

void put_nameEscapedString(struct Serial * serial, const char *s, const char *v,
                           int length);

void put_bytes(struct Serial *serial, char *data, unsigned int length);

void put_crlf(struct Serial *serial);

void read_line(struct Serial *serial, char *buffer, size_t bufferSize);

void interactive_read_line(struct Serial *serial, char * buffer, size_t bufferSize);

CPP_GUARD_END

#endif /* _SERIAL_H_ */
