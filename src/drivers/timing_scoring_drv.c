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

#include "timing_scoring_drv.h"
#include "CAN.h"
#define _LOG_PFX "[TimingScoring] "

#define DEFAULT_CAN_TIMEOUT   100

static TimingScoringState timing_scoring_state = {0};

void timing_scoring_reset_config(TimingScoringConfig * cfg)
{
        *cfg = (TimingScoringConfig) DEFAULT_TIMING_SCORING_CONFIG;
}

TimingScoringState * timing_scoring_get_state(void)
{
        return &timing_scoring_state;
}

int timing_scoring_get_driver_id(void)
{
        return timing_scoring_state.driver_id;
}

int timing_scoring_get_position_in_class(void)
{
        return timing_scoring_state.position_in_class;
}

int timing_scoring_get_car_number_ahead(void)
{
        return timing_scoring_state.car_number_ahead;
}

float timing_scoring_get_gap_to_ahead(void)
{
        return timing_scoring_state.gap_to_ahead;
}

int timing_scoring_get_car_number_behind(void)
{
        return timing_scoring_state.car_number_behind;
}

float timing_scoring_get_gap_to_behind(void)
{
        return timing_scoring_state.gap_to_behind;
}

float timing_scoring_get_tns_laptime(void)
{
        return timing_scoring_state.tns_laptime;
}

int timing_scoring_get_full_course_status(void)
{
        return timing_scoring_state.full_course_flag_status;
}

bool timing_scoring_get_black_flag(void)
{
        return timing_scoring_state.black_flag;
}
