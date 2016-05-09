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

#include "at.h"
#include "cpp_guard.h"
#include "dateTime.h"
#include "net/protocol.h"
#include "serial.h"
#include <stdbool.h>

CPP_GUARD_BEGIN

void esp8266_do_loop(const size_t timeout);

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

typedef void client_wifi_disconnect_cb_t();
typedef void socket_connect_cb_t(const size_t chan_id);
typedef void socket_closed_cb_t(const size_t chan_id);

struct esp8266_event_hooks {
        client_wifi_disconnect_cb_t *client_wifi_disconnect_cb;
        socket_connect_cb_t *socket_connect_cb;
        socket_closed_cb_t *socket_closed_cb;
};

bool esp8266_init(struct Serial *s, const size_t max_cmd_len,
                  const struct esp8266_event_hooks hooks,
                  void (*cb)(enum dev_init_state));

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
        tiny_millis_t snapshot_time;
        bool has_ap;
        char ssid[24];
        char mac[18];
        char ip[16];
};

void esp8266_log_client_info(const struct esp8266_client_info *info);

bool esp8266_join_ap(const char* ssid, const char* pass, void (*cb)(bool));

bool esp8266_get_client_ap(void (*cb)
                           (bool, const struct esp8266_client_info*));

bool esp8266_get_client_ip(void (*cb)
                           (bool, const char*));

bool esp8266_connect(const int chan_id, const enum protocol proto,
                     const char *ip_addr, const int dest_port,
                     void (*cb) (bool, const int));

bool esp8266_send_data(const int chan_id, struct Serial *data,
                       const size_t len, void (*cb)(int));

bool esp8266_send_serial(const int chan_id, struct Serial *serial,
                         const size_t len, void (*cb)(bool));

/**
 * These ENUM values match the AT values needed for the command.  Do not
 * change them unless you know what you are doing.
 */
enum esp8266_server_action {
        ESP8266_SERVER_ACTION_DELETE = 0,
        ESP8266_SERVER_ACTION_CREATE = 1,
};

bool esp8266_server_cmd(const enum esp8266_server_action action, int port,
                        void (*cb)(bool));

bool esp8266_register_ipd_cb(void (*cb)(int, size_t, const char*));

CPP_GUARD_END

#endif /* _ESP8266_H_ */
