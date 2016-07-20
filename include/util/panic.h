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

#ifndef _PANIC_H_
#define _PANIC_H_

#include "cpp_guard.h"

CPP_GUARD_BEGIN

enum panic_cause {
	PANIC_CAUSE_HARDWARE	= 1,
	PANIC_CAUSE_SCHEDULER	= 2,
	PANIC_CAUSE_OVERFLOW	= 3,
	PANIC_CAUSE_MALLOC	= 4,
	PANIC_CAUSE_UNREACHABLE = 5,
	PANIC_CAUSE_TASK_CREATE = 6,
	PANIC_CAUSE_ASSERT	= 7,
};

void panic(const enum panic_cause cause);

CPP_GUARD_END

#endif /* _PANIC_H_ */
