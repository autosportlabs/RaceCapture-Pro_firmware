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

bool CAN_aux_filterqueue_init(void)
{
        return true;
}

void CAN_aux_filterqueue_configure(uint8_t new_can_bus, uint32_t new_low_id_range, uint32_t new_high_id_range)
{
}

bool CAN_aux_filterqueue_put_msg(CAN_msg * can_msg)
{
        return false;
}

bool CAN_aux_filterqueue_get_msg(CAN_msg * can_msg, size_t timeout_ms)
{
        return false;
}
