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

#ifndef IMU_DEVICE_H_
#define IMU_DEVICE_H_

#include "cpp_guard.h"
#include "imu.h"

CPP_GUARD_BEGIN

void imu_device_init();

enum imu_init_status {
        IMU_INIT_STATUS_FAILED  = -1,
        IMU_INIT_STATUS_UNINIT  =  0,
        IMU_INIT_STATUS_SUCCESS =  1,
};

enum imu_init_status imu_device_init_status();

int imu_device_read(enum imu_channel channel);

float imu_device_counts_per_unit(enum imu_channel channel);

CPP_GUARD_END

#endif /* IMU_DEVICE_H_ */
