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


#include <math.h>

#include "imu_gsum.h"
#include "imu.h"
#include "imu_device.h"
#include "lap_stats.h"
#include "loggerConfig.h"
#include "stddef.h"
#include "capabilities.h"

static float g_gsum;

float get_imu_gsum(void)
{
        LoggerConfig *config = getWorkingLoggerConfig();
        float x_value = imu_read_value(IMU_CHANNEL_X, &config->ImuConfigs[IMU_CHANNEL_X]);
        float y_value = imu_read_value(IMU_CHANNEL_Y, &config->ImuConfigs[IMU_CHANNEL_Y]);
        g_gsum = sqrtf((y_value*y_value)+(x_value*x_value));

        return g_gsum;
}

#ifdef GSUMMAX
static bool  g_gsum_initd = false;
static float g_gsummaxes[GSUMMAX_SEGMENTS+1]; // add an extra segment for when you are "beyond" the track.

void gsum_reset_maxes(void)
{
	for( int i=0; i <= GSUMMAX_SEGMENTS; i++ )
	{
		g_gsummaxes[i] = GSUMMAX_MINVAL;
	}
	g_gsum_initd = true;
}

int get_segment_by_distance() 
{

        float current_dist_km = getLapDistance();
        int current_segment = (int)(current_dist_km / GSUMMAX_SEGMENT_LEN);
        if (current_segment > GSUMMAX_SEGMENTS)
                current_segment = GSUMMAX_SEGMENTS;

        return current_segment;
}

float get_imu_gsummax(void)
{
	if ( ! g_gsum_initd )
		gsum_reset_maxes();

        int current_segment = get_segment_by_distance();

        float current_gsummax = g_gsummaxes[current_segment];
        if (g_gsum > current_gsummax) {
                current_gsummax = g_gsum;
                g_gsummaxes[current_segment] = g_gsum;
        }

        return current_gsummax;
}

float get_imu_gsumpct(void)
{
        int current_segment = get_segment_by_distance();
	float ret = (g_gsum / g_gsummaxes[current_segment]) * 100.0f;
	return ret;
}

#endif
