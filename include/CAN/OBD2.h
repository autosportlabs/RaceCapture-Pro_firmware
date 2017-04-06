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
#ifndef OBD2_H_
#define OBD2_H_

#include "cpp_guard.h"
#include "CAN.h"
#include "stddef.h"

CPP_GUARD_BEGIN

#define OBD2_PID_DEFAULT_TIMEOUT_MS 300
#define OBD2_PID_REQUEST_TIMEOUT_MS 10

/**
 * Call to flag that the OBD2 state is stale
 */
void OBD2_state_stale(void);

/**
 * Indicates if the OBD2 state is stale
 * @return true if the state is stale
 */
bool OBD2_is_state_stale(void);

/**
 * Initialize the current values based on the provided OBD2 config
 * @param obd2_config the obd2 configuration used for initialization
 */
bool OBD2_init_current_values(OBD2Config *obd2_config);

/**
 * Get the current OBD2 channel value for the index
 * @param index the channel index
 * @return the current value for the index
 */
float OBD2_get_current_channel_value(int index);

/**
 * Sets the current channel value for the specified index
 * @param index the channel index to set
 * @param value the value to set
 */
void OBD2_set_current_channel_value(int index, float value);

/**
 * Schedule the next OBD2 query, if there is one to schedule.
 * @param obd2_config the current OBDII configuration
 * @param enabled_obd2_pids_count the number of OBDII PIDs to process
 */
void sequence_next_obd2_query(OBD2Config * obd2_config, uint16_t enabled_obd2_pids_count);

/**
 * Update the current OBD2 channel values with the CAN message
 * @param msg the CAN message containing the PID response
 * @param cfg the OBD2 configuration containing the channel mapping
 */
void update_obd2_channels(CAN_msg *msg, OBD2Config *cfg);

/**
 * Get the current value matching the specified OBD2 PID.
 * Will return the value for the first matching PID
 * @param pid the pid for the value they want
 * @param value pointer to the value to populate, if the PID was found
 * @return true if the PID was found
 */
bool OBD2_get_value_for_pid(uint16_t pid, float *value);

/**
 * Sends an OBD2 PID request on the CAN bus.
 * @param pid the OBD2 PID to request
 * @param mode the OBD2 mode to request
 * @param timeout the timeout in ms for sending the OBD2 request
 */
int OBD2_request_PID(uint16_t pid, uint8_t mode, size_t timeout_ms);

CPP_GUARD_END

#endif /* OBD2_H_ */
