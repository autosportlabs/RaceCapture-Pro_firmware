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
#include "auto_control.h"
#include "api.h"

void auto_control_reset_trigger(struct auto_control_trigger *start, struct auto_control_trigger *stop)
{
        start->time = DEFAULT_AUTO_CONTROL_START_TIME_SEC;
        start->threshold = DEFAULT_AUTO_CONTROL_START_THRESHOLD;
        start->greater_than = true;

        stop->time = DEFAULT_AUTO_CONTROL_STOP_TIME_SEC;
        stop->threshold = DEFAULT_AUTO_CONTROL_STOP_THRESHOLD;
        stop->greater_than = false;
}

void auto_control_init_state(struct auto_control_state *state)
{
        state->triggered = false;
        state->timestamp_start = 0;
        state->timestamp_stop = 0;
}

void get_auto_control_trigger(struct Serial* serial,
                           struct auto_control_trigger *alst,
                           const char* name,
                           const bool more)
{
        json_objStartString(serial, name);
        json_float(serial, "thresh", alst->threshold, 2, true);
        json_bool(serial, "gt", alst->greater_than, true);
        json_uint(serial, "time", alst->time, false);
        json_objEnd(serial, more);
}

void set_auto_control_trigger(struct auto_control_trigger *alst,
                           const char* name,
                           const jsmntok_t* root)
{
        const jsmntok_t* tok = jsmn_find_node(root, name);
        if (!tok)
                return;

        jsmn_exists_set_val_float(tok, "thresh", &alst->threshold);
        jsmn_exists_set_val_bool(tok, "gt", &alst->greater_than);
        jsmn_exists_set_val_int(tok, "time", &alst->time);
}

bool auto_control_should_start(const float current_value,
                                 const tiny_millis_t uptime,
                                 const struct auto_control_trigger * auto_trigger,
                                 struct auto_control_state * auto_state)
{
        const tiny_millis_t trig_time =
                (tiny_millis_t) auto_trigger->time * 1000;
        const float threshold = auto_trigger->threshold;
        const bool gt = auto_trigger->greater_than;

        if (0 == trig_time)
                return false;

        if ((gt && current_value < threshold) || (!gt && current_value > threshold)) {
          auto_state->timestamp_start = 0;
                return false;
        }

        if (0 == auto_state->timestamp_start) {
                auto_state->timestamp_start = uptime;
                return false;
        }

        const tiny_millis_t time_diff =
                uptime - auto_state->timestamp_start;
        return time_diff > trig_time;
}

bool auto_control_should_stop(const float current_value,
                                const tiny_millis_t uptime,
                                const struct auto_control_trigger * auto_trigger,
                                struct auto_control_state * auto_state)
{
        const tiny_millis_t trig_time =
                (tiny_millis_t) auto_trigger->time * 1000;
        const float threshold = auto_trigger->threshold;
        const bool gt = auto_trigger->greater_than;

        if (0 == trig_time)
                return false;

        if ((gt && current_value < threshold) || (!gt && current_value > threshold)) {
          auto_state->timestamp_stop = 0;
                return false;
        }

        if (0 == auto_state->timestamp_stop) {
                auto_state->timestamp_stop = uptime;
                return false;
        }

        const tiny_millis_t time_diff =
                uptime - auto_state->timestamp_stop;
        return time_diff > trig_time;
}

enum auto_control_trigger_result auto_control_check_trigger(float value,
                                                            struct auto_control_trigger *start,
                                                            struct auto_control_trigger *stop,
                                                            struct auto_control_state *state)
{
        const tiny_millis_t uptime = getUptime();
        if (!state->triggered) {
                if (!auto_control_should_start(value, uptime, start, state))
                        return AUTO_CONTROL_NO_ACTION;

                state->triggered = true;
                return AUTO_CONTROL_TRIGGERED;
        } else {
                if (!auto_control_should_stop(value, uptime, stop, state))
                        return AUTO_CONTROL_NO_ACTION;

                state->triggered = false;
                return AUTO_CONTROL_UNTRIGGERED;
        }
}
