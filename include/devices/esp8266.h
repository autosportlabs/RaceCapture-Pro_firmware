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

#ifndef _ESP8266_H_
#define _ESP8266_H_

#include "cpp_guard.h"

CPP_GUARD_BEGIN

/**
 * The various initialization states of the device.  Probably should
 * get put in a more generic file.  Here is good for now.
 */
enum dev_init_state {
        DEV_INIT_STATE_NOT_READY = 0,
        DEV_INIT_INITIALIZING,
        DEV_INIT_STATE_READY,
        DEV_INIT_STATE_FAILED,
};

bool esp8266_begin_init(struct at_info *ati);

enum dev_init_state esp1866_get_dev_init_state();


CPP_GUARD_END

#endif /* _ESP8266_H_ */
