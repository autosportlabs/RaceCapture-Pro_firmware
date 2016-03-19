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
#include "str_util.h"

#include <stdbool.h>
#include <string.h>

#define LOG_PFX	"[rx_buff] "

/**
 * Clears the contents of an rx_buff struct.
 * @param rxb The rx_buff struct to adjust.
 */
void rx_buff_clear(struct rx_buff *rxb)
{
        rxb->msg_ready = false;
        rxb->chan_id = -1;
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
                pr_error(LOG_PFX "alloc failed\r\n");
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
 * Appends received data to our buffer
 * @param rxb The receive buffer to append too.
 * @param data The data to append.
 * @param len The length of the data to append.
 * @return true if we have received a full message, false otherwise.
 */
bool rx_buff_append(struct rx_buff *rxb, int chan_id, const char *data,
                    const size_t len)
{
        if (rxb->chan_id > 0 && rxb->chan_id != chan_id) {
                pr_warning(LOG_PFX "Data from differnt channel appended!  "
                           "This will likely cause corruption.");
        }
        rxb->chan_id = chan_id;

        for (size_t i = 0; rxb->idx < rxb->cap && i < len;
             ++rxb->idx, ++data, ++i)
                rxb->buff[rxb->idx] = *data;

        if (rxb->idx >= rxb->cap) {
                /* Overflow */
                pr_warning(LOG_PFX "Overflow!");
                /* Cap the end so we don't do undefined things */
                rxb->buff[rxb->cap - 1] = 0;
                return rxb->msg_ready = true;
        }

        /* Cap the end of the string for sanity */
        rxb->buff[rxb->idx] = 0;

        /*
         * If here, then we know we hit end of data stream.  Just need to
         * check if we see our delimeters at the end of the message.
         */
        bool delim_seen = false;
        switch(rxb->buff[rxb->idx - 1]) {
        case '\n':
        case '\r':
                delim_seen = true;
        }

        return rxb->msg_ready = delim_seen;
}

/**
 * Gets our buffer as a const char*
 * @return A const char* buff pointer to the rx buffer.
 */
const char* rx_buff_get_buff(struct rx_buff *rxb)
{
        return rxb->buff;
}

/**
 * Use to determine if the buffer has a message that is ready for processing.
 * @return true if it does, false otherwise.
 */
bool rx_buff_is_msg_ready(struct rx_buff *rxb)
{
        return rxb->msg_ready;
}

/**
 * @return The channel ID that provided the data.
 */
int rx_buff_get_chan_id(struct rx_buff *rxb)
{
        return rxb->chan_id;
}

/**
 * Strips all whitespace characters from the end of the buffer inline.
 */
void rx_buff_rstrip(struct rx_buff *rxb)
{
        rstrip_inline(rxb->buff);
}
