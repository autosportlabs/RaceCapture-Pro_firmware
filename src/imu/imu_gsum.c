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


#include "imu_gsum.h"
#include "imu.h"
#include "imu_device.h"
#include "loggerConfig.h"
#include "stddef.h"
#include "capabilities.h"

static float g_gsum;
static float g_gsum_maxes[MAX_SEGMENTS];

int gsum_init()
{
        init_gsum_distance_segments();
}

float get_imu_gsum(void)
{
        LoggerConfig *config = getWorkingLoggerConfig();
        float x_value = imu_read_value(IMU_CHANNEL_X, &config->ImuConfigs[IMU_CHANNEL_X]);
        float y_value = imu_read_value(IMU_CHANNEL_Y, &config->ImuConfigs[IMU_CHANNEL_Y]);
        g_gsum = sqrt((powf(y_value,2))+(powf(x_value,2)));

        return g_gsum;
}

float get_imu_gsum_max(void)
{
        int current_segment = get_segment_by_distance();

        float current_gsum_max = g_gsum_maxes[current_segment];
        if (g_gsum > current_gsum_max) {
                current_gsum_max = g_gsum;
                g_gsum_maxes[current_segment] = g_gsum;
        }

        return current_gsum_max;
}

float get_imu_gsum_pct(void)
{
        int current_segment = get_segment_by_distance();

        return g_gsum / g_gsum_maxes[current_segment];
}
