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
#include "convert.h"
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
#include <string.h>

#define DEFAULT_START_SPEED_MPH	25
#define DEFAULT_START_TIME_SEC	5
#define DEFAULT_STOP_SPEED_MPH	3
#define DEFAULT_STOP_TIME_SEC	30
#define LOG_PFX			"[auto_logger] "

static struct {
        struct auto_logger_config *cfg;
        tiny_millis_t timestamp_start;
        tiny_millis_t timestamp_stop;
} auto_logger_state;

void auto_logger_reset_config(struct auto_logger_config* cfg)
{
        memset(cfg, 0, sizeof(*cfg));

        cfg->time_start = DEFAULT_START_TIME_SEC;
        cfg->speed_start = DEFAULT_START_SPEED_MPH;

        cfg->time_stop = DEFAULT_STOP_TIME_SEC;
        cfg->speed_stop = DEFAULT_STOP_SPEED_MPH;
}

void auto_logger_get_config(struct auto_logger_config* cfg,
                            struct Serial* serial,
                            const bool more)
{
        json_objStartString(serial, "autoLoggerCfg");
        json_bool(serial, "active", cfg->active, true);

        json_objStartString(serial, "time");
        json_uint(serial, "start", cfg->time_start, true);
        json_uint(serial, "stop", cfg->time_stop, false);
        json_objEnd(serial, true);

        json_objStartString(serial, "speed");
        json_float(serial, "start", cfg->speed_start, 2, true);
        json_float(serial, "stop", cfg->speed_stop, 2, false);
        json_objEnd(serial, false);

        json_objEnd(serial, more);
}

bool auto_logger_set_config(struct auto_logger_config* cfg,
                            const jsmntok_t *json)
{
        const jsmntok_t* tok;
        jsmn_exists_set_val_bool(json, "active", &cfg->active);

        tok = jsmn_find_node(json, "time");
        if (tok) {
                jsmn_exists_set_val_int(tok, "start", &cfg->time_start);
                jsmn_exists_set_val_int(tok, "stop", &cfg->time_stop);
        }

        tok = jsmn_find_node(json, "speed");
        if (tok) {
                jsmn_exists_set_val_float(tok, "start", &cfg->speed_start);
                jsmn_exists_set_val_float(tok, "stop", &cfg->speed_stop);
        }

        return true;
}

bool auto_logger_init(struct auto_logger_config* cfg)
{
        if (!cfg)
                return false;

        auto_logger_state.cfg = cfg;
        auto_logger_state.timestamp_start = 0;
        auto_logger_state.timestamp_stop = 0;
        return true;
}

static bool should_start_logging(const GpsSample* sample,
                                 const tiny_millis_t uptime)
{
        const tiny_millis_t trig_time =
                (tiny_millis_t) auto_logger_state.cfg->time_start * 1000;
        const float trig_speed =
                convert_mph_kph(auto_logger_state.cfg->speed_start);

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
                (tiny_millis_t) auto_logger_state.cfg->time_stop * 1000;
        const float trig_speed =
                convert_mph_kph(auto_logger_state.cfg->speed_stop);

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
        if (!logging_is_active()) {
                if (!should_start_logging(sample, uptime))
                        return;

                pr_info(LOG_PFX "Starting logging\r\n");
                startLogging();
        } else {
                if (!should_stop_logging(sample, uptime))
                        return;

                pr_info(LOG_PFX "Stopping logging\r\n");
                stopLogging();
        }
}
