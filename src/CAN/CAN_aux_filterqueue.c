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
#include "CAN_aux_filterqueue.h"
#include "capabilities.h"
#include "printk.h"
#include "taskUtil.h"


#define _LOG_PFX "[CAN AUX FILTERQUEUE] "
static xQueueHandle can_aux_filterqueue = NULL;
static uint8_t can_bus = 0;
static uint32_t low_id_range = 0;
static uint32_t high_id_range = 0;

bool CAN_aux_filterqueue_init(void)
{
        can_aux_filterqueue = xQueueCreate(CAN_AUX_FILTERQUEUE_LENGTH, sizeof(CAN_msg));
        if (! can_aux_filterqueue) {
                pr_error_int_msg(_LOG_PFX "Failed to alloc CAN aux filterqueue with size ", CAN_AUX_FILTERQUEUE_LENGTH);
                return false;
        }
        return true;
}

void CAN_aux_filterqueue_configure(uint8_t new_can_bus, uint32_t new_low_id_range, uint32_t new_high_id_range)
{
        can_bus = new_can_bus;
        low_id_range = new_low_id_range;
        high_id_range = new_high_id_range;
}

bool CAN_aux_filterqueue_put_msg(CAN_msg * can_msg)
{
        /* match on configured CAN bus */
        if (can_msg->can_bus != can_bus)
                return false;

        /* check if in range and if filter is enabled (low and high range > 0)*/
        if (!(can_msg->addressValue >= low_id_range && can_msg->addressValue <= high_id_range))
                return false;

        /* add to queue with no delay */
        if (pdTRUE == xQueueSend(can_aux_filterqueue, can_msg, 0))
                return true;
        return false;
}

bool CAN_aux_filterqueue_get_msg(CAN_msg * can_msg, size_t timeout_ms)
{
        if (pdTRUE == xQueueReceive(can_aux_filterqueue, can_msg, msToTicks(timeout_ms)))
                return true;

        return false;
}
