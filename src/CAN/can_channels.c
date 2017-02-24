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
#include <string.h>

/* CAN bus channels current channel values */
static float * CAN_current_values = NULL;

bool CAN_init_current_values(size_t values) {
        if (CAN_current_values != NULL)
                portFree(CAN_current_values);

        values = MAX(1, values);
        size_t size = sizeof(float[values]);
        CAN_current_values = portMalloc(size);

        if (CAN_current_values != NULL)
                memset(CAN_current_values, 0, size);

        return CAN_current_values != NULL;
}

float CAN_get_current_channel_value(int index) {
        if (CAN_current_values == NULL)
                return 0;
        return CAN_current_values[index];
}

void CAN_set_current_channel_value(int index, float value)
{
        if (CAN_current_values == NULL)
                return;
        CAN_current_values[index] = value;
}

void update_can_channels(CAN_msg *msg, uint8_t can_bus, CANChannelConfig *cfg, uint16_t enabled_mapping_count)
{
        for (size_t i = 0; i < enabled_mapping_count; i++) {
                CANMapping *mapping = &cfg->can_channels[i].mapping;
                /* only process the mapping for the bus we're handling messages for */
                if (can_bus != mapping->can_channel)
                        continue;

                float value;
                /* map the CAN message to the value */
                bool result = canmapping_map_value(&value, msg, mapping);
                if (!result)
                        continue;

                CAN_set_current_channel_value(i, value);
                }
}
