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

#ifndef IMU_GSUM_H_
#define IMU_GSUM_H_

#include "cpp_guard.h"
#include "imu.h"
#include "loggerConfig.h"

CPP_GUARD_BEGIN

float get_imu_gsum(void);
void  reset_gsum(void);

#ifdef GSUMMAX
//can handle a track up to 10km with fixed seg length of 10m
#define GSUMMAX_SEGMENT_LEN		(0.01f)	// in km
#define GSUMMAX_SEGMENTS		1000    // prime number to avoid aliasing artifacts. 

#define GSUMMAX_MINVAL			(0.000001f)

float get_imu_gsummax(void);
float get_imu_gsumpct(void);
int   get_segment_by_distance(void);
#endif


CPP_GUARD_END

#endif /* IMU_GSUM_H_ */
