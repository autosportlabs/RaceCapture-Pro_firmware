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

#ifndef CAN_H_
#define CAN_H_

#include "cpp_guard.h"
#include "loggerConfig.h"

#include <stdint.h>

CPP_GUARD_BEGIN

#define CAN_BAUD_50K			50000
#define CAN_BAUD_100K 			100000
#define CAN_BAUD_125K			125000
#define CAN_BAUD_250K			250000
#define CAN_BAUD_500K			500000
#define CAN_BAUD_1M				1000000

#define CAN_MSG_SIZE 8

typedef struct _CAN_msg {
    uint32_t addressValue;
    union {
        uint8_t data[CAN_MSG_SIZE];
        uint16_t data16[CAN_MSG_SIZE / 2];
        uint32_t data32[CAN_MSG_SIZE / 4];
        uint64_t data64;
    };
    uint8_t dataLength;
    uint8_t can_bus;
    bool isExtendedAddress;
} CAN_msg;

int CAN_init(LoggerConfig *loggerConfig);
int CAN_init_port(const uint8_t port, const uint32_t baud, const bool termination_enabled);
int CAN_set_filter(const uint8_t, const uint8_t id, const uint8_t extended, const uint32_t filter,
		   const uint32_t mask, const bool enabled);
int CAN_tx_msg(const uint8_t channel, const CAN_msg *msg, const unsigned int timeoutMs);
int CAN_rx_msg(CAN_msg *msg, const unsigned int timeoutMs);

CPP_GUARD_END

#endif /* CAN_H_ */
