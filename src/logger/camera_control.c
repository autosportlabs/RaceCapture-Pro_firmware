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
#include <string.h>

#define LOG_PFX			"[cam_ctrl] "

#define DEFAULT_CAMERA_CONTROL_CHANNEL "Speed"

static struct {
        struct camera_control_config *cfg;
        struct auto_control_state control_state;
} camera_control_state;

void camera_control_reset_config(struct camera_control_config* cfg)
{
        cfg->active = false;
        strcpy(cfg->channel, DEFAULT_CAMERA_CONTROL_CHANNEL);
        cfg->make_model = CAMERA_MAKEMODEL_GOPRO_HERO2_3;
        auto_control_reset_trigger(&cfg->start, &cfg->stop);
}

void camera_control_get_config(struct camera_control_config* cfg,
                            struct Serial* serial,
                            const bool more)
{
        json_objStartString(serial, "camCtrlCfg");
        json_bool(serial, "active", cfg->active, true);
        json_int(serial, "makeModel", cfg->make_model, true);
        json_string(serial, "channel", cfg->channel, true);
        get_auto_control_trigger(serial, &cfg->start, "start", true);
        get_auto_control_trigger(serial, &cfg->stop, "stop", false);
        json_objEnd(serial, more);
}

bool camera_control_set_config(struct camera_control_config* cfg,
                            const jsmntok_t *json)
{
        jsmn_exists_set_val_bool(json, "active", &cfg->active);
        jsmn_exists_set_val_string(json, "channel", cfg->channel, DEFAULT_LABEL_LENGTH, true);
        jsmn_exists_set_val_uint8(json, "makeModel", &cfg->make_model, NULL);
        set_auto_control_trigger(&cfg->start, "start", json);
        set_auto_control_trigger(&cfg->stop, "stop", json);
        return true;
}

static void camera_control_sample_cb(const struct sample* sample,
                           const int tick, void* data)
{
        if (!camera_control_state.cfg || !camera_control_state.cfg->active)
                return;

        double value;
        if (!get_sample_value_by_name(sample, camera_control_state.cfg->channel, &value))
                return;

        enum auto_control_trigger_result res = auto_control_check_trigger(value,
                                                                             &camera_control_state.cfg->start,
                                                                             &camera_control_state.cfg->stop,
                                                                             &camera_control_state.control_state);
        switch(res){
                case AUTO_CONTROL_TRIGGERED:
                        wifi_trigger_camera(true);
                        pr_info(LOG_PFX "Auto-starting camera\r\n");
                        break;
                case AUTO_CONTROL_UNTRIGGERED:
                        wifi_trigger_camera(false);
                        pr_info(LOG_PFX "Auto-stopping camera\r\n");
                        break;
                default:
                        break;
        }
}

bool camera_control_init(struct camera_control_config* cfg)
{
        if (!cfg)
                return false;

        camera_control_state.cfg = cfg;
        auto_control_init_state(&camera_control_state.control_state);

        logger_sample_create_callback(camera_control_sample_cb, 10, NULL);
        return true;
}
