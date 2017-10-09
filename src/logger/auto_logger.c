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

#include "api.h"
#include "auto_logger.h"
#include "cpp_guard.h"
#include "dateTime.h"
#include "loggerTaskEx.h"
#include "printk.h"
#include <stdbool.h>
#include "loggerSampleData.h"
#include <string.h>

#define DEFAULT_AUTO_LOGGER_START_SPEED	40
#define DEFAULT_AUTO_LOGGER_START_TIME_SEC	5
#define DEFAULT_AUTO_LOGGER_STOP_SPEED	25
#define DEFAULT_AUTO_LOGGER_STOP_TIME_SEC	10
#define DEFAULT_AUTO_LOGGER_CHANNEL "Speed"
#define LOG_PFX			"[auto_logger] "

/*
 * NOTE:
 * We use the logging bool here because it helps us prevent contention
 * with manual user directives.  In example it will allow a user to start
 * logging using the manual button and not have the auto logger fight them
 * by shutting logging off right away.  Vice versa applies when coming off
 * the track.  It runs on the assumption that changing logging to a state
 * that it is already in is a no-op.
 */

static struct {
        struct auto_logger_config *cfg;
        bool logging;
        tiny_millis_t timestamp_start;
        tiny_millis_t timestamp_stop;
} auto_logger_state;

void auto_logger_reset_config(struct auto_logger_config* cfg)
{
        cfg->active = false;
        strcpy(cfg->channel, DEFAULT_AUTO_LOGGER_CHANNEL);

        cfg->start.time = DEFAULT_AUTO_LOGGER_START_TIME_SEC;
        cfg->start.threshold = DEFAULT_AUTO_LOGGER_START_SPEED;
        cfg->start.greater_than = true;

        cfg->stop.time = DEFAULT_AUTO_LOGGER_STOP_TIME_SEC;
        cfg->stop.threshold = DEFAULT_AUTO_LOGGER_STOP_SPEED;
        cfg->stop.greater_than = false;
}

static void get_speed_time(struct Serial* serial,
                           struct auto_logger_trigger *alst,
                           const char* name,
                           const bool more)
{
        json_objStartString(serial, name);
        json_float(serial, "thresh", alst->threshold, 2, true);
        json_bool(serial, "gt", alst->greater_than, true);
        json_uint(serial, "time", alst->time, false);
        json_objEnd(serial, more);
}

void auto_logger_get_config(struct auto_logger_config* cfg,
                            struct Serial* serial,
                            const bool more)
{
        json_objStartString(serial, "autoLoggerCfg");
        json_bool(serial, "active", cfg->active, true);
        json_string(serial, "channel", cfg->channel, true);
        get_speed_time(serial, &cfg->start, "start", true);
        get_speed_time(serial, &cfg->stop, "stop", false);
        json_objEnd(serial, more);
}

static void set_speed_time(struct auto_logger_trigger *alst,
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


bool auto_logger_set_config(struct auto_logger_config* cfg,
                            const jsmntok_t *json)
{
        jsmn_exists_set_val_bool(json, "active", &cfg->active);
        jsmn_exists_set_val_string(json, "channel", cfg->channel, DEFAULT_LABEL_LENGTH, true);
        set_speed_time(&cfg->start, "start", json);
        set_speed_time(&cfg->stop, "stop", json);
        return true;
}

static bool should_start_logging(const float current_value,
                                 const tiny_millis_t uptime)
{
        const tiny_millis_t trig_time =
                (tiny_millis_t) auto_logger_state.cfg->start.time * 1000;
        const float threshold = auto_logger_state.cfg->start.threshold;
        const bool gt = auto_logger_state.cfg->start.greater_than;

        if (0 == trig_time)
                return false;

        if ((gt && current_value < threshold) || (!gt && current_value > threshold)) {
          auto_logger_state.timestamp_start = 0;
                return false;
        }

        if (0 == auto_logger_state.timestamp_start) {
                auto_logger_state.timestamp_start = uptime;
                return false;
        }

        const tiny_millis_t time_diff =
                uptime - auto_logger_state.timestamp_start;
        return time_diff > trig_time;
}

static bool should_stop_logging(const float current_value,
                                const tiny_millis_t uptime)
{
        const tiny_millis_t trig_time =
                (tiny_millis_t) auto_logger_state.cfg->stop.time * 1000;
        const float threshold = auto_logger_state.cfg->stop.threshold;
        const bool gt = auto_logger_state.cfg->stop.greater_than;

        if (0 == trig_time)
                return false;

        if ((gt && current_value < threshold) || (!gt && current_value > threshold)) {
          auto_logger_state.timestamp_stop = 0;
                return false;
        }

        if (0 == auto_logger_state.timestamp_stop) {
                auto_logger_state.timestamp_stop = uptime;
                return false;
        }

        const tiny_millis_t time_diff =
                uptime - auto_logger_state.timestamp_stop;
        return time_diff > trig_time;
}

static void auto_logger_sample_cb(const struct sample* sample,
                           const int tick, void* data)
{
        if (!auto_logger_state.cfg || !auto_logger_state.cfg->active)
                return;

        double value;
        if (!get_sample_value_by_name(sample, auto_logger_state.cfg->channel, &value))
                return;

        const tiny_millis_t uptime = getUptime();
        if (!auto_logger_state.logging) {
                if (!should_start_logging(value, uptime))
                        return;

                pr_info(LOG_PFX "Auto-starting logging\r\n");
                startLogging();
                auto_logger_state.logging = true;
        } else {
                if (!should_stop_logging(value, uptime))
                        return;

                pr_info(LOG_PFX "Auto-stopping logging\r\n");
                stopLogging();
                auto_logger_state.logging = false;
        }
}

bool auto_logger_init(struct auto_logger_config* cfg)
{
        if (!cfg)
                return false;

        auto_logger_state.cfg = cfg;
        auto_logger_state.logging = false;
        auto_logger_state.timestamp_start = 0;
        auto_logger_state.timestamp_stop = 0;

        logger_sample_create_callback(auto_logger_sample_cb, 10, NULL);
        return true;
}
