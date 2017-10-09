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

#include "camera_control.h"
#include "cpp_guard.h"
#include "dateTime.h"
#include "api.h"
#include "printk.h"
#include <stdbool.h>
#include "wifi.h"
#include "loggerSampleData.h"

#define DEFAULT_START_SPEED_KPH	40
#define DEFAULT_START_TIME_SEC	5
#define DEFAULT_STOP_SPEED_KPH	25
#define DEFAULT_STOP_TIME_SEC	10
#define LOG_PFX			"[cam_ctrl] "

static struct {
        struct camera_control_config *cfg;
        bool recording;
        tiny_millis_t timestamp_start;
        tiny_millis_t timestamp_stop;
} camera_control_state;

void camera_control_reset_config(struct camera_control_config* cfg)
{
        cfg->active = false;
        cfg->make_model = CAMERA_MAKEMODEL_GOPRO_HERO2_3;
        cfg->start.time = DEFAULT_START_TIME_SEC;
        cfg->start.speed = DEFAULT_START_SPEED_KPH;

        cfg->stop.time = DEFAULT_STOP_TIME_SEC;
        cfg->stop.speed = DEFAULT_STOP_SPEED_KPH;
}

static void get_speed_time(struct Serial* serial,
                           struct camera_control_speed_time *ccst,
                           const char* name,
                           const bool more)
{
        json_objStartString(serial, name);
        json_float(serial, "speed", ccst->speed, 2, true);
        json_uint(serial, "time", ccst->time, false);
        json_objEnd(serial, more);
}

void camera_control_get_config(struct camera_control_config* cfg,
                            struct Serial* serial,
                            const bool more)
{
        json_objStartString(serial, "camCtrlCfg");
        json_bool(serial, "active", cfg->active, true);
        json_int(serial, "makeModel", cfg->make_model, true);
        get_speed_time(serial, &cfg->start, "start", true);
        get_speed_time(serial, &cfg->stop, "stop", false);
        json_objEnd(serial, more);
}

static void set_speed_time(struct camera_control_speed_time *ccst,
                           const char* name,
                           const jsmntok_t* root)
{
        const jsmntok_t* tok = jsmn_find_node(root, name);
        if (!tok)
                return;

        jsmn_exists_set_val_float(tok, "speed", &ccst->speed);
        jsmn_exists_set_val_int(tok, "time", &ccst->time);
}


bool camera_control_set_config(struct camera_control_config* cfg,
                            const jsmntok_t *json)
{
        jsmn_exists_set_val_bool(json, "active", &cfg->active);
        jsmn_exists_set_val_int(json, "makeModel", &cfg->make_model);
        set_speed_time(&cfg->start, "start", json);
        set_speed_time(&cfg->stop, "stop", json);
        return true;
}

static bool should_start_recording(const float current_speed,
                                 const tiny_millis_t uptime)
{
        const tiny_millis_t trig_time =
                (tiny_millis_t) camera_control_state.cfg->start.time * 1000;
        const float trig_speed = camera_control_state.cfg->start.speed;

        if (0 == trig_time)
                return false;

        if (current_speed < trig_speed) {
        		camera_control_state.timestamp_start = 0;
                return false;
        }

        if (0 == camera_control_state.timestamp_start) {
        		camera_control_state.timestamp_start = uptime;
                return false;
        }

        const tiny_millis_t time_diff =
                uptime - camera_control_state.timestamp_start;
        return time_diff > trig_time;
}

static bool should_stop_recording(const float current_speed,
                                const tiny_millis_t uptime)
{
        const tiny_millis_t trig_time =
                (tiny_millis_t) camera_control_state.cfg->stop.time * 1000;
        const float trig_speed = camera_control_state.cfg->stop.speed;

        if (0 == trig_time)
                return false;

        if (current_speed > trig_speed) {
        		camera_control_state.timestamp_stop = 0;
                return false;
        }

        if (0 == camera_control_state.timestamp_stop) {
        		camera_control_state.timestamp_stop = uptime;
                return false;
        }

        const tiny_millis_t time_diff =
                uptime - camera_control_state.timestamp_stop;
        return time_diff > trig_time;
}

static void camera_control_sample_cb(const struct sample* sample,
                           const int tick, void* data)
{
        return;

        if (!camera_control_state.cfg || !camera_control_state.cfg->active)
                return;

        float current_speed = 0;

        const tiny_millis_t uptime = getUptime();
        if (!camera_control_state.recording) {
                if (!should_start_recording(current_speed, uptime))
                        return;

                wifi_trigger_camera(true);
                camera_control_state.recording = true;
        } else {
                if (!should_stop_recording(current_speed, uptime))
                        return;

                wifi_trigger_camera(false);
                camera_control_state.recording = false;
        }
}

bool camera_control_init(struct camera_control_config* cfg)
{
        if (!cfg)
                return false;

        camera_control_state.cfg = cfg;
        camera_control_state.recording = false;
        camera_control_state.timestamp_start = 0;
        camera_control_state.timestamp_stop = 0;

        logger_sample_create_callback(camera_control_sample_cb, 10, NULL);

        return true;
}
