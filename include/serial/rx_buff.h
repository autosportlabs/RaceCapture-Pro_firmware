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

#ifndef _RX_BUFF_H_
#define _RX_BUFF_H_

#include "cpp_guard.h"

#include <stdbool.h>

CPP_GUARD_BEGIN

struct rx_buff {
        int chan_id;
        bool msg_ready;
        size_t idx;
        size_t cap;
        char *buff;
};

void rx_buff_clear(struct rx_buff *rxb);

bool rx_buff_init(struct rx_buff *rxb, const size_t cap, char *buff);

void rx_buff_free(struct rx_buff *rxb);

bool rx_buff_append(struct rx_buff *rxb, int chan_id, const char *data,
                    const size_t len);

const char* rx_buff_get_buff(struct rx_buff *rxb);

bool rx_buff_is_msg_ready(struct rx_buff *rxb);

int rx_buff_get_chan_id(struct rx_buff *rxb);

CPP_GUARD_END

#endif /* _RX_BUFF_H_ */
