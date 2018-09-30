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


#include "loggerData.h"
#include "loggerHardware.h"
#include "imu.h"
#include "ADC.h"
#include "gps.h"
#include "linear_interpolate.h"
#include "predictive_timer_2.h"
#include "filter.h"

void init_logger_data()
{
}

void doBackgroundSampling()
{
        imu_sample_all();
        ADC_sample_all();
}
