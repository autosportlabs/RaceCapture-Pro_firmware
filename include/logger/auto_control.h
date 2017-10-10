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

void auto_control_reset_trigger(struct auto_control_trigger *start, struct auto_control_trigger *stop);

void auto_control_init_state(struct auto_control_state *state);

void get_auto_control_trigger(struct Serial* serial,
                           struct auto_control_trigger *alst,
                           const char* name,
                           const bool more);

void set_auto_control_trigger(struct auto_control_trigger *alst,
                           const char* name,
                           const jsmntok_t* root);

bool auto_control_should_start(const float current_value,
                                 const tiny_millis_t uptime,
                                 const struct auto_control_trigger * auto_trigger,
                                 struct auto_control_state * auto_state);

bool auto_control_should_stop(const float current_value,
                                const tiny_millis_t uptime,
                                const struct auto_control_trigger * auto_trigger,
                                struct auto_control_state * auto_state);

enum auto_control_trigger_result auto_control_check_trigger(float value,
                                                            struct auto_control_trigger *start,
                                                            struct auto_control_trigger *stop,
                                                            struct auto_control_state *state);

CPP_GUARD_END

#endif /* _AUTO_CONTROL_H_ */
