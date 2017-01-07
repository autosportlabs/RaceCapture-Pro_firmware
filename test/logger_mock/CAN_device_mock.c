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

#include "CAN_device.h"
#include <string.h>
#include <stdbool.h>

#define CANBUSSES 2
#define CANQUEUESIZE 10

CAN_msg msgs[CANBUSSES][CANQUEUESIZE];
static bool queueEmpty = true;
static int msgRxIndex = 0;
static int queueCount = 0;
static int msgTxIndex = 0;

int CAN_device_init(uint8_t channel, uint32_t baud)
{
    queueEmpty = true;
    msgRxIndex = 0;
    msgTxIndex = 0;

    return 1;
}

int CAN_device_tx_msg(uint8_t channel, CAN_msg *msg, unsigned int timeoutMs)
{
    if (channel < (CANBUSSES - 1) && queueCount < CANQUEUESIZE) {
        if (msgTxIndex >= CANQUEUESIZE) {
            msgTxIndex = 0;
        }

        CAN_msg *txMsg;
        txMsg = &(msgs[channel][msgTxIndex]);
        txMsg->isExtendedAddress = msg->isExtendedAddress;
        txMsg->addressValue = msg->addressValue;
        txMsg->dataLength = msg->dataLength;
        memcpy(txMsg->data, msg->data, msg->dataLength);

        ++msgTxIndex;
        ++queueCount;
        queueEmpty = false;
        return 1;
    }
    else {
        return 0;
    }
}

int CAN_device_rx_msg(uint8_t channel, CAN_msg *msg, unsigned int timeoutMs)
{
    if (channel < (CANBUSSES - 1) && queueCount > 0) {
        if (msgRxIndex >= CANQUEUESIZE) {
            msgRxIndex = 0;
        }

        *msg = msgs[channel][msgRxIndex];
        ++msgRxIndex;
        --queueCount;

        if (queueCount <= 0)
        {
            queueCount = 0;
        }

        return 1;
    }
    else
    {
        return 0;
    }
}

int CAN_device_set_filter(uint8_t channel, uint8_t id, uint8_t extended,
			  uint32_t filter, uint32_t mask, const bool enabled)
{
    return 1;
}
