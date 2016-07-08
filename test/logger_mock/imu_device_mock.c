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


#include "imu_device.h"
#include "imu_mock.h"
#include "loggerConfig.h"

#define ACCEL_DEVICE_COUNTS_PER_G 				819
#define YAW_DEVICE_COUNTS_PER_DEGREE_PER_SEC	4.69

static unsigned int g_imuDevice[CONFIG_IMU_CHANNELS] = {0,0,0,0};

void imu_mock_set_value(unsigned int channel, unsigned int value)
{
    g_imuDevice[channel] = value;
}

void imu_device_init() { }

int imu_device_read(unsigned int channel)
{
    return g_imuDevice[channel];
}

float imu_device_counts_per_unit(unsigned int channel)
{
    return (channel == IMU_CHANNEL_YAW ? YAW_DEVICE_COUNTS_PER_DEGREE_PER_SEC : ACCEL_DEVICE_COUNTS_PER_G);
}

enum imu_init_status imu_device_init_status()
{
        return IMU_INIT_STATUS_SUCCESS;
}
