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

#include "memory_device.h"
#include "memory_mock.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int g_isFlashed = 0;

bool memory_device_region_clear(const volatile void *addr)
{
	g_isFlashed = 0;
	return true;
}

int memory_device_write_words(const volatile void* addr, const void* data,
			      const size_t len)
{
	memcpy((void *) addr, data, len);
	g_isFlashed = 1;
	return (int) len;
}

enum memory_flash_result_t memory_device_flash_region(const volatile void *vAddress,
						      const void *vData,
						      unsigned int length)
{
	const bool status =
		memory_device_region_clear(vAddress) &&
		length <= memory_device_write_words(vAddress, vData, length);

	return status ? MEMORY_FLASH_SUCCESS : MEMORY_FLASH_WRITE_ERROR;
}

void memory_mock_set_is_flashed(int isFlashed)
{
	g_isFlashed = isFlashed;
}

int memory_mock_get_is_flashed()
{
	return g_isFlashed;
}
