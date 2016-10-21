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
#include <stdio.h>
#include <string.h>

#define LOG_PFX	"[rx_buff] "

struct rx_buff {
        size_t cap;
        size_t idx;
        char *buff;
        bool msg_ready;
        bool echo;
};

/**
 * Clears the contents of an rx_buff struct.
 * @param rxb The rx_buff struct to adjust.
 */
void rx_buff_clear(struct rx_buff *rxb)
{
        rxb->idx = 0;
        rxb->buff[0] = 0;
        rxb->msg_ready = false;
}

/**
 * Allocates the data necessary for an rx_buff.  If no buff object is
 * provided, it will dynamically allocate one.
 * @param rxb The rx_buff struct to adjust.
 * @param cap The capacity of the buffer in bytes.
 * allocate a buffer for you.
 */
struct rx_buff* rx_buff_create(const size_t cap)
{
        struct rx_buff* rxb = (struct rx_buff*)
		calloc(1, sizeof(struct rx_buff));

        if (!rxb)
                goto fail_obj_alloc;

        rxb->buff = (char*) calloc(cap, sizeof(char));
        if (!rxb->buff)
                goto fail_buff_alloc;

        rxb->cap = cap;
        rx_buff_clear(rxb);
        return rxb;

fail_buff_alloc:
        free(rxb);
fail_obj_alloc:
        /* Failed to get the memory we need */
        pr_error(LOG_PFX "create failed\r\n");
        return NULL;
}

/**
 * Destroys the rx_buff object and its internal components.
 * @param rxb The rx_buff struct to adjust.
 */
void rx_buff_destroy(struct rx_buff *rxb)
{
        free(rxb->buff);
        free(rxb);
}

/**
 * Reads data from the Serial device into our buffer.
 * @param s The serial device to read data from.
 * @param echo Echo characters if not JSON?
 * @return true if we have received a full message that is ready to be
 * read, false otherwise.
 */
bool rx_buff_read(struct rx_buff *rxb, struct Serial *s, const bool echo)
{
        xQueueHandle h = serial_get_rx_queue(s);
	if (!h)
		return false;

        char c = INVALID_CHAR;
        while (rxb->idx < rxb->cap && !rxb->msg_ready) {
                const bool rx_status = xQueueReceive(h, &c, 0);
                if (!rx_status) {
                        /* If here, no more data to read for now */
                        return false;
                }

                /* Set echo based on first character */
                if (0 == rxb->idx)
                        rxb->echo = echo && '{' != c;

                switch(c) {
                case 0x08: /* Backspace */
                        /*
                         * On RaceCapture this acts like a delete command
                         * per the VT220 mapping.
                         */
                        if (rxb->idx) {
                                --rxb->idx;
                                if (rxb->echo) {
                                        serial_write_c(s, 0x08);
                                        serial_write_c(s, 0x7f);
                                }
                        }
                        break;
                case '\r':
                case '\0':
                        rxb->msg_ready = true;
                        /* Break intentionally missing here */
                default:
                        /* If we are echoing, do it */
                        if (rxb->echo)
                                serial_write_c(s, c);

                        rxb->buff[rxb->idx] = c;
                        ++rxb->idx;
                }
        }

        /*
         * Possible Overflow Scenario
         * If the message length is exactly cap - 1, then the term
         * character trigger overflow to happen if we weren't careful.
         * This is not an overflow so handle it accordingly.  But if the
         * last character was not a term character, then this is an
         * overflow for real.  Terminate the string so its stillusable
         * and set our state to indicate overflow. To save a bit of space,
         * we simply set the index to be cap + 1.
         */
        if (rxb->idx < rxb->cap ||
            (rxb->idx == rxb->cap && rxb->msg_ready)) {
                /* Turn the term character into the null */
                if (rxb->echo)
                        serial_write_c(s, c);

                rxb->buff[rxb->idx - 1] = 0;
        } else {
                pr_warning(LOG_PFX "Overflow!\r\n");
                /* Cap the end so we don't do undefined things */
                rxb->buff[rxb->cap - 1] = 0;
                /* Set our idx value to cap + 1 to indicate overflow */
                rxb->idx = rxb->cap + 1;
        }

        /* If there is a \n after the \r, remove it */
        if ('\r' == c && xQueuePeek(h, &c, 0) && '\n' == c) {
                xQueueReceive(h, &c, 0);
                if (rxb->echo)
                        serial_write_c(s, c);
        }

        return true;
}

char* rx_buff_get_msg(struct rx_buff *rxb)
{
        return rxb->msg_ready ? strip_inline(rxb->buff) : NULL;
}

bool rx_buff_is_overflow(struct rx_buff *rxb)
{
        return rxb->idx > rxb->cap;
}

/**
 * Gives us our status in enum form.
 */
enum rx_buff_status rx_buff_get_status(struct rx_buff *rxb)
{
	if (rxb->msg_ready)
		return RX_BUFF_STATUS_READY;

	if (rx_buff_is_overflow(rxb))
		return RX_BUFF_STATUS_OVERFLOW;

        return rxb->idx == 0 ?
                RX_BUFF_STATUS_EMPTY : RX_BUFF_STATUS_PARTIAL;
}
