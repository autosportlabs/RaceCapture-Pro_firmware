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


#include "CAN.h"
#include "FreeRTOS.h"
#include "OBD2.h"
#include "loggerConfig.h"
#include "printk.h"
#include "task.h"
#include "taskUtil.h"

int OBD2_get_value_for_pid(uint8_t pid)
{
    return 0;
}

int OBD2_request_PID(uint8_t pid, uint8_t mode, size_t timeout)
{
		CAN_msg msg;
		msg.addressValue = 0x7df;
		msg.data[0] = 2;
		msg.data[1] = mode;
		msg.data[2] = pid;
		msg.data[3] = 0x55;
		msg.data[4] = 0x55;
		msg.data[5] = 0x55;
		msg.data[6] = 0x55;
		msg.data[7] = 0x55;
		msg.dataLength = 8;
		msg.isExtendedAddress = 0;
		return CAN_tx_msg(0, &msg, timeout);
}

