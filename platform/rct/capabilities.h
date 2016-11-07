#ifndef CAPABILITIES_H_
#define CAPABILITIES_H_
#include "serial.h"
#include "FreeRTOSConfig.h"

//Hardware capabilities for RC/T

#define TICK_RATE_HZ			configTICK_RATE_HZ
#define MS_PER_TICK 1

/* Support Flags */
#define BLUETOOTH_SUPPORT	0
#define CELLULAR_SUPPORT	0
#define LUA_SUPPORT		0
#define SDCARD_SUPPORT		0
#define USB_SERIAL_SUPPORT	1
#define VIRTUAL_CHANNEL_SUPPORT	0
#define WIFI_SUPPORT		1

/* Wifi Specific Info */
#define WIFI_MAX_BAUD		921600
#define WIFI_MAX_SAMPLE_RATE	50
#define WIFI_ENABLED_DEFAULT	true

/* Configuration */
#define MAX_TRACKS	0
#define MAX_SECTORS	20
#define MAX_VIRTUAL_CHANNELS	30
/*
 * What is the maximum number of samples available per predictive time
 * buffer.  More samples == better resolution. Each slot is 12 bytes.
 */
#define PREDICTIVE_TIME_MAX_SAMPLES	72


//Input / output Channels
#define ANALOG_CHANNELS	1
#define IMU_CHANNELS	7
#define	GPIO_CHANNELS	0
#define TIMER_CHANNELS	0
#define PWM_CHANNELS	0
#define CAN_CHANNELS	1
#define CONNECTIVITY_CHANNELS	2

//sample rates
#define MAX_SENSOR_SAMPLE_RATE	50
#define MAX_GPS_SAMPLE_RATE	50
#define MAX_OBD2_SAMPLE_RATE	50

//logger message buffering
#define LOGGER_MESSAGE_BUFFER_SIZE  5

/* Logging Buffer Size (in 1K Blocks) */
#define LOG_BUFFER_SIZE	(1024 * 3)

/* Rx Max Message length */
#define RX_MAX_MSG_LEN	768

//system info
#define DEVICE_NAME    "RCT"
#define FRIENDLY_DEVICE_NAME "RaceCapture"

/* LUA Configuration */

/*
 * What is the maximum length of the script that can be provided?
 * Must be divisible by 256.
 */
#define SCRIPT_MEMORY_LENGTH	0

/*
 * Defines the memory ceiling for LUA.  In other words, how much RAM can
 * LUA allocate before we say no.  This keeps LUA from crashing the system
 * when a memory hog LUA script is running.  Set to 0 for no limit.
 */
#define LUA_MEM_MAX 0

/*
 * These values dictate how the LUA garbage collector will behave.
 * Tweaking these is necessary in low memory environments to ensure
 * that LUA's memory footprint does not exceed what can be spared.
 * A value of 0 means that you want to use the default.  For more info
 * see http://www.lua.org/manual/5.1/manual.html#2.10
 */
#define LUA_GC_PAUSE_PCT	0
#define LUA_GC_STEP_MULT_PCT	0

/*
 * Controls whether or not we allow LUA to register the nice to have
 * external libs.  These come at a memory cost, but are useful.
 */
#define LUA_REGISTER_EXTERNAL_LIBS	0

/* How big is our hardware init stack */
#define HARDWARE_INIT_STACK_SIZE	192

#endif /* CAPABILITIES_H_ */
