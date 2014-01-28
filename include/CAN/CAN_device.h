/*
 * CAN_device.h
 *
 *  Created on: Jan 26, 2014
 *      Author: brent
 */

#ifndef CAN_DEVICE_H_
#define CAN_DEVICE_H_

#include "CAN.h"

int CAN_device_init();
int CAN_device_set_baud(int baud);
int CAN_device_tx_msg(CAN_msg *msg, unsigned int timeoutMs);
int CAN_device_rx_msg(CAN_msg *msg, unsigned int timeoutMs);

#endif /* CAN_DEVICE_H_ */
