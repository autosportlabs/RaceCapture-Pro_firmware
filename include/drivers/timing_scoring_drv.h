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
#include "channel_config.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

CPP_GUARD_BEGIN

#define DEFAULT_TIMING_SCORING_SAMPLE_RATE SAMPLE_1Hz
#define DEFAULT_COMPETITOR_NUMBER_AHEAD_CONFIG {"CompAhead", "", 0, 0, DEFAULT_TIMING_SCORING_SAMPLE_RATE, 0, 0}
#define DEFAULT_GAP_TO_AHEAD_CONFIG {"GapToAhead", "", 0, 0, DEFAULT_TIMING_SCORING_SAMPLE_RATE, 0, 0}
#define DEFAULT_COMPETITOR_NUMBER_BEHIND_CONFIG {"CompBehind", "", 0, 0, DEFAULT_TIMING_SCORING_SAMPLE_RATE, 0, 0}
#define DEFAULT_GAP_TO_BEHIND_CONFIG {"GapToBehind", "", 0, 0, DEFAULT_TIMING_SCORING_SAMPLE_RATE, 0, 0}
#define DEFAULT_TNS_LAPTIME_CONFIG {"TnSLaptime", "", 0, 0, DEFAULT_TIMING_SCORING_SAMPLE_RATE, 0, 0}
#define DEFAULT_DRIVER_ID_CONFIG {"DriverId", "", 0, 0, DEFAULT_TIMING_SCORING_SAMPLE_RATE, 0, 0}
#define DEFAULT_FLAG_STATUS_CONFIG {"FlagStatus", "", 0, 0, DEFAULT_TIMING_SCORING_SAMPLE_RATE, 0, 0}
#define DEFAULT_POSITION_IN_CLASS_CONFIG {"PosInClass", "", 0, 0, DEFAULT_TIMING_SCORING_SAMPLE_RATE, 0, 0}
#define DEFAULT_BLACK_FLAG_CONFIG {"BlackFlag", "", 0, 0, DEFAULT_TIMING_SCORING_SAMPLE_RATE, 0, 0}

#define DEFAULT_TIMING_SCORING_CONFIG {                         \
                DEFAULT_COMPETITOR_NUMBER_AHEAD_CONFIG,        \
                DEFAULT_GAP_TO_AHEAD_CONFIG,                   \
                DEFAULT_COMPETITOR_NUMBER_BEHIND_CONFIG,       \
                DEFAULT_GAP_TO_BEHIND_CONFIG,                  \
                DEFAULT_TNS_LAPTIME_CONFIG,                    \
                DEFAULT_DRIVER_ID_CONFIG,                      \
                DEFAULT_FLAG_STATUS_CONFIG,             \
                DEFAULT_POSITION_IN_CLASS_CONFIG,              \
                DEFAULT_BLACK_FLAG_CONFIG                      \
                        }

typedef struct _TimingScoringConfig {
        ChannelConfig driver_id;
        ChannelConfig position_in_class;
        ChannelConfig car_number_ahead;
        ChannelConfig gap_to_ahead;
        ChannelConfig car_number_behind;
        ChannelConfig gap_to_behind;
        ChannelConfig tns_laptime;
        ChannelConfig full_course_status;
        ChannelConfig black_flag;
} TimingScoringConfig;

typedef struct _TimingScoringState {
        uint16_t car_number_ahead;
        uint16_t car_number_behind;
        float gap_to_ahead;
        float gap_to_behind;

        float tns_laptime;
        uint32_t driver_id;
        uint8_t full_course_flag_status;
        uint8_t position_in_class;
        bool black_flag;
} TimingScoringState;

void timing_scoring_reset_config(TimingScoringConfig * cfg);

TimingScoringState * timing_scoring_get_state(void);

uint32_t timing_scoring_get_driver_id(void);
uint8_t timing_scoring_get_position_in_class(void);
uint16_t timing_scoring_get_car_number_ahead(void);
float timing_scoring_get_gap_to_ahead(void);
uint16_t timing_scoring_get_car_number_behind(void);
float timing_scoring_get_gap_to_behind(void);
float timing_scoring_get_tns_laptime(void);
uint8_t timing_scoring_get_full_course_status(void);
bool timing_scoring_get_black_flag(void);

CPP_GUARD_END

#endif /* _TIMING_SCORING_DRV_H_ */
