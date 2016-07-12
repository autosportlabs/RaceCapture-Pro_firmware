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

#include "at_basic.h"
#include "macros.h"
#include "serial.h"
#include "str_util.h"
#include <stdbool.h>
#include <stddef.h>

#define MAX_AT_REPLY_READS	10
#define PROBE_PING_ATTEMPTS	3
#define PROBE_PING_DELAY_MS	100

/**
 * Sends a basic Ping command to the device and waits for a reply.
 * @param serial The serial device.
 * @param tries The number of timest to ping before giving up.
 * @param delay_ms The time to wait for each ping in ms before giving up.
 */
bool at_basic_ping(struct Serial* serial, const size_t tries,
		   const size_t delay_ms)
{
	char buff[16];
	const char cmd[] = "AT\r\n";
	const size_t len = ARRAY_LEN(buff) - 1;

	for (size_t try = 0; try < tries; ++try) {
		serial_flush(serial);
		serial_put_s(serial, cmd);

		for (size_t reads = MAX_AT_REPLY_READS;
		     reads && serial_get_line_wait(serial, buff, len, delay_ms);
		     --reads) {
			const char* msg = strip_inline(buff);
			if (STR_EQ("OK", msg))
				return true;
		}
	}

	return false;
}

/**
 * Probes a Serial port for a device that will respond to the basic
 * AT command.
 * @param Serial The serial device
 * @param bauds A NULL terminated list of baud rates.
 * @param tries The number of tries per baud rate before giving up.
 * @param delay_ms The time we wait in ms for a response before giving up.
 * @param msg_bits Serial configuration of # of bits per message
 * @param parity 0 - Even parity, 1 - odd parity.
 * @param stop_bits Number of stop bits per message.
 * @return The baud rate that the device responded to. 0 if no response.
 */
int at_basic_probe(struct Serial* serial, const int bauds[],
		   const size_t tries, const size_t delay_ms,
		   const size_t msg_bits, const size_t parity,
		   const size_t stop_bits)
{
	for (const int* baud = bauds; bauds && *baud; ++baud) {
		serial_config(serial, msg_bits, parity, stop_bits, *baud);
		if (at_basic_ping(serial, tries, delay_ms))
			return *baud;
	}

	return 0;
}
