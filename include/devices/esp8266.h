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

#include <stdbool.h>

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

bool esp8266_init(struct at_info *ati, void (*cb)(enum dev_init_state));

enum dev_init_state esp1866_get_dev_init_state();

/**
 * These are the AT codes for the mode of the device represented in enum
 * form.  Do not change them unless you know what you are doing.
 */
enum esp8266_op_mode {
        ESP8266_OP_MODE_UNKNOWN = 0, /* Default value. */
        ESP8266_OP_MODE_CLIENT  = 1,
        ESP8266_OP_MODE_AP      = 2,
        ESP8266_OP_MODE_BOTH    = 3, /* Client & AP */
};

bool esp8266_set_op_mode(const enum esp8266_op_mode mode,
                         void (*cb)(bool status));

bool esp8266_get_op_mode(void (*cb)(bool, enum esp8266_op_mode));

struct esp8266_client_info {
        bool has_ap;
        char ssid[24];
        char mac[18];
        char ip[16];
};

bool esp8266_join_ap(const char* ssid, const char* pass, void (*cb)(bool));

bool esp8266_get_client_ap(void (*cb)
                           (bool, const struct esp8266_client_info*));

bool esp8266_get_client_ip(void (*cb)
                           (bool, const struct esp8266_client_info*));

bool esp8266_get_client_info(void (*cb)
                             (bool, const struct esp8266_client_info*));

enum esp8266_net_proto {
        ESP8266_NET_PROTO_TCP,
        ESP8266_NET_PROTO_UDP,
};

bool esp8266_connect(const int chan_id, const enum esp8266_net_proto proto,
                     const char *ip_addr, const int dest_port,
                     const int udp_port, const int udp_mode,
                     void (*cb) (bool, const int));

bool esp8266_send_data(const int chan_id, const char *data,
                       const size_t len, void (*cb)(bool));

CPP_GUARD_END

#endif /* _ESP8266_H_ */
