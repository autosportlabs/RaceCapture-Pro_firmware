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

#ifndef _AUTO_LOGGER_H_
#define _AUTO_LOGGER_H_

#include "cpp_guard.h"
#include "dateTime.h"
#include "serial.h"
#include "jsmn.h"
#include "gps.h"
#include <stdbool.h>

CPP_GUARD_BEGIN

struct auto_logger_speed_time {
        float speed;
        uint32_t time;
};

struct auto_logger_config {
        bool active;
        struct auto_logger_speed_time start;
        struct auto_logger_speed_time stop;
};

void auto_logger_reset_config(struct auto_logger_config* cfg);

void auto_logger_get_config(struct auto_logger_config* cfg,
                            struct Serial* serial,
                            const bool more);

bool auto_logger_set_config(struct auto_logger_config* cfg,
                            const jsmntok_t *json);

bool auto_logger_init(struct auto_logger_config* cfg);

void auto_logger_gps_sample_cb(const GpsSample* sample);

CPP_GUARD_END

#endif /* _AUTO_LOGGER_H_ */
