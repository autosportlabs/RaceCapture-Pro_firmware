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


#include "mock_serial.h"
#include "serial.h"

#include <stddef.h>

#define BUFF_SIZE	(1024 * 16)

static struct Serial *s;
static char buff[BUFF_SIZE + 1];
static char *ptr = buff;

static void  _post_tx_cb(xQueueHandle q, void *arg)
{
        for(; ptr < buff + BUFF_SIZE && xQueueReceive(q, ptr, 0); ++ptr);
        *ptr = 0;
}

void setupMockSerial()
{
        if (!s)
                s = serial_create("Mock", 10, BUFF_SIZE,
                                  NULL, NULL, _post_tx_cb, NULL);

        serial_flush(s);
        mock_resetTxBuffer();
}

char* mock_getTxBuffer()
{
        return buff;
}

void mock_resetTxBuffer()
{
        ptr = buff;
        *ptr = 0;
}

void mock_appendRxBuffer(const char *src)
{
        xQueueHandle q = serial_get_rx_queue(s);
        for (; *src && xQueueSend(q, src, 0); ++src);
}

void mock_setRxBuffer(const char *src)
{
        serial_flush(s);
        mock_appendRxBuffer(src);
}

struct Serial* getMockSerial()
{
        return s;
}
