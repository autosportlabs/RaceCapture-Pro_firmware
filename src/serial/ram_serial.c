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

#include "FreeRTOS.h"
#include "mem_mang.h"
#include "ram_serial.h"
#include "serial.h"
#include "queue.h"

static struct {
        struct Serial *serial;
        char *buff;
        size_t idx;
        size_t cap;
} state;

/**
 * Clears the buffer that the serial writes to.
 */
void ram_serial_clear()
{
        state.idx = 0;
        state.buff[0] = 0;
}

static void _post_tx(xQueueHandle q, void *post_tx_arg)
{
        char c;
        while (state.idx < state.cap && xQueueReceive(q, &c, 0))
                state.buff[state.idx++] = c;
}

/**
 * Initializes the ram_serial device by allocating the buffer size
 * based on the cap parameter
 * @param cap The capacity of the buffer to allocate.
 * @return true if successful, false otherwise.
 */
bool ram_serial_init(const size_t cap)
{
        if (!cap || state.buff || state.serial)
                return false;

        state.buff = portMalloc(cap);
        if (!state.buff)
                return false;

        state.cap = cap;
        state.serial = serial_create("RamSerial", 1, 1, NULL, NULL,
                                     _post_tx, NULL);
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

        serial_destroy(state.serial);
        state.serial = NULL;

        return true;
}

/**
 * @return The serial device that is RAM backed.
 */
struct Serial* ram_serial_get_serial()
{
        return state.serial;
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
