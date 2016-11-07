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

#ifndef CAPABILITIES_H_
#define CAPABILITIES_H_

#include "serial.h"
#include "cpp_guard.h"

CPP_GUARD_BEGIN

//Hardware capabilities for mocked logger

#define TICK_RATE_HZ			1000
#define MS_PER_TICK 5

//USB support
#define USB_SERIAL_SUPPORT	1
#define LUA_SUPPORT		1
#define VIRTUAL_CHANNEL_SUPPORT	1
#define SDCARD_SUPPORT		1
#define CELLULAR_SUPPORT	1
#define BLUETOOTH_SUPPORT	1
#define WIFI_SUPPORT		1

//configuration
#define MAX_TRACKS				240
#define MAX_SECTORS				20
#define MAX_VIRTUAL_CHANNELS	10

/* Wifi Specific Info */
#define WIFI_MAX_BAUD		921600
#define WIFI_MAX_SAMPLE_RATE	50
#define WIFI_ENABLED_DEFAULT	true

/* Rx Max Message length */
#define RX_MAX_MSG_LEN	768

/*
 * What is the maximum number of samples available per predictive time
 * buffer.  More samples == better resolution. Each slot is 12 bytes.
 */
#define PREDICTIVE_TIME_MAX_SAMPLES	96
#define LOGGER_MESSAGE_BUFFER_SIZE	5

/* LUA Configuration */

/*
 * What is the maximum length of the script that can be provided?
 *
 * Must be divisible by 256 (write block size in Sam7s)
 */
#define SCRIPT_MEMORY_LENGTH	(1024 * 16)

/*
 * Defines the memory ceiling for LUA.  In other words, how much RAM can
 * LUA allocate before we say no.  This keeps LUA from crashing the system
 * when a memory hog LUA script is running.  Set to 0 for no limit.
 *
 * 16 K seems to be all we really have.
 */
#define LUA_MEM_MAX (1024 * 50)

/*
 * These values dictate how the LUA garbage collector will behave.
 * Tweaking these is necessary in low memory environments to ensure
 * that LUA's memory footprint does not exceed what can be spared.
 * A value of 0 means that you want to use the default.  For more info
 * see http://www.lua.org/manual/5.1/manual.html#2.10
 *
 * Decreasing the pause pct to 125 to ensure the GC runs frequently.
 * Setting the mult to 300 so the GC can be more agressive.
 */
#define LUA_GC_PAUSE_PCT	0
#define LUA_GC_STEP_MULT_PCT	0

/*
 * Controls whether or not we allow LUA to register the nice to have
 * external libs.  These come at a memory cost, but are useful.
 */
#define LUA_REGISTER_EXTERNAL_LIBS	1


//Input / output Channels
#define ANALOG_CHANNELS 		8
#define IMU_CHANNELS			7
#define	GPIO_CHANNELS			3
#define TIMER_CHANNELS			3
#define PWM_CHANNELS			4
#define CAN_CHANNELS			2
#define CONNECTIVITY_CHANNELS	2

//sample rates
#define MAX_SENSOR_SAMPLE_RATE	1000
#define MAX_GPS_SAMPLE_RATE		50
#define MAX_OBD2_SAMPLE_RATE	1000

//logger message buffering
// Should have no effect in testing.  Kept for consistency.
#define LOGGER_MESSAGE_BUFFER_SIZE  5

// Include this for posterity.  Should make no difference.
#define TASK_TASK_INIT 1

//logging
#define LOG_BUFFER_SIZE			1024

//system info
#define DEVICE_NAME    "RCP_SIM"
#define FRIENDLY_DEVICE_NAME "RaceCapture/Pro Sim"

/* How big is our hardware init stack */
#define HARDWARE_INIT_STACK_SIZE	192

CPP_GUARD_END

#endif /* CAPABILITIES_H_ */
