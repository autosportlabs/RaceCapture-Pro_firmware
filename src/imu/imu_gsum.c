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


#include "imu.h"
#include "imu_device.h"
#include "loggerConfig.h"
#include "stddef.h"
#include "capabilities.h"

static float g_gsum;
static float g_gsum_maxes[CONFIG_GSUM_MAX_SEGMENTS];

int gsum_init()
{
        calculate_gsum_distance_segments();
}

float calculate_gsum_max()
{
        int current_segment = get_segment_by_distance();
        float *distance_segments = get_distance_segments();

        float current_gsum_max = g_gsum_maxes[segment];
        if (g_gsum > current_gsum_max) {
                current_gsum_max = g_gsum;
                g_gsum_maxes[segment] = g_gsum;
        }

        return current_gsum_max;
}

float get_current_gsum()
{
        return g_gsum;
}
