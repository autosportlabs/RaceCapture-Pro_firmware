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
#include "gps.h"
#include "jsmn.h"
#include "logger.h"
#include "loggerConfig.h"
#include "loggerTaskEx.h"
#include "printk.h"
#include "serial.h"
#include <stdbool.h>

#define DEFAULT_START_SPEED_KPH	40
#define DEFAULT_START_TIME_SEC	5
#define DEFAULT_STOP_SPEED_KPH	25
#define DEFAULT_STOP_TIME_SEC	10
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

        cfg->start.time = DEFAULT_START_TIME_SEC;
        cfg->start.speed = DEFAULT_START_SPEED_KPH;

        cfg->stop.time = DEFAULT_STOP_TIME_SEC;
        cfg->stop.speed = DEFAULT_STOP_SPEED_KPH;
}

static void get_speed_time(struct Serial* serial,
                           struct auto_logger_speed_time *alst,
                           const char* name,
                           const bool more)
{
        json_objStartString(serial, name);
        json_float(serial, "speed", alst->speed, 2, true);
        json_uint(serial, "time", alst->time, false);
        json_objEnd(serial, more);
}

void auto_logger_get_config(struct auto_logger_config* cfg,
                            struct Serial* serial,
                            const bool more)
{
        json_objStartString(serial, "autoLoggerCfg");
        json_bool(serial, "active", cfg->active, true);
        get_speed_time(serial, &cfg->start, "start", true);
        get_speed_time(serial, &cfg->stop, "stop", false);
        json_objEnd(serial, more);
}

static void set_speed_time(struct auto_logger_speed_time *alst,
                           const char* name,
                           const jsmntok_t* root)
{
        const jsmntok_t* tok = jsmn_find_node(root, name);
        if (!tok)
                return;

        jsmn_exists_set_val_float(tok, "speed", &alst->speed);
        jsmn_exists_set_val_int(tok, "time", &alst->time);
}


bool auto_logger_set_config(struct auto_logger_config* cfg,
                            const jsmntok_t *json)
{
        jsmn_exists_set_val_bool(json, "active", &cfg->active);
        set_speed_time(&cfg->start, "start", json);
        set_speed_time(&cfg->stop, "stop", json);
        return true;
}

bool auto_logger_init(struct auto_logger_config* cfg)
{
        if (!cfg)
                return false;

        auto_logger_state.cfg = cfg;
        auto_logger_state.logging = logging_is_active();
        auto_logger_state.timestamp_start = 0;
        auto_logger_state.timestamp_stop = 0;
        return true;
}

static bool should_start_logging(const GpsSample* sample,
                                 const tiny_millis_t uptime)
{
        const tiny_millis_t trig_time =
                (tiny_millis_t) auto_logger_state.cfg->start.time * 1000;
        const float trig_speed = auto_logger_state.cfg->start.speed;

        if (0 == trig_time)
                return false;

        if (sample->speed < trig_speed) {
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

static bool should_stop_logging(const GpsSample* sample,
                                const tiny_millis_t uptime)
{
        const tiny_millis_t trig_time =
                (tiny_millis_t) auto_logger_state.cfg->stop.time * 1000;
        const float trig_speed = auto_logger_state.cfg->stop.speed;

        if (0 == trig_time)
                return false;

        if (sample->speed > trig_speed) {
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

void auto_logger_gps_sample_cb(const GpsSample* sample)
{
        if (!auto_logger_state.cfg || !auto_logger_state.cfg->active)
                return;

        const tiny_millis_t uptime = getUptime();
        if (!auto_logger_state.logging) {
                if (!should_start_logging(sample, uptime))
                        return;

                pr_info(LOG_PFX "Starting logging\r\n");
                startLogging();
                auto_logger_state.logging = true;
        } else {
                if (!should_stop_logging(sample, uptime))
                        return;

                pr_info(LOG_PFX "Stopping logging\r\n");
                stopLogging();
                auto_logger_state.logging = false;
        }
}
