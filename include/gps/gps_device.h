/*
 * gps_device.h
 *
 *  Created on: Aug 24, 2014
 *      Author: brent
 */

#ifndef GPS_DEVICE_H_
#define GPS_DEVICE_H_
#include "serial.h"
#include "gps.h"

typedef enum {
	GPS_MSG_SUCCESS = 0,
	GPS_MSG_TIMEOUT,
	GPS_MSG_NONE
} gps_msg_result_t;

int GPS_device_provision(uint8_t targetSampleRate, Serial *serial);
gps_msg_result_t GPS_device_get_update(GpsSample *gpsSample, Serial *serial);

#endif /* GPS_DEVICE_H_ */
