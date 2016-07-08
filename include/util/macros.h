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

#ifndef _MACROS_H_
#define _MACROS_H_

#include "cpp_guard.h"

#include <string.h>

CPP_GUARD_BEGIN

/**
 * Calculates the length of an array of items.
 */
#define ARRAY_LEN(x) (sizeof(x)/sizeof(*x))

/**
 * Checks if two strings are equal.
 */
#define STR_EQ(s1, s2)	(0 == strcmp((s1), (s2)))

/**
 * Chooses the maximum of two values.
 */
#define MAX(a,b) (((a)>(b))?(a):(b))

/**
 * Chooses the minimum of two values.
 */
#define MIN(a,b) (((a)<(b))?(a):(b))

CPP_GUARD_END

#endif /* _MACROS_H_ */
