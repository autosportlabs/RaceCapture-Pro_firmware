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

#ifndef _WIFI_H_
#define _WIFI_H_

#include "cpp_guard.h"
#include "esp8266.h"
#include <stdbool.h>

CPP_GUARD_BEGIN

#define WIFI_SSID_MAX_LEN	24
#define WIFI_PASSWD_MAX_LEN	24

struct wifi_client_cfg {
        bool active;
        char ssid[WIFI_SSID_MAX_LEN];
        char passwd[WIFI_PASSWD_MAX_LEN];
};

struct wifi_ap_cfg {
        bool active;
        char ssid[WIFI_SSID_MAX_LEN];
        char password[WIFI_PASSWD_MAX_LEN];
        uint8_t channel;
        /* Ok for now.  If needed could make independent */
        enum esp8266_encryption encryption;
};

struct wifi_cfg {
        bool active;
        struct wifi_client_cfg client;
        struct wifi_ap_cfg ap;
};

bool wifi_init_task(const int wifi_task_priority,
                    const int wifi_drv_priority);

void wifi_reset_config(struct wifi_cfg *cfg);

bool wifi_update_client_config(struct wifi_client_cfg *wcc);

bool wifi_update_ap_config(struct wifi_ap_cfg *wac);

bool wifi_validate_ap_config(const struct wifi_ap_cfg *wac);

const char* wifi_api_get_encryption_str_val(const enum esp8266_encryption enc);

enum esp8266_encryption wifi_api_get_encryption_enum_val(const char* str);

CPP_GUARD_END

#endif /* _WIFI_H_ */
