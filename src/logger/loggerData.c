/*
 * loggerData.c
 *
 *  Created on: Jun 1, 2012
 *      Author: brent
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
