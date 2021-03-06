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

#ifndef INCLUDE_CAN_CAN_AUX_QUEUE_H_
#define INCLUDE_CAN_CAN_AUX_QUEUE_H_
#include "CAN.h"
#include <stddef.h>

#define CAN_AUX_QUEUE_LENGTH 5

/**
 * Initializes the CAN aux message queues
 */
bool CAN_aux_queue_init(void);

/**
 * Puts a CAN message into the Auxiliary CAN message queue
 * @param msg the CAN message to put
 * @return true if the message was successfully added
 */
bool CAN_aux_queue_put_msg(CAN_msg * can_msg);

/**
 * Gets a CAN message from the Auxiliary CAN message queue
 * @param can_bus the CAN bus to retrieve from
 * @param msg the CAN message to populate
 * @param timeout to wait in ms
 * @return true if the message was successfully retrieved
 */
bool CAN_aux_queue_get_msg(uint8_t can_bus, CAN_msg * can_msg, size_t timeout_ms);

#endif /* INCLUDE_CAN_CAN_AUX_QUEUE_H_ */
