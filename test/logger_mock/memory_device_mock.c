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
#include <string.h>
#include <stdio.h>

static int g_isFlashed = 0;

enum memory_flash_result_t memory_device_flash_region(const void *vAddress, const void *vData, unsigned int length)
{
    g_isFlashed = 1;
    void * addr = (void *)vAddress;
    memcpy(addr, vData, length);
    //printf("\r\nflash: %d %d |%s|\r\n", length, strlen((const char *)vData), (const char*)vData);
    return MEMORY_FLASH_SUCCESS;
}

void memory_mock_set_is_flashed(int isFlashed)
{
    g_isFlashed = isFlashed;
}

int memory_mock_get_is_flashed()
{
    return g_isFlashed;
}
