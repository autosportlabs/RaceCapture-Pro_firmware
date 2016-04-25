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

#ifndef _ESP8266_DRV_H_
#define _ESP8266_DRV_H_

#include "cpp_guard.h"
#include "serial.h"
#include "wifi.h"

CPP_GUARD_BEGIN

typedef void new_conn_func_t(struct Serial *s);

bool esp8266_drv_update_client_cfg(const struct wifi_client_cfg *cc);

bool esp8266_drv_init(struct Serial *s, const int priority,
                      new_conn_func_t new_conn_cb);

CPP_GUARD_END

#endif /* _ESP8266_DRV_H_ */
