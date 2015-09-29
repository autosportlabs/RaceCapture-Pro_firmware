#ifndef CAPABILITIES_H_
#define CAPABILITIES_H_
#include "serial.h"
#include "FreeRTOSConfig.h"

//Hardware capabilities for RCP MK1

#define TICK_RATE_HZ			configTICK_RATE_HZ
#define MS_PER_TICK 5

//configuration
#define MAX_TRACKS				40
#define MAX_SECTORS				20
#define MAX_VIRTUAL_CHANNELS	10
#define LOGGER_MESSAGE_BUFFER_SIZE	5


/* LUA Configuration */
/*
 * What is the maximum length of the script that can be provided?
 *
 * Must be divisible by 256 (write block size in Sam7s)
 */
#define SCRIPT_MEMORY_LENGTH	(1024 * 4)

/*
 * Defines the memory ceiling for LUA.  In other words, how much RAM can
 * LUA allocate before we say no.  This keeps LUA from crashing the system
 * when a memory hog LUA script is running.  Set to 0 for no limit.
 *
 * 16 K seems to be all we really have.
 */
#define LUA_MEM_MAX (1024 * 16)

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
#define LUA_GC_PAUSE_PCT	110
#define LUA_GC_STEP_MULT_PCT	1000


//Input / output Channels
#define ANALOG_CHANNELS 		8
#define IMU_CHANNELS			4
#define	GPIO_CHANNELS			3
#define TIMER_CHANNELS			3
#define PWM_CHANNELS			4
#define CAN_CHANNELS			1
#define CONNECTIVITY_CHANNELS	1

//sample rates
#define MAX_SENSOR_SAMPLE_RATE	100
#define MAX_GPS_SAMPLE_RATE		10
#define MAX_OBD2_SAMPLE_RATE	10

//logging
#define LOG_BUFFER_SIZE			1024

//system info
#define DEVICE_NAME    "RCP"
#define FRIENDLY_DEVICE_NAME "RaceCapture/Pro"
#define COMMAND_PROMPT "RaceCapture/Pro"
#define VERSION_STR MAJOR_REV_STR "." MINOR_REV_STR "." BUGFIX_REV_STR
#define WELCOME_MSG "Welcome to RaceCapture/Pro : Firmware Version " VERSION_STR

//initialize main tasks in temporary FreeRTOS task
#define TASK_TASK_INIT 0
#endif /* CAPABILITIES_H_ */
