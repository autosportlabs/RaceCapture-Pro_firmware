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

#ifndef CAN_CHANNELS_H_
#define CAN_CHANNELS_H_

#include "loggerConfig.h"
#include "CAN.h"

CPP_GUARD_BEGIN

/**
 * Initialize the list of current values
 * @param values the number of values in the list
 * @return true if the initialization was successful
 */
bool CAN_init_current_values(size_t values);

/**
 * retrieves the current value for the specified channel index
 * @param index the index of the channel to retrieve
 * @return the current value for the channel
 */
float CAN_get_current_channel_value(int index);

/**
 * Sets the current channel value for the specified index
 * @param index the index of the channel to set
 * @param value the value to set
 */
void CAN_set_current_channel_value(int index, float value);

/**
 * Apply the CAN message to the current list of of CAN channel mappings.
 * @param msg the CAN message containing the raw data
 * @param cfg the CAN channel configuration, containing the mappings
 * @param enabled_mapping_count the number of channel mappings
 */
void update_can_channels(CAN_msg *msg, uint8_t can_bus, CANChannelConfig *cfg, uint16_t enabled_mapping_count);

CPP_GUARD_END
#endif /* CAN_CHANNELS_H_ */
