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

#ifndef __DEBUG_H__
#define __DEBUG_H__

#include "cpp_guard.h"

CPP_GUARD_BEGIN

//#define RCP_INFO
#ifdef RCP_INFO
#include <stdio.h>
#define INFO(s, ...) printf(s, ##__VA_ARGS__)
#else
#define INFO(s, ...)
#endif // RCP_INFO

//#define RCP_DEBUG
#ifdef RCP_DEBUG
#include <stdio.h>
#define DEBUG(s, ...) printf(s, ##__VA_ARGS__)
#else
#define DEBUG(s, ...)
#endif // RCP_DEBUG

//#define RCP_DEVEL
#ifdef RCP_DEVEL
#include <stdio.h>
#define DEVEL(s, ...) printf(s, ##__VA_ARGS__)
#else
#define DEVEL(s, ...)
#endif // RCP_DEVEL

CPP_GUARD_END

#endif // __DEBUG_H__
