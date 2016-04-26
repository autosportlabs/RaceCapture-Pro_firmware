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
#include "printk.h"
#include "rx_buff.h"
#include "serial.h"
#include "str_util.h"

#include <ctype.h>
#include <stdbool.h>
#include <string.h>

#define _LOG_PFX	"[rx_buff] "

/**
 * Clears the contents of an rx_buff struct.
 * @param rxb The rx_buff struct to adjust.
 */
void rx_buff_clear(struct rx_buff *rxb)
{
        rxb->idx = 0;
        rxb->buff[0] = 0;
}

/**
 * Allocates the data necessary for an rx_buff.  If no buff object is
 * provided, it will dynamically allocate one.
 * @param rxb The rx_buff struct to adjust.
 * @param cap The capacity of the buffer in bytes.
 * @param buff Pointer to a buffer to use.  If NULL, the function will
 * allocate a buffer for you.
 */
bool rx_buff_init(struct rx_buff *rxb, const size_t cap, char *buff)
{
        if (NULL == buff)
                buff = portMalloc(cap);

        if (NULL == buff) {
                /* Failed to get the memory we need */
                pr_error(_LOG_PFX "alloc failed\r\n");
                return false;
        }

        rxb->cap = cap;
        rxb->buff = buff;
        rx_buff_clear(rxb);
        return true;
}

/**
 * Frees the dynamically allocated buffer.
 * @param rxb The rx_buff struct to adjust.
 */
void rx_buff_free(struct rx_buff *rxb)
{
        portFree(rxb->buff);
        rxb->buff = NULL;
        rxb->cap = 0;
}

/**
 * Reads data from the Serial device into our buffer.
 * @param s The serial device to read data from.
 * @param ticks_to_wait The mount of time to wait before we timeout.
 * @return Pointer to a full message that is ready for processing if it was
 * received in time, NULL otherwise.
 */
char* rx_buff_read(struct rx_buff *rxb, struct Serial *s,
                  const size_t ticks_to_wait)
{

        xQueueHandle h = serial_get_rx_queue(s);

        for(; rxb->idx < rxb->cap; ++rxb->idx) {
                if (!xQueueReceive(h, rxb->buff + rxb->idx, ticks_to_wait)) {
                        /* If here, we timed out on receiving a message */
                        rx_buff_clear(rxb);
                        return NULL;
                }

                if ('\r' == rxb->buff[rxb->idx])
                        /* Then we got a message */
                        break;
        }

        /* If there is a \n after the \r, remove it */
        char c;
        if (xQueuePeek(h, &c, 0) && '\n' == c)
                xQueueReceive(h, &c, 0);

        if (rxb->idx >= rxb->cap) {
                /* Overflow scenario */
                pr_warning(_LOG_PFX "Overflow!");
                /* Cap the end so we don't do undefined things */
                rxb->buff[rxb->cap - 1] = 0;
        } else {
                /* Cap the end of the string for sanity */
                rxb->buff[rxb->idx] = 0;
        }

        /* Now strip any leading or trailing characters */
        /* STIEG: Make constant when process_read_msg is updated */
        return strip_inline(rxb->buff);
}
