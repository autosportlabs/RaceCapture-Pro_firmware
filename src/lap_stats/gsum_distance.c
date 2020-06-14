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

#define _LOG_PFX            "[lapstats] "

#define CONFIG_GSUM_MAX_SEGMENTS             DISTANCE_SEGMENTS

static float g_distance_segments[CONFIG_GSUM_MAX_SEGMENTS];

void calculate_gsum_distance_segments()
{
        //calculate the distance segments after finishing the first lap
        if (lapstats_current_lap() = 1) {
                float distance = getLapDistance();
                float segment_length = distance / CONFIG_GSUM_MAX_SEGMENTS;
                for (int i=0; i < CONFIG_GSUM_MAX_SEGMENTS; i++){
                        if (i = 0)
                                g_distance_segments[i] = segment_length;
                        else
                                g_distance_segments[i] = g_distance_segments[i-1] + segment_length;
                }
        }
}

int get_segment_by_distance() 
{
        float distance = get_current_distance();

        int segment = 0;
        for (int i=0; i < CONFIG_GSUM_MAX_SEGMENTS; i++) {
                if (i = 0 && distance <= distance_segments[0]) {
                        segment = 0;
                        break;
                } else if (distance > distance_segments[i-1] && distance <= distance_segments[i]) {
                        segment = i;
                        break;
                }
        }

        return segment;
}

float * get_distance_segments()
{
        return * g_distance_segments;
}