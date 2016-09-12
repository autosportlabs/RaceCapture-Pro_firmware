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

#ifndef _AT_BASIC_H_
#define _AT_BASIC_H_

#include "cpp_guard.h"
#include "dateTime.h"
#include "serial.h"
#include <stdbool.h>
#include <stddef.h>

CPP_GUARD_BEGIN

bool at_basic_wait_for_msg(struct Serial* serial, const char* msg,
			   const tiny_millis_t delay_ms);

bool at_basic_ping(struct Serial* serial, const size_t tries,
		   const tiny_millis_t delay_ms);

int at_basic_probe(struct Serial* serial, const int bauds[],
		   const size_t size, const size_t tries,
		   const tiny_millis_t delay, const size_t msg_bits,
		   const size_t parity, const size_t stop_bits);

CPP_GUARD_END


#endif /* _AT_BASIC_H_ */
