/**
 * Race Capture Firmware
 *
 * Copyright (C) 2014 Autosport Labs
 *
 * This file is part of the Race Capture firmware suite
 *
 * This is free software: you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details. You should have received a copy of the GNU
 * General Public License along with this code. If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors: Stieg
 */

#include "FreeRTOS.h"
#include "task.h"
#include "task_testing.h"
#include <unistd.h>

static portTickType ticks;

portTickType xTaskGetTickCount() {
        return ticks;
}

void set_ticks(portTickType new_ticks){
	ticks = new_ticks;
}

void reset_ticks() {
        ticks = 0;
}

void increment_tick() {
        ticks++;
}

void vTaskDelay(portTickType xTicksToDelay) {
        usleep((useconds_t)xTicksToDelay * 1000);
}

signed portBASE_TYPE xTaskGenericCreate(
        pdTASK_CODE pvTaskCode,
        const signed char * const pcName,
        unsigned short usStackDepth,
        void *pvParameters,
        unsigned portBASE_TYPE uxPriority,
        xTaskHandle *pxCreatedTask,
        portSTACK_TYPE *puxStackBuffer,
        const xMemoryRegion * const xRegions )
{
        return 0;
}
