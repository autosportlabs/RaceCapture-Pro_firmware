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

#ifndef _CAMERA_CONTROL_H_
#define _CAMERA_CONTROL_H_

#include "cpp_guard.h"
#include <stdbool.h>
#include "jsmn.h"
#include "gps.h"
#include "channel_config.h"
#include "auto_control.h"
CPP_GUARD_BEGIN

enum camera_make_model {
    CAMERA_MAKEMODEL_GOPRO_HERO2_3 = 0,
    CAMERA_MAKEMODEL_GOPRO_HERO4_5 = 1,
};

struct camera_control_config {
        bool active;
        uint8_t make_model;
        char channel[DEFAULT_LABEL_LENGTH];
        struct auto_control_trigger start;
        struct auto_control_trigger stop;
};

void camera_control_reset_config(struct camera_control_config* cfg);

void camera_control_get_config(struct camera_control_config* cfg,
                            struct Serial* serial,
                            const bool more);

bool camera_control_set_config(struct camera_control_config* cfg,
                            const jsmntok_t *json);

bool camera_control_init(struct camera_control_config* cfg);

CPP_GUARD_END

#endif /* _CAMERA_CONTROL_H_ */
