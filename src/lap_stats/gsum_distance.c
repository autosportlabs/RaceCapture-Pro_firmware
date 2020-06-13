/*
 * Race Capture Pro Firmware
 *
 * Copyright (C) 2016 Autosport Labs
 *
 * This file is part of the Race Capture Pro fimrware suite
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

#include "gsum_distance.h"
#include "lap_stats.h"
#include "loggerConfig.h"
#include <stdint.h>
#include <string.h>

#define _LOG_PFX            "[gsumdistance] "

//can handle a track up to 10km
#define GSUM_MAX_SEGMENTS                    1000
#define SEGMENT_LEN                          0.01 //10 meters

int get_segment_by_distance() 
{
        float current_distance = getLapDistance();
        int current_segment = (int)(current_distance / SEGMENT_LEN);
        if (current_segment > GSUM_MAX_SEGMENTS - 1)
                current_segment = GSUM_MAX_SEGMENTS - 1;

        return current_segment;
}