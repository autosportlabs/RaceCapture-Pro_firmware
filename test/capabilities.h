#ifndef CAPABILITIES_H_
#define CAPABILITIES_H_
#include "serial.h"


//Hardware capabilities for mocked logger

#define TICK_RATE_HZ			1000
#define MS_PER_TICK 5
//configuration
#define MAX_TRACKS				240
#define MAX_SECTORS				20
#define MAX_VIRTUAL_CHANNELS	10

#define LOGGER_MESSAGE_BUFFER_SIZE	5

/*
 * Adds additional memory saving behavior for low memory systems.
 * These come at a cost of interruption of other services as needed
 * to save RAM.  Usually quick interruptions, but interruptions
 * none the less.
 */
#define RCP_LOW_MEM	0


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

//logging
#define LOG_BUFFER_SIZE			1024

//system info
#define DEVICE_NAME    "RCP_SIM"
#define FRIENDLY_DEVICE_NAME "RaceCapture/Pro Sim"
#define COMMAND_PROMPT "RaceCapture/Pro Sim"
#define VERSION_STR MAJOR_REV_STR "." MINOR_REV_STR "." BUGFIX_REV_STR
#define WELCOME_MSG "Welcome to RaceCapture/Pro Sim: Firmware Version " VERSION_STR



#endif /* CAPABILITIES_H_ */
