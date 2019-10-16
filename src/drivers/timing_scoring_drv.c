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
#include "printk.h"
#define _LOG_PFX "[TimingScoring] "

#define TIMING_SCORING_CAN_BUS 1
#define TIMING_SCORING_DEFAULT_CAN_TIMEOUT   100
#define TIMING_SCORING_CAN_ID 0xA200

static TimingScoringState timing_scoring_state = {0};

static void _timing_scoring_broadcast_can(void)
{
        {
                CAN_msg msg;
                msg.data16[0] = (uint16_t)(timing_scoring_get_gap_to_ahead() * 10);
                msg.data16[1] = (uint16_t)(timing_scoring_get_car_number_ahead());
                msg.data16[2] = (uint16_t)(timing_scoring_get_gap_to_behind() * 10);
                msg.data16[3] = (uint16_t)(timing_scoring_get_car_number_behind());
                msg.addressValue = TIMING_SCORING_CAN_ID;
                msg.isExtendedAddress = true;
                msg.dataLength = 8;
                if (! CAN_tx_msg(TIMING_SCORING_CAN_BUS, &msg, TIMING_SCORING_DEFAULT_CAN_TIMEOUT)) {
                        pr_warning(_LOG_PFX "Timed out sending timing&scoring CAN message 1\r\n");
                }
        }

        {
                CAN_msg msg;
                msg.data16[0] = (uint16_t)(timing_scoring_get_tns_laptime() * 2000);
                msg.data[2] = (uint8_t)(timing_scoring_get_full_course_status());
                msg.data[3] = (uint8_t)(timing_scoring_get_position_in_class());
                msg.data[4] = 0;
                msg.data[5] = (uint8_t)(timing_scoring_get_black_flag());
                msg.addressValue = TIMING_SCORING_CAN_ID + 1;
                msg.isExtendedAddress = true;
                msg.dataLength = 6;
                if (! CAN_tx_msg(TIMING_SCORING_CAN_BUS, &msg, TIMING_SCORING_DEFAULT_CAN_TIMEOUT)) {
                        pr_warning(_LOG_PFX "Timed out sending timing&scoring CAN message 2\r\n");
                }
        }

        {
                CAN_msg msg;
                msg.data32[0] = (uint32_t)(timing_scoring_get_driver_id());
                msg.addressValue = TIMING_SCORING_CAN_ID + 2;
                msg.isExtendedAddress = true;
                msg.dataLength = 4;
                if (! CAN_tx_msg(TIMING_SCORING_CAN_BUS, &msg, TIMING_SCORING_DEFAULT_CAN_TIMEOUT)) {
                        pr_warning(_LOG_PFX "Timed out sending timing&scoring CAN message 3\r\n");
                }
        }
}

void timing_scoring_reset_config(TimingScoringConfig * cfg)
{
        *cfg = (TimingScoringConfig) DEFAULT_TIMING_SCORING_CONFIG;
}

TimingScoringState * timing_scoring_get_state(void)
{
        return &timing_scoring_state;
}

void timing_scoring_update(const jsmntok_t *json)
{
        TimingScoringState *ts = timing_scoring_get_state();
        jsmn_exists_set_val_int(json, "driverId", &ts->driver_id);
        jsmn_exists_set_val_int(json, "posInCls", &ts->position_in_class);
        jsmn_exists_set_val_int(json, "carNumAhead", &ts->car_number_ahead);
        jsmn_exists_set_val_float(json, "gapToAhead", &ts->gap_to_ahead);
        jsmn_exists_set_val_int(json, "carNumBehind", &ts->car_number_behind);
        jsmn_exists_set_val_float(json, "gapToBehind", &ts->gap_to_behind);
        jsmn_exists_set_val_float(json, "tnsLaptime", &ts->tns_laptime);
        jsmn_exists_set_val_int(json, "fcFlag", &ts->full_course_flag_status);
        jsmn_exists_set_val_bool(json, "blackFlag", &ts->black_flag);

        _timing_scoring_broadcast_can();
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
