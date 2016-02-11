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
#include "filter.h"
#include "stddef.h"
#include "printk.h"

//Channel Filters
static Filter g_imu_filter[CONFIG_IMU_CHANNELS];

static void init_filters(LoggerConfig *loggerConfig)
{
    ImuConfig *config  = loggerConfig->ImuConfigs;
    for (size_t i = 0; i < CONFIG_IMU_CHANNELS; i++) {
        float alpha = (config + i)->filterAlpha;
        init_filter(&g_imu_filter[i], alpha);
    }
}

void imu_sample_all()
{
    for (size_t i = 0; i < CONFIG_IMU_CHANNELS; i++) {
        update_filter(&g_imu_filter[i], imu_read(i));
    }
}

float imu_read_value(enum imu_channel channel, ImuConfig *ac)
{
        const size_t physicalChannel = ac->physicalChannel;
        const int raw = g_imu_filter[physicalChannel].current_value;
        const int zeroValue = ac->zeroValue;
        const float countsPerUnit = imu_device_counts_per_unit(channel);
        const float scaledValue = ((float) (raw - zeroValue)) / countsPerUnit;

        /* now alter based on configuration */
        switch (ac->mode) {
        case IMU_MODE_NORMAL:
                return scaledValue;
        case IMU_MODE_INVERTED:
                return -scaledValue;
        case IMU_MODE_DISABLED:
        default:
                return 0;
        }
}

static void imu_flush_filter(size_t physicalChannel)
{
    for (size_t i = 0; i < 1000; i++) {
        update_filter(&g_imu_filter[physicalChannel], imu_read(physicalChannel));
    }
}

void imu_calibrate_zero()
{
    for (size_t logicalChannel = 0; logicalChannel < CONFIG_IMU_CHANNELS; logicalChannel++) {
        ImuConfig * c = getImuConfigChannel(logicalChannel);
        size_t physicalChannel = c->physicalChannel;
        imu_flush_filter(physicalChannel);
        int zeroValue = g_imu_filter[physicalChannel].current_value;
        float countsPerUnit = imu_device_counts_per_unit(physicalChannel);
        if (logicalChannel == IMU_CHANNEL_Z) { //adjust for gravity
            if (c->mode == IMU_MODE_INVERTED) {
                countsPerUnit = -countsPerUnit;
            }
            zeroValue -= countsPerUnit;
        }
        c->zeroValue = zeroValue;
    }
}

int imu_init(LoggerConfig *loggerConfig)
{
        /* TODO BAP: IMU is unhappy */
        imu_device_init();
        init_filters(loggerConfig);
        return 1;
}

int imu_soft_init(LoggerConfig *loggerConfig)
{
    init_filters(loggerConfig);
    return 1;
}

int imu_read(enum imu_channel channel)
{
    return imu_device_read(channel);
}
