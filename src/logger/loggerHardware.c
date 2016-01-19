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


#include "loggerHardware.h"
#include "loggerConfig.h"
#include "imu.h"
#include "ADC.h"
#include "timer.h"
#include "CAN.h"
#include "PWM.h"
#include "LED.h"
#include "GPIO.h"
#include "watchdog.h"
#include "sdcard.h"
#include "memory.h"
#include "memory.h"
#include "constants.h"
#include "virtual_channel.h"
#include "usb_comm.h"
#include "usart.h"

void InitLoggerHardware()
{

    LoggerConfig *loggerConfig = getWorkingLoggerConfig();
    usart_init();
    init_serial();
    LED_init();
    imu_init(loggerConfig);
    ADC_init(loggerConfig);
    PWM_init(loggerConfig);
    GPIO_init(loggerConfig);
    InitFSHardware();
    timer_init(loggerConfig);
    CAN_init(loggerConfig);
}
