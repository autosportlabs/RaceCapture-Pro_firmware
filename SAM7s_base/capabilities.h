#ifndef CAPABILITIES_H_
#define CAPABILITIES_H_
#include "serial.h"
#include "FreeRTOSConfig.h"

//Hardware capabilities for RCP MK1

#define TICK_RATE_HZ			configTICK_RATE_HZ

//configuration
#define MAX_CHANNELS			90
#define MAX_TRACKS				40
#define SCRIPT_MEMORY_LENGTH	10240

//Input / output Channels
#define ANALOG_CHANNELS 		8
#define IMU_CHANNELS			4
#define	GPIO_CHANNELS			3
#define TIMER_CHANNELS			3
#define PWM_CHANNELS			4
#define CAN_CHANNELS			1
#define CONNECTIVITY_CHANNELS	2

//sample rates
#define MAX_SENSOR_SAMPLE_RATE	100
#define MAX_GPS_SAMPLE_RATE		10

//system info
#define DEVICE_NAME    "RCP"
#define FRIENDLY_DEVICE_NAME "RaceCapture/Pro"
#define COMMAND_PROMPT "RaceCapture/Pro"

#endif /* CAPABILITIES_H_ */
