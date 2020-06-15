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

#define MAX_SEGMENTS                         DISTANCE_SEGMENTS
#define MIN_SEGMENT_LEN                      10

static int num_segments;
static float segment_len;

void init_gsum_distance_segments()
{
        float track_len = getLapDistance();
        if (track_len / MIN_SEGMENT_LEN < MAX_SEGMENTS) {
                num_segments = track_len / MIN_SEGMENT_LEN;
                segment_len = MIN_SEGMENT_LEN;
        } else {
                num_segments = MAX_SEGMENTS;
                segment_len = track_len / MAX_SEGMENTS;
        }
}

int get_segment_by_distance() 
{
        float current_distance = getLapDistance();
        int current_segment = current_distance / segment_len;

        return current_segment;
}
