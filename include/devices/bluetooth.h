/*
 * bluetooth.h
 *
 *  Created on: Oct 29, 2013
 *      Author: brent
 */

#ifndef BLUETOOTH_H_
#define BLUETOOTH_H_

#include "stdint.h"

void bt_init_connection();
int bt_check_connection_status(char *buffer, size_t size);

#endif /* BLUETOOTH_H_ */
