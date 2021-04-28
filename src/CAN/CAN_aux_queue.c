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
#include "CAN_aux_queue.h"
#include "capabilities.h"
#include "printk.h"
#include "taskUtil.h"


#define _LOG_PFX "[CAN AUX QUEUE] "
static xQueueHandle can_aux_queue[CAN_CHANNELS] = {0};

bool CAN_aux_queue_init(void)
{
        for (size_t i = 0; i < CAN_CHANNELS; i++) {
                can_aux_queue[i] = xQueueCreate(CAN_AUX_QUEUE_LENGTH, sizeof(CAN_msg));
                if (! can_aux_queue[i]) {
                        pr_error_int_msg(_LOG_PFX "Failed to alloc CAN aux queue with size ", CAN_AUX_QUEUE_LENGTH);
                        return false;
                }
        }
        return true;
}

bool CAN_aux_queue_put_msg(CAN_msg * can_msg)
{
        uint8_t can_bus = can_msg->can_bus;
        if (can_bus >= CAN_CHANNELS)
                return false;

        /* add to queue with no delay */
        if (pdTRUE == xQueueSend(can_aux_queue[can_bus], can_msg, 0))
                return true;
        return false;
}

bool CAN_aux_queue_get_msg(uint8_t can_bus, CAN_msg * can_msg, size_t timeout_ms)
{
        if (pdTRUE == xQueueReceive(can_aux_queue[can_bus], can_msg, msToTicks(timeout_ms)))
                return true;
        return false;
}
