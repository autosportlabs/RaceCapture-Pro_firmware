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

/**
 * Process an incoming CAN message
 * @param msg the CAN message to process
 */
void shiftx_handle_can_rx_msg(CAN_msg *msg);

/**
 * Retreive a pointer to the current runtime configuration
 * @return pointer to struct of the shiftx_configuration
 */
struct shiftx_configuration * shiftx_get_config(void);


/**
 * Send the configuration update message to the connected ShiftX device, using the current runtime configurationi
 */
bool shiftx_update_config(void);

/**
 * Set a discrete LED on the ShiftX device
 * @param led_index the index of the led to set
 * @param leds_to_set number of LEDs to set. 0 = set all remaining LEDs
 * @param red red color value
 * @param green green color value
 * @param blue blue color value
 * @param flash flash rate, in Hz
 * @return true if CAN message was successfully broadcasted
 */
bool shiftx_set_discrete_led(uint8_t led_index, uint8_t leds_to_set, uint8_t red, uint8_t green, uint8_t blue, uint8_t flash);

/**
 * Set the display value
 * @param digit_index index of digit to set
 * @param ascii ascii value to set (48 = '0', 49 = '1')
 * @return true if CAN message was successfully broadcasted
 */
bool shiftx_set_display(uint8_t digit_index, uint8_t ascii);

/**
 * Configures the linear graph on the ShiftX device
 * @param rendering_style 0=left->right; 1=center; 2=right->left
 * @param linear_style 0=smooth/interpolated; 1=stepped
 * @param low_range low value range of linear display
 * @param high_range high value range of linear display (ignored if rendering_style = stepped)
 * @return true if CAN message was successfully broadcasted
 */
bool shiftx_config_linear_graph(uint8_t rendering_style, uint8_t linear_style, uint16_t low_range, uint16_t high_range);

/**
 * Configures a threshold for the linear graph on the ShiftX device
 * @param threshold_id id of threshold (0-4)
 * @param segment_length number of segments to display for this threshold (ignored if linear style = smooth)
 * @param threshold value for this threshold
 * @param red red color value
 * @param green green color value
 * @param blue blue color value
 * @param flash flash rate, in Hz
 * @return true if CAN message was successfully broadcasted
 */
bool shiftx_set_linear_threshold(uint8_t threshold_id, uint8_t segment_length, uint16_t threshold, uint8_t red, uint8_t green, uint8_t blue, uint8_t flash);

/**
 * Updates the current value for the linear graph
 * @param value the current runtime value
 * @return true if CAN message was successfully broadcasted
 */
bool shiftx_update_linear_graph(uint16_t value);

/**
 * Configures a threshold for an alert on the ShiftX device
 * @param alert_id id of alert (0-> number of alert indicators on device)
 * @param threshold_id id of threshold (0-4)
 * @param threshold value for this threshold
 * @param red red color value
 * @param green green color value
 * @param blue blue color value
 * @param flash flash rate, in Hz
 * @return true if CAN message was successfully broadcasted
 */
bool shiftx_set_alert_threshold(uint8_t alert_id, uint8_t threshold_id, uint16_t threshold, uint8_t red, uint8_t green, uint8_t blue, uint8_t flash);

/**
 * Directly set an alert indicator
 * @param alert_id id of alert (0-> number of alert indicators on device)
 * @param red red color value
 * @param green green color value
 * @param blue blue color value
 * @param flash flash rate, in Hz
 * @return true if CAN message was successfully broadcasted
 */
bool shiftx_set_alert(uint8_t alert_id, uint8_t red, uint8_t green, uint8_t blue, uint8_t flash);

/**
 * Updates the current value for the specified alert
 * @param alert_id id of alert (0-> number of alert indicators on device)
 * @param value the current runtime value
 * @return true if CAN message was successfully broadcasted
 */
bool shiftx_update_alert(uint8_t alert_id, uint16_t value);

/**
 * Check if a button has been pressed
 * @param button_id pointer to the ID of the button event (stored in passed-in parameter if button press was receive)
 * @param state pointer to state of the button event (1=down, 0 = up) (stored in passed-in parameter if button press was received)
 * @return true if a button press was received
 */
bool shiftx_rx_button_press(uint8_t * button_id, uint8_t * state);

CPP_GUARD_END

#endif /* _SHIFTX_DRV_H_ */
