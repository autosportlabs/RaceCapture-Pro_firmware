#ifndef CAPABILITIES_H_
#define CAPABILITIES_H_
#include "serial.h"
#include "FreeRTOSConfig.h"

//Hardware capabilities for RC/T

#define TICK_RATE_HZ			configTICK_RATE_HZ
#define MS_PER_TICK 1

//USB support
#define USB_SERIAL_SUPPORT      0
#define LUA_SUPPORT             0
#define VIRTUAL_CHANNEL_SUPPORT 0
#define SDCARD_SUPPORT          0
#define CELLULAR_SUPPORT        0

//configuration
#define MAX_TRACKS				20
#define MAX_SECTORS				20
#define MAX_VIRTUAL_CHANNELS	30
#define SCRIPT_MEMORY_LENGTH	10240

//Input / output Channels
#define ANALOG_CHANNELS 		1
#define IMU_CHANNELS			7
#define	GPIO_CHANNELS			0
#define TIMER_CHANNELS			0
#define PWM_CHANNELS			0
#define CAN_CHANNELS			1
#define CONNECTIVITY_CHANNELS	2

//sample rates
#define MAX_SENSOR_SAMPLE_RATE	100
#define MAX_GPS_SAMPLE_RATE		50
#define MAX_OBD2_SAMPLE_RATE	100

//logger message buffering
#define LOGGER_MESSAGE_BUFFER_SIZE  5

//logging
#define LOG_BUFFER_SIZE			1024

//system info
#define DEVICE_NAME    "RCT"
#define FRIENDLY_DEVICE_NAME "RaceCapture"
#define COMMAND_PROMPT "RaceCapture"
#define VERSION_STR MAJOR_REV_STR "." MINOR_REV_STR "." BUGFIX_REV_STR
#define WELCOME_MSG "Welcome to RaceCapture : Firmware Version " VERSION_STR

//initialize main tasks in temporary FreeRTOS task
#define TASK_TASK_INIT 1
#endif /* CAPABILITIES_H_ */
