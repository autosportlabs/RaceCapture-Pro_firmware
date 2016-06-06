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

#include "ADC.h"
#include "CAN.h"
#include "GPIO.h"
#include "led.h"
#include "PWM.h"
#include "constants.h"
#include "imu.h"
#include "loggerConfig.h"
#include "loggerHardware.h"
#include "memory.h"
#include "sdcard.h"
#include "timer.h"
#include "usart.h"
#include "usb_comm.h"
#include "virtual_channel.h"
#include "watchdog.h"

void InitLoggerHardware()
{
    LoggerConfig *loggerConfig = getWorkingLoggerConfig();
    usart_init();
    led_init();

#if IMU_CHANNELS > 0
    imu_init(loggerConfig);
#endif
#if ANALOG_CHANNELS > 0
    ADC_init(loggerConfig);
#endif
#if PWM_CHANNELS > 0
    PWM_init(loggerConfig);
#endif
#if GPIO_CHANNELS > 0
    GPIO_init(loggerConfig);
#endif
#if TIMER_CHANNELS > 0
    timer_init(loggerConfig);
#endif
#if CAN_CHANNELS > 0
    CAN_init(loggerConfig);
#endif
#if SDCARD_SUPPORT
    InitFSHardware();
#endif

}
