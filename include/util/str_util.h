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

#ifndef _STR_UTIL_H_
#define _STR_UTIL_H_

#include "cpp_guard.h"
#include <stddef.h>

CPP_GUARD_BEGIN

#define INVALID_CHAR	0xFF

size_t serial_msg_strlen(const char *data);

char* lstrip_inline(char *str);

char* rstrip_inline(char *str);

char* strip_inline(char *str);

char* str_util_lstrip_zeros_inline(char *str);

char* str_util_rstrip_zeros_inline(char *str);

char* str_util_strip_zeros_inline(char *str);

char* strntcpy(char* dest, const char* src, size_t n);

CPP_GUARD_END

#endif /* _STR_UTIL_H_ */
