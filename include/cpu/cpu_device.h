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

#ifndef CPU_DEVICE_H_
#define CPU_DEVICE_H_

#include "cpp_guard.h"
#include <stdint.h>

CPP_GUARD_BEGIN

int cpu_device_init(void);
void cpu_device_reset(int bootloader);
const char * cpu_device_get_serialnumber(void);

void cpu_device_spin(uint32_t ms);

CPP_GUARD_END

#endif /* CPU_DEVICE_H_ */
