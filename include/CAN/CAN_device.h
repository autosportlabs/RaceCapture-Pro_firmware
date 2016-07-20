/*
 * Race Capture Firmware
 *
 * Copyright (C) 2016 Autosport Labs
 *
 * This file is part of the Race Capture firmware suite
 *
 * This is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details. You should
 * have received a copy of the GNU General Public License along with
 * this code. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CAN_DEVICE_H_
#define CAN_DEVICE_H_

#include "cpp_guard.h"
#include "CAN.h"
#include "loggerConfig.h"
#include <stdbool.h>
#include <stdint.h>

CPP_GUARD_BEGIN

int CAN_device_init(uint8_t channel, uint32_t baud);
int CAN_device_set_filter(uint8_t channel, uint8_t id, uint8_t extended,
			  uint32_t filter, uint32_t mask, const bool enabled);
int CAN_device_tx_msg(uint8_t channel, CAN_msg *msg, unsigned int timeoutMs);
int CAN_device_rx_msg(uint8_t channel, CAN_msg *msg, unsigned int timeoutMs);

CPP_GUARD_END

#endif /* CAN_DEVICE_H_ */
