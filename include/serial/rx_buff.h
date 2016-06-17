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

struct rx_buff;

enum rx_buff_status {
        RX_BUFF_STATUS_EMPTY,
        RX_BUFF_STATUS_PARTIAL,
        RX_BUFF_STATUS_READY,
        RX_BUFF_STATUS_OVERFLOW,
};

void rx_buff_clear(struct rx_buff *rxb);

struct rx_buff* rx_buff_create(const size_t cap);

void rx_buff_destroy(struct rx_buff *rxb);

bool rx_buff_read(struct rx_buff *rxb, struct Serial *s, const bool echo);

char* rx_buff_get_msg(struct rx_buff *rxb);

bool rx_buff_is_overflow(struct rx_buff *rxb);

enum rx_buff_status rx_buff_get_status(struct rx_buff *rxb);

CPP_GUARD_END

#endif /* _RX_BUFF_H_ */
