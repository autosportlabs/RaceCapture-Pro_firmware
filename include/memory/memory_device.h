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

#ifndef MEMORY_DEVICE_H_
#define MEMORY_DEVICE_H_

#include "cpp_guard.h"
#include "memory.h"
#include <stdbool.h>
#include <stddef.h>

CPP_GUARD_BEGIN

bool memory_device_region_clear(const volatile void *address);

int memory_device_write_words(const volatile void* address, const void* data,
			      const size_t len);

enum memory_flash_result_t memory_device_flash_region(const volatile void *vAddress,
						      const void *vData,
						      unsigned int length);

CPP_GUARD_END

#endif /* MEMORY_DEVICE_H_ */
