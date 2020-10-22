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
#include "can_channels.h"
#include "loggerConfig.h"
#include "can_mapping.h"
#include "mem_mang.h"
#include "stdutil.h"
#include "printk.h"
#include <string.h>


/* manages the running state of the CAN channels*/
struct CANState {
        /* CAN bus channels current channel values */
        float * CAN_current_values;

        /* flag to indicate if state is stale */
        bool stale;
};

static struct CANState can_state = {0};

void CAN_state_stale(void)
{
        can_state.stale = true;
}

bool CAN_is_state_stale(void)
{
        return can_state.stale;
}

bool CAN_init_current_values(size_t values)
{
        if (can_state.CAN_current_values != NULL)
                portFree(can_state.CAN_current_values);

        values = MAX(1, values);
        size_t size = sizeof(float[values]);
        can_state.CAN_current_values = portMalloc(size);

        if (can_state.CAN_current_values != NULL)
                memset(can_state.CAN_current_values, 0, size);

        can_state.stale = false;
        return can_state.CAN_current_values != NULL;
}

float CAN_get_current_channel_value(int index)
{
        if (can_state.CAN_current_values == NULL)
                return 0;
        return can_state.CAN_current_values[index];
}

void CAN_set_current_channel_value(int index, float value)
{
        if (can_state.CAN_current_values == NULL)
                return;
        can_state.CAN_current_values[index] = value;
}

void update_can_channels(CAN_msg *msg, CANChannelConfig *cfg, uint16_t enabled_mapping_count)
{
        for (size_t i = 0; i < enabled_mapping_count; i++) {
                CANMapping *mapping = &cfg->can_channels[i].mapping;

                /* only process the mapping for the bus we're handling messages for */
                if (msg->can_bus != mapping->can_channel)
                        continue;

                float value;
                /* map the CAN message to the value */
                bool result = canmapping_map_value(&value, msg, mapping);
                if (!result)
                        continue;

                CAN_set_current_channel_value(i, value);
        }
}
