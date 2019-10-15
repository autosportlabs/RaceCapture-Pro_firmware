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

#ifndef _TIMING_SCORING_DRV_H_
#define _TIMING_SCORING_DRV_H_

#include "CAN.h"

CPP_GUARD_BEGIN

struct timing_scoring_event {
        uint16_t car_number_ahead;
        uint16_t car_number_behind;
        float gap_to_ahead;
        float gap_to_behind;

        float last_lap_time;
        uint32_t podium_driver_id;
        uint8_t full_course_status;
        uint8_t position_in_class;
        bool black_flag;
};

void update_timing_scoring(const struct timing_scoring_event * timing_scoring);

CPP_GUARD_END

#endif /* _TIMING_SCORING_DRV_H_ */
