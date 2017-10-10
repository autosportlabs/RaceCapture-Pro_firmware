/*
 * Race Capture Firmware
 *
 * Copyright (C) 2017 Autosport Labs
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
#ifndef _AUTO_CONTROL_H_
#define _AUTO_CONTROL_H_

#include "cpp_guard.h"
#include "dateTime.h"
#include <stdbool.h>
#include "serial.h"
#include "jsmn.h"

CPP_GUARD_BEGIN

/**
 * Provides the generic logic for automatic control, triggered
 * based on a threshold value and time
 */
#define DEFAULT_AUTO_CONTROL_START_THRESHOLD 40
#define DEFAULT_AUTO_CONTROL_START_TIME_SEC 5
#define DEFAULT_AUTO_CONTROL_STOP_THRESHOLD 25
#define DEFAULT_AUTO_CONTROL_STOP_TIME_SEC 10

enum auto_control_trigger_result {
        AUTO_CONTROL_NO_ACTION,
        AUTO_CONTROL_TRIGGERED,
        AUTO_CONTROL_UNTRIGGERED
};

struct auto_control_trigger {
        float threshold;
        bool greater_than;
        uint32_t time;
};

struct auto_control_state {
        bool triggered;
        tiny_millis_t timestamp_start;
        tiny_millis_t timestamp_stop;
};

/**
 * Resets the trigger with default values.
 * @param start the start trigger
 * @return stop the stop trigger
 */
void auto_control_reset_trigger(struct auto_control_trigger *start, struct auto_control_trigger *stop);

/**
 * Sets the initial auto control state
 * @param the state to initialize
 */
void auto_control_init_state(struct auto_control_state *state);

/**
 * Serialize auto control trigger value to JSON
 * @param serial the Serial connection where the JSON will be output
 * @param alst the trigger to output
 * @param name the name of the trigger type
 * @param more there are more JSON segments
 */
void get_auto_control_trigger(struct Serial* serial,
                           struct auto_control_trigger *alst,
                           const char* name,
                           const bool more);

/**
 * De-serialize the auto control trigger value from JSON
 * @param alst the trigger to populate
 * @param name the name of the trigger parent (e.g. start or stop)
 * @param root the parent of the JSON object graph
 */
void set_auto_control_trigger(struct auto_control_trigger *alst,
                           const char* name,
                           const jsmntok_t* root);

/**
 * Tests if the trigger should start
 * @param value the current value to test
 * @param uptime the current uptime
 * @param auto_trigger the trigger configuration
 * @param auto_state the auto-control state
 * @return true if the control should trigger
 */
bool auto_control_should_start(const float current_value,
                                 const tiny_millis_t uptime,
                                 const struct auto_control_trigger * auto_trigger,
                                 struct auto_control_state * auto_state);

/**
 * Tests if the trigger should stop
 * @param value the current value to test
 * @param uptime the current uptime
 * @param auto_trigger the trigger configuration
 * @param auto_state the auto-control state
 * @return true if the control should de-trigger
 */
bool auto_control_should_stop(const float current_value,
                                const tiny_millis_t uptime,
                                const struct auto_control_trigger * auto_trigger,
                                struct auto_control_state * auto_state);


/**
 * @param value the value to check
 * @param start the start trigger
 * @param stop the stop trigger
 * @param the auto-control state
 * @param an enum indicating what action should be taken
 */
enum auto_control_trigger_result auto_control_check_trigger(float value,
                                                            struct auto_control_trigger *start,
                                                            struct auto_control_trigger *stop,
                                                            struct auto_control_state *state);

CPP_GUARD_END

#endif /* _AUTO_CONTROL_H_ */
