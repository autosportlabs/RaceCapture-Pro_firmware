/*
 * null_device.h
 *
 *  Created on: Oct 29, 2013
 *      Author: brent
 */

#ifndef SIM900_H_
#define SIM900_H_

#include "stddef.h"
#include "devices_common.h"

int sim900_init_connection(DeviceConfig *config);
int sim900_check_connection_status(DeviceConfig *config);

#endif /* SIM900_H_ */
