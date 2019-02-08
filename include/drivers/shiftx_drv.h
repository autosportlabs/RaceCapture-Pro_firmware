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

#ifndef _SHIFTX_DRV_H_
#define _SHIFTX_DRV_H_

#include "CAN.h"

CPP_GUARD_BEGIN

struct shiftx_configuration {
        uint8_t orientation_inverted;
        uint8_t brightness;
        uint32_t can_bus;
        uint32_t base_address;
        uint8_t auto_brightness_scaling;
};

struct shiftx_configuration * shiftx_get_config(void);
bool shiftx_update_config(void);

bool shiftx_set_discrete_led(uint8_t led_index, uint8_t leds_to_set, uint8_t red, uint8_t green, uint8_t blue, uint8_t flash);
bool shiftx_set_display(uint8_t digit_index, uint8_t ascii);

bool shiftx_config_linear_graph(uint8_t rendering_style, uint8_t linear_style, uint16_t low_range, uint16_t high_range);
bool shiftx_set_linear_threshold(uint8_t threshold_id, uint8_t segment_length, uint16_t threshold, uint8_t red, uint8_t green, uint8_t blue, uint8_t flash);
bool shiftx_update_linear_graph(uint16_t value);

bool shiftx_set_alert_threshold(uint8_t alert_id, uint8_t threshold_id, uint16_t threshold, uint8_t red, uint8_t green, uint8_t blue, uint8_t flash);
bool shiftx_set_alert(uint8_t alert_id, uint8_t red, uint8_t green, uint8_t blue, uint8_t flash);
bool shiftx_update_alert(uint8_t alert_id, uint16_t value);

CPP_GUARD_END

#endif /* _SHIFTX_DRV_H_ */
