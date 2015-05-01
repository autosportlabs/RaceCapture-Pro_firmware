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
#define SCRIPT_MEMORY_LENGTH	10240
#define MAX_VIRTUAL_CHANNELS	10

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
