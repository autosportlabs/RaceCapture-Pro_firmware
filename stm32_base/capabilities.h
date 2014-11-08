#ifndef CAPABILITIES_H_
#define CAPABILITIES_H_
#include "serial.h"
#include "FreeRTOSConfig.h"

//Hardware capabilities for RCP MK2

#define TICK_RATE_HZ			configTICK_RATE_HZ
#define MS_PER_TICK 1

//configuration
#define MAX_CHANNELS			200
#define MAX_TRACKS				240
#define SCRIPT_MEMORY_LENGTH	10240

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

//system info
#define DEVICE_NAME    "RCP_MK2"
#define FRIENDLY_DEVICE_NAME "RaceCapture/Pro MK2"
#define COMMAND_PROMPT "RaceCapture/Pro MK2"
#define WELCOME_MSG "Welcome to RaceCapture/Pro MK2 : Firmware Version " MAJOR_REV_STR "." MINOR_REV_STR "." BUGFIX_REV_STR

//initialize main tasks in temporary FreeRTOS task
#define TASK_TASK_INIT 1
#endif /* CAPABILITIES_H_ */
