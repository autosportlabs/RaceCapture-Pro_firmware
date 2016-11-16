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
#include "CANMap.h"
#include "loggerConfig.h"
#include "printk.h"
#include "task.h"
#include "taskUtil.h"

static int CANMap_current_values[CAN_MAP_CHANNELS];

void CANMap_set_current_map_value(size_t index, int value)
{
    if (index < CAN_MAP_CHANNELS) {
        CANMap_current_values[index] = value;
    }
}

int CANMap_get_current_map_value(int index)
{
    if (index < CAN_MAP_CHANNELS) {
        return CANMap_current_values[index];
    }
    else {
        return 0;
    }
}

bool CAN_process_map(CANMapConfig *cMapConf, CAN_msg *msg, int *value)
{
    bool result = false;
    *value = 0; // Reset this to 0

    for (size_t i = 0; i < cMapConf->enabledChannels; i++) {
        CANMapChannelConfig *chanCfg = &cMapConf->maps[i];

        if ((msg->addressValue & chanCfg->canIdMask) == chanCfg->canId) {
            // We only have enough space for an int (4 bytes)
            if ((chanCfg->byteStart + chanCfg->byteLength) > 4)
            {
                return false;
            }

            for (size_t i = 0; i < chanCfg->byteLength; i++) {
                *value = (msg->data[chanCfg->byteStart + i] << (8 * i));
            }

            if (chanCfg->dataMask) {
                *value = *value & (int)chanCfg->dataMask;
            }

            if (chanCfg->multipler) {
                *value = *value * (int)chanCfg->multipler;
            }

            if (chanCfg->divisor) {
                *value = *value / (int)chanCfg->divisor;
            }

            if (chanCfg->adder) {
                *value = *value + (int)chanCfg->adder;
            }

            CANMap_set_current_map_value(i, *value);

            if (TRACE_LEVEL) {
                pr_trace("read CAN ");
                pr_trace_int(i);
                pr_trace(":");
                pr_trace(chanCfg->cfg.label);
                pr_trace("=")
                pr_trace_int(*value);
                pr_trace("\r\n");
            }

            result = true;
        }
    }       

    return result;
}