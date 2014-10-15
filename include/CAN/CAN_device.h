/*
 * CAN_device.h
 *
 *  Created on: Jan 26, 2014
 *      Author: brent
 */

#ifndef CAN_DEVICE_H_
#define CAN_DEVICE_H_
#include "CAN.h"
#include <stdint.h>
#include "loggerConfig.h"

int CAN_device_init(size_t channel, uint32_t baud);
int CAN_device_set_filter(uint8_t id, uint8_t extended, uint32_t filter, uint32_t mask);
int CAN_device_tx_msg(CAN_msg *msg, unsigned int timeoutMs);
int CAN_device_rx_msg(CAN_msg *msg, unsigned int timeoutMs);

#endif /* CAN_DEVICE_H_ */
