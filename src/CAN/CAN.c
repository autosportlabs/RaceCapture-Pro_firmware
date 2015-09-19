/**
 * AutoSport Labs - Race Capture Firmware
 *
 * Copyright (C) 2014 AutoSport Labs
 *
 * This file is part of the Race Capture firmware suite
 *
 * This is free software: you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details. You should have received a copy of the GNU
 * General Public License along with this code. If not, see <http://www.gnu.org/licenses/>.
 */

#include "CAN.h"
#include "CAN_device.h"
#include "loggerConfig.h"
#include "printk.h"

int CAN_init(LoggerConfig *loggerConfig)
{
    CANConfig *canConfig = &loggerConfig->CanConfig;

    for (size_t i = 0; i < CAN_CHANNELS; i++)
        if (!CAN_init_port(i, canConfig->baud[i]))
            return 0;


    return 1;
}

int CAN_init_port(uint8_t port, uint32_t baud)
{
    return CAN_device_init(port, baud);
}

int CAN_set_filter(uint8_t channel, uint8_t id, uint8_t extended, uint32_t filter, uint32_t mask)
{
    return CAN_device_set_filter(channel, id, extended, filter, mask);
}

int CAN_tx_msg(uint8_t channel, CAN_msg *msg, unsigned int timeoutMs)
{
    return CAN_device_tx_msg(channel, msg, timeoutMs);
}

int CAN_rx_msg(uint8_t channel, CAN_msg *msg, unsigned int timeoutMs)
{
    return CAN_device_rx_msg(channel, msg, timeoutMs);
}
