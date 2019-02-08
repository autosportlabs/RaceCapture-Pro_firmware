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

#include "shiftx_drv.h"
#include "CAN.h"

#define DEFAULT_CAN_TIMEOUT   100
#define CONFIG_MESSAGE_OFFSET 3

#define CONFIG_MESSAGE_SET_DISCRETE_LED_OFFSET 10

#define CONFIG_MESSAGE_SET_ALERT_OFFSET 20
#define CONFIG_MESSAGE_SET_ALERT_THRESHOLD_OFFSET 21
#define CONFIG_MESSAGE_UPDATE_ALERT_VALUE_OFFSET 22

#define CONFIG_MESSAGE_CONFIGURE_LINEAR_GRAPH_OFFSET 40
#define CONFIG_MESSAGE_SET_LINEAR_THRESHOLD_OFFSET 41
#define CONFIG_MESSAGE_UPDATE_LINEAR_GRAPH_VALUE_OFFSET 42

#define CONFIG_MESSAGE_SET_DISPLAY_OFFSET 50

static struct shiftx_configuration shiftx_config = {0, 0, 1, 0xE3600, 51};

struct shiftx_configuration * shiftx_get_config(void)
{
        return &shiftx_config;
}

bool shiftx_update_config(void)
{
        CAN_msg msg;
        msg.data[0] = shiftx_config.brightness;
        msg.data[1] = shiftx_config.auto_brightness_scaling;
        msg.data[2] = shiftx_config.orientation_inverted;
        msg.addressValue = shiftx_config.base_address + CONFIG_MESSAGE_OFFSET;
        msg.isExtendedAddress = true;
        msg.dataLength = 3;
        return CAN_tx_msg(shiftx_config.can_bus, &msg, DEFAULT_CAN_TIMEOUT);
}

bool shiftx_set_discrete_led(uint8_t led_index, uint8_t leds_to_set, uint8_t red, uint8_t green, uint8_t blue, uint8_t flash)
{
        CAN_msg msg;
        msg.data[0] = led_index;
        msg.data[1] = leds_to_set;
        msg.data[2] = red;
        msg.data[3] = green;
        msg.data[4] = blue;
        msg.data[5] = flash;
        msg.addressValue = shiftx_config.base_address + CONFIG_MESSAGE_SET_DISCRETE_LED_OFFSET;
        msg.isExtendedAddress = true;
        msg.dataLength = 6;
        return CAN_tx_msg(shiftx_config.can_bus, &msg, DEFAULT_CAN_TIMEOUT);
}

bool shiftx_set_display(uint8_t digit_index, uint8_t ascii)
{
        CAN_msg msg;
        msg.data[0] = digit_index;
        msg.data[1] = ascii;
        msg.addressValue = shiftx_config.base_address + CONFIG_MESSAGE_SET_DISPLAY_OFFSET;
        msg.isExtendedAddress = true;
        msg.dataLength = 2;
        return CAN_tx_msg(1, &msg, DEFAULT_CAN_TIMEOUT);
}

bool shiftx_config_linear_graph(uint8_t rendering_style, uint8_t linear_style, uint16_t low_range, uint16_t high_range)
{
        CAN_msg msg;
        msg.data[0] = rendering_style;
        msg.data[1] = linear_style;
        msg.data[2] = low_range & 0xFF;
        msg.data[3] = low_range >> 8;
        msg.data[4] = high_range & 0xFF;
        msg.data[5] = high_range >> 8;
        msg.addressValue = shiftx_config.base_address + CONFIG_MESSAGE_CONFIGURE_LINEAR_GRAPH_OFFSET;
        msg.isExtendedAddress = true;
        msg.dataLength = 6;
        return CAN_tx_msg(shiftx_config.can_bus, &msg, DEFAULT_CAN_TIMEOUT);
}

bool shiftx_set_linear_threshold(uint8_t threshold_id, uint8_t segment_length, uint16_t threshold, uint8_t red, uint8_t green, uint8_t blue, uint8_t flash)
{
        CAN_msg msg;
        msg.data[0] = threshold_id;
        msg.data[1] = segment_length;
        msg.data[2] = threshold & 0xFF;
        msg.data[3] = threshold >> 8;
        msg.data[4] = red;
        msg.data[5] = green;
        msg.data[6] = blue;
        msg.data[7] = flash;
        msg.addressValue = shiftx_config.base_address + CONFIG_MESSAGE_SET_LINEAR_THRESHOLD_OFFSET;
        msg.isExtendedAddress = true;
        msg.dataLength = 8;
        return CAN_tx_msg(shiftx_config.can_bus, &msg, DEFAULT_CAN_TIMEOUT);
}

bool shiftx_update_linear_graph(uint16_t value)
{
        CAN_msg msg;
        msg.data[0] = value & 0xFF;
        msg.data[1] = value >> 8;
        msg.addressValue = shiftx_config.base_address + CONFIG_MESSAGE_UPDATE_LINEAR_GRAPH_VALUE_OFFSET;
        msg.isExtendedAddress = true;
        msg.dataLength = 2;
        return CAN_tx_msg(shiftx_config.can_bus, &msg, DEFAULT_CAN_TIMEOUT);
}

bool shiftx_set_alert_threshold(uint8_t alert_id, uint8_t threshold_id, uint16_t threshold, uint8_t red, uint8_t green, uint8_t blue, uint8_t flash)
{
        CAN_msg msg;
        msg.data[0] = alert_id;
        msg.data[1] = threshold_id;
        msg.data[2] = threshold & 0xFF;
        msg.data[3] = threshold >> 8;
        msg.data[4] = red;
        msg.data[5] = green;
        msg.data[6] = blue;
        msg.data[7] = flash;
        msg.addressValue = shiftx_config.base_address + CONFIG_MESSAGE_SET_ALERT_THRESHOLD_OFFSET;
        msg.isExtendedAddress = true;
        msg.dataLength = 8;
        return CAN_tx_msg(shiftx_config.can_bus, &msg, DEFAULT_CAN_TIMEOUT);
}

bool shiftx_set_alert(uint8_t alert_id, uint8_t red, uint8_t green, uint8_t blue, uint8_t flash)
{
        CAN_msg msg;
        msg.data[0] = alert_id;
        msg.data[1] = red;
        msg.data[2] = green;
        msg.data[3] = blue;
        msg.data[4] = flash;
        msg.addressValue = shiftx_config.base_address + CONFIG_MESSAGE_SET_ALERT_OFFSET;
        msg.isExtendedAddress = true;
        msg.dataLength = 5;
        return CAN_tx_msg(shiftx_config.can_bus, &msg, DEFAULT_CAN_TIMEOUT);
}

bool shiftx_update_alert(uint8_t alert_id, uint16_t value)
{
        CAN_msg msg;
        msg.data[0] = alert_id;
        msg.data[1] = value & 0xFF;
        msg.data[2] = value >> 8;
        msg.addressValue = shiftx_config.base_address + CONFIG_MESSAGE_UPDATE_ALERT_VALUE_OFFSET;
        msg.isExtendedAddress = true;
        msg.dataLength = 3;
        return CAN_tx_msg(shiftx_config.can_bus, &msg, DEFAULT_CAN_TIMEOUT);
}

