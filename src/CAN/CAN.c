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
#include "CAN_device.h"
#include "loggerConfig.h"
#include "printk.h"
#include "led.h"
#include <stdbool.h>

int CAN_init(LoggerConfig *loggerConfig)
{
    CANConfig *canConfig = &loggerConfig->CanConfig;

    for (size_t i = 0; i < CAN_CHANNELS; i++) {
        bool termination = false;
#if CAN_SW_TERMINATION == true
        termination = canConfig->termination[i];
#endif
        if (!CAN_init_port(i, canConfig->baud[i], termination))
            return 0;
    }
    return 1;
}

int CAN_init_port(const uint8_t port, const uint32_t baud, const bool termination_enabled)
{
    return CAN_device_init(port, baud, termination_enabled);
}

int CAN_set_filter(const uint8_t channel, const uint8_t id, const uint8_t extended,
           const uint32_t filter, const uint32_t mask, const bool enabled)
{
    return CAN_device_set_filter(channel, id, extended, filter,
                     mask, enabled);
}

int CAN_tx_msg(const uint8_t channel, const CAN_msg *msg, const unsigned int timeoutMs)
{
    const int rc = CAN_device_tx_msg(channel, msg, timeoutMs);
    if (rc)
            led_toggle(LED_CAN);
    return rc;
}

int CAN_rx_msg(CAN_msg *msg, const unsigned int timeoutMs)
{
    const int rc = CAN_device_rx_msg(msg, timeoutMs);
    if (rc)
            led_toggle(LED_CAN);
    return rc;
}
