/*
 * bluetooth.h
 *
 *  Created on: Oct 29, 2013
 *      Author: brent
 */

#ifndef BLUETOOTH_H_
#define BLUETOOTH_H_

#include "stddef.h"
#include "devices_common.h"

int bt_init_connection(DeviceConfig *config);
int bt_check_connection_status(DeviceConfig *config);

#endif /* BLUETOOTH_H_ */
