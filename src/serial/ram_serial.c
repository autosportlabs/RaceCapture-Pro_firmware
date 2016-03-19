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

#include "mem_mang.h"
#include "ram_serial.h"
#include "serial.h"

static void _flush() {}
static char _get_c() { return 0; }
static int _get_c_wait(char *c, size_t delay) { return 0; }
static int _get_line(char *s, int len) { return 0; }
static int _get_line_wait(char *s, int len, size_t delay) { return 0; }
static void _init(unsigned int bits, unsigned int parity,
                  unsigned int stopBits, unsigned int baud) {}
static void _put_c(char c);
static void _put_s(const char *s);

static struct {
        Serial serial;
        char *buff;
        size_t idx;
        size_t cap;
} state = {
        .serial = {
                .rx_callback = NULL,
                .tx_callback = NULL,
                .flush = _flush,
                .get_c = _get_c,
                .get_c_wait = _get_c_wait,
                .get_line = _get_line,
                .get_line_wait = _get_line_wait,
                .init = _init,
                .put_c = _put_c,
                .put_s = _put_s,
        },
};

static void _put_c(char c)
{
        if (state.idx < state.cap)
                state.buff[state.idx++] = c;
}

static void _put_s(const char *s)
{
        for(; *s; ++s)
                _put_c(*s);
}

/**
 * Clears the buffer that the serial writes to.
 */
void ram_serial_clear()
{
        state.idx = 0;
        state.buff[0] = 0;
}

/**
 * Initializes the ram_serial device by allocating the buffer size
 * based on the cap parameter
 * @param cap The capacity of the buffer to allocate.
 * @return true if successful, false otherwise.
 */
bool ram_serial_init(const size_t cap)
{
        if (!cap || state.buff)
                return false;

        state.buff = portMalloc(cap);
        if (!state.buff)
                return false;

        state.cap = cap;
        ram_serial_clear();

        return true;
}

/**
 * Destroys the buffer associated with the ram_serial device.
 * @return true if successful, false otherwise.
 */
bool ram_serial_destroy()
{
        if (NULL == state.buff)
                return false;

        portFree(state.buff);
        state.buff = NULL;

        return true;
}

/**
 * @return The serial device that is RAM backed.
 */
Serial* ram_serial_get_serial()
{
        return &state.serial;
}

/**
 * @return The number of characters in the buffer.
 */
size_t ram_serial_get_len()
{
        return state.idx;
}

/**
 * @return The total capacity of the buffer.
 */
size_t ram_serial_get_cap()
{
        return state.cap;
}

/**
 * @return A const char* to the buffer that represents the serial data.
 */
const char* ram_serial_get_buff()
{
        return state.buff;
}
