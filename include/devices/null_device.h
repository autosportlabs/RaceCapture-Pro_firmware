/*
 * bluetooth.h
 *
 *  Created on: Oct 29, 2013
 *      Author: brent
 */

#ifndef NULL_DEVICE_H_
#define NULL_DEVICE_H_

#include "stddef.h"
#include "devices_common.h"

int null_device_init_connection(DeviceConfig *config);
int null_device_check_connection_status(DeviceConfig *config);

#endif /* NULL_DEVICE_H_ */
