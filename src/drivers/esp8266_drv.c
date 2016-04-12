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

#include "FreeRTOS.h"
#include "dateTime.h"
#include "esp8266.h"
#include "esp8266_drv.h"
#include "macros.h"
#include "loggerConfig.h"
#include "panic.h"
#include "printk.h"
#include "queue.h"
#include "task.h"
#include "wifi.h"
#include <stdbool.h>
#include <string.h>

#define _AT_TASK_TIMEOUT_MS	3
#define _BAD_STATE_BACKOFF_MS	3000
#define _CLIENT_BACKOFF_MS	30000
#define _DAEMON_SERVER_PORT	7223
#define _INIT_FAIL_SLEEP_MS	10000
#define _LOG_PFX		"[ESP8266 Driver] "
#define _MAX_CHANNELS		5
#define _SERIAL_BAUD		115200
#define _SERIAL_BITS		8
#define _SERIAL_CMD_MAX_LEN	1024
#define _SERIAL_PARITY		0
#define _SERIAL_RX_BUFF_SIZE	256
#define _SERIAL_STOP_BITS	1
#define _SERIAL_TX_BUFF_SIZE	256
#define _TASK_STACK_SIZE	512
#define _TASK_THREAD_NAME	"ESP8266 Driver"

enum _cmd {
        _CMD_UNKNOWN = 0,
        _CMD_NOOP,
        _CMD_INIT,
        _CMD_CLIENT_AP_GET,
        _CMD_CLIENT_AP_SET,
        _CMD_CLIENT_IP_GET,
        _CMD_DAEMON_SETUP,
};

struct _client {
        struct esp8266_client_info info;
        const struct wifi_client_cfg *config;
        tiny_millis_t next_check_ms;
};

struct _daemon {
        bool listening;
};

struct _channel {
        struct Serial *serial;
        size_t tx_chars_buffered;
};

static struct _state {
        struct Serial *serial;
        new_conn_func_t *new_conn_cb;
        struct _channel channels[_MAX_CHANNELS];
        struct _client client;
        struct _daemon daemon;
        bool cmd_ip;
        enum _cmd cmd;
        enum dev_init_state dev_state;
        tiny_millis_t cmd_sleep_until;
} state;

static void cmd_started()
{
        state.cmd_ip = true;
}

static void cmd_completed(const enum _cmd next_cmd,
                          const tiny_millis_t sleep_ms)
{
        state.cmd = next_cmd;
        state.cmd_sleep_until = getUptimeFromNow(sleep_ms);
        state.cmd_ip = false;
}

static const char* get_channel_name(const size_t chan_id)
{
        static const char* _serial_names[] = {
                "Wifi Chan 0",
                "Wifi Chan 1",
                "Wifi Chan 2",
                "Wifi Chan 3",
        };

        if (chan_id >= ARRAY_LEN(_serial_names))
                return NULL;

        return _serial_names[chan_id];
}

static void _tx_char_cb(xQueueHandle queue, void *post_tx_arg)
{
        struct _channel *ch = post_tx_arg;
        ++ch->tx_chars_buffered;
}

static struct _channel* get_channel(const int chan_id)
{
        struct _channel *ch = state.channels + chan_id;
        if (ch->serial)
                return ch;

        const char* name = get_channel_name(chan_id);
        struct Serial *s = serial_create(name, _SERIAL_TX_BUFF_SIZE,
                                         _SERIAL_RX_BUFF_SIZE, NULL, NULL,
                                         _tx_char_cb, ch);
        if (NULL == s)
                pr_error(_LOG_PFX "Failed to create serial port\r\n");

        ch->serial = s;
        ch->tx_chars_buffered = 0;

        return ch;
}

static void rx_data_cb(int chan_id, size_t len, const char* data)
{
        if (chan_id >= ARRAY_LEN(state.channels)) {
                pr_error_int_msg(_LOG_PFX "Channel id to big: ", chan_id);
                return;
        }

        struct _channel *ch = get_channel(chan_id);
        if (NULL == ch) {
                pr_error(_LOG_PFX "No channel available.  Dropping\r\n");
                return;
        }

        /* Call back to listening task to indicate new socket */
        if (!state.new_conn_cb) {
                pr_error(_LOG_PFX "No Serial callback defined\r\n");
                return;
        }
        state.new_conn_cb(ch->serial);

        pr_info_str_msg(_LOG_PFX "Message: ", data);
        /* STIEG: Make this not portMAX_DELAY to prevent stalling? */
        xQueueHandle q = serial_get_rx_queue(ch->serial);
        for (size_t i = 0; i < len; ++i)
                xQueueSend(q, data + i, portMAX_DELAY);
}

static void init_wifi_cb(enum dev_init_state dev_state)
{
        state.dev_state = dev_state;
        pr_info_int_msg(_LOG_PFX "Device state: ", dev_state);

        if (DEV_INIT_STATE_READY != dev_state) {
                /* Then init failed.  We should sleep and try again */
                cmd_completed(_CMD_INIT, _INIT_FAIL_SLEEP_MS);
        } else {
                /* W00t!  Now we let our state determine next cmd */
                cmd_completed(_CMD_UNKNOWN, 0);
        }
}

static void init_wifi()
{
        if (!esp8266_init(state.serial, _SERIAL_CMD_MAX_LEN, init_wifi_cb)) {
                /* Failed to init critical bits.  */
                pr_error(_LOG_PFX "Failed to init esp8266 device code.\r\n");
                cmd_completed(_CMD_INIT, _INIT_FAIL_SLEEP_MS);
                return;
        }

        cmd_started();

        /*
         * Now register the callback for incoming data.  Safe to do
         * before init has completed b/c this invokes no modem calls.
         * This should only fail if there was an issue with space or config.
         */
        if (!esp8266_register_ipd_cb(rx_data_cb))
                pr_error(_LOG_PFX "Failed to register IPD callback\r\n");
}

static bool is_client_wifi_on_desired_network(const struct esp8266_client_info *ci)
{
        if (!ci || !ci->has_ap || !state.client.config)
                return false;

        return STR_EQ(ci->ssid, state.client.config->ssid);
}

static void get_client_ap_cb(bool status, const struct esp8266_client_info *ci)
{
        memcpy(&state.client.info, ci, sizeof(struct esp8266_client_info));
        state.client.next_check_ms = getUptimeFromNow(_CLIENT_BACKOFF_MS);

        if (!status) {
                /* Command failed */
                cmd_completed(_CMD_UNKNOWN, 0);
        } else if (!is_client_wifi_on_desired_network(ci)) {
                /* Not on the network we want to be on.  Set it */
                pr_info(_LOG_PFX "Client not on correct network\r\n");
                cmd_completed(_CMD_CLIENT_AP_SET, 0);
        } else {
                /* On the network we want to be on.  Get the IP. */
                pr_info(_LOG_PFX "Client is on desired network\r\n");
                esp8266_log_client_info(&state.client.info);
                cmd_completed(_CMD_CLIENT_IP_GET, 0);
        }
}

static void get_client_ap()
{
        esp8266_get_client_ap(get_client_ap_cb);
        cmd_started();
}

static void get_client_ip_cb(bool status, const char* ip)
{
        if (!status) {
                /* We don't know the IP */
                *state.client.info.ip = '\0';
        } else {
                /* On the network we want to be on */
                pr_info_str_msg(_LOG_PFX "Got IP: ", ip);
                strncpy(state.client.info.ip, ip,
                        ARRAY_LEN(state.client.info.ip));
                esp8266_log_client_info(&state.client.info);
        }

        cmd_completed(_CMD_UNKNOWN, 0);
}

static void get_client_ip()
{
        esp8266_get_client_ip(get_client_ip_cb);
        cmd_started();
}


static void set_client_ap_cb(bool status)
{
        if (!status) {
                /* Failed. */
                pr_info(_LOG_PFX "Failed to join network\r\n");
                cmd_completed(_CMD_UNKNOWN, 0);
        } else {
                /* If here, we were successful.  Now get client wifi info */
                pr_info(_LOG_PFX "Successfully joined network\r\n");
                cmd_completed(_CMD_CLIENT_AP_GET, 0);
        }
}

static void set_client_ap()
{
        const struct wifi_client_cfg *cc = state.client.config;
        pr_info_str_msg(_LOG_PFX "Joining network: ", cc->ssid);
        esp8266_join_ap(cc->ssid, cc->passwd, set_client_ap_cb);
        cmd_started();
}

static void server_cmd_cb(bool status)
{
        if (!status)
                pr_warning(_LOG_PFX "Failed to setup server\r\n");

        state.daemon.listening = status;
        cmd_completed(_CMD_UNKNOWN, 0);
}

static void setup_server()
{
        pr_info_int_msg(_LOG_PFX "Starting server on port: ",
                        _DAEMON_SERVER_PORT);
        esp8266_server_cmd(ESP8266_SERVER_ACTION_CREATE, _DAEMON_SERVER_PORT,
                           server_cmd_cb);
        cmd_started();
}

/**
 * Determines what the next command should be given that we have none that
 * need to explicitly be done.
 */
static enum _cmd determine_cmd()
{
        /* Are we initialized.  If not, should do that */
        /* TODO: Do this when re-init support is added. */

        /* If client is enabled, ensure setup if not in timeout */
        /* STIEG: Assume it enabled for now */
        if (isTimePassed(state.client.next_check_ms))
                return _CMD_CLIENT_AP_GET;

        /* If host is enabled, ensure setup if not in timeount*/
        /* STIEG: Do this later when host implemented */

        /* Start daemon if either host or client is successfully setup. */
        /* STIEG: Add host logic here when implemented */
        if (state.client.info.has_ap && !state.daemon.listening)
                return _CMD_DAEMON_SETUP;

        /* If we get this far, nothing to do. */
        return _CMD_NOOP;
}

static void _send_serial_cb(int bytes_sent)
{
        if (bytes_sent < 0)
                /* STIEG: Include channel info here somehow */
                pr_warning(_LOG_PFX "Failed to send data\r\n");

        cmd_completed(_CMD_UNKNOWN, 0);
}

static void process_outgoing()
{
        /* STIEG: Optimize this for runtime */
        for (size_t i = 0; i < ARRAY_LEN(state.channels); ++i) {
                struct _channel *ch = state.channels + i;
                const size_t size = ch->tx_chars_buffered;

                /* If the size is 0, nothing to send */
                if (0 == size)
                        continue;

                /* If here, then we need to queue up a send */
                const bool cmd_queued =
                        esp8266_send_data(i, ch->serial,
                                          ch->tx_chars_buffered,
                                          _send_serial_cb);
                if (cmd_queued) {
                        cmd_started();

                        /* STIEG: Sane?  I guess so for now */
                        ch->tx_chars_buffered -= size;
                }

                return;
        }
}

static void _task_loop()
{
        esp8266_do_loop(_AT_TASK_TIMEOUT_MS);

        /* If there is a command in progress, no commands */
        if (state.cmd_ip)
                return;

        /* Check if there is outgoing data that needs processing */
        process_outgoing();

        /* If we are in a command sleep, do nothing until it expires. */
        if (!isTimePassed(state.cmd_sleep_until))
                return;

        enum _cmd cmd = state.cmd;
        if (_CMD_UNKNOWN == cmd)
                cmd = determine_cmd();

        switch(cmd) {
        case _CMD_INIT:
                return init_wifi();
        case _CMD_CLIENT_AP_GET:
                return get_client_ap();
        case _CMD_CLIENT_AP_SET:
                return set_client_ap();
        case _CMD_CLIENT_IP_GET:
                return get_client_ip();
        case _CMD_DAEMON_SETUP:
                return setup_server();
        case _CMD_NOOP:
                break;
        default:
                pr_warning_int_msg(_LOG_PFX "CMD State Unhandled: ", cmd);
                /* Put a sleep here to prevent log flooding */
                cmd_completed(_CMD_UNKNOWN, _BAD_STATE_BACKOFF_MS);
        }
}

static void _task(void *params)
{
        for(;;)
                _task_loop();

        panic(PANIC_CAUSE_UNREACHABLE);
}

bool esp8266_drv_update_client_cfg(const struct wifi_client_cfg *cc)
{
        if (NULL == cc)
                return false;

        state.client.config = cc;
        /* Zero this out so we force the scheduler to check it */
        state.client.next_check_ms = 0;
        return true;
}

bool esp8266_drv_init(struct Serial *s, const int priority,
                      new_conn_func_t new_conn_cb)
{
        if (state.serial)
                return false; /* Already setup */

        state.serial = s;

        if (!state.serial) {
                pr_error(_LOG_PFX "NULL serial\r\n");
                return false;
        }

        serial_config(state.serial, _SERIAL_BITS, _SERIAL_PARITY,
                      _SERIAL_STOP_BITS, _SERIAL_BAUD);

        /* Initialize our WiFi configs here */
        LoggerConfig *lc = getWorkingLoggerConfig();
        const struct wifi_client_cfg *cfg =
                &lc->ConnectivityConfigs.wifi.client;
        if (!esp8266_drv_update_client_cfg(cfg)) {
                pr_error(_LOG_PFX "Failed to set WiFi cfg\r\n");
                return false;
        }

        state.cmd = _CMD_INIT;
        state.new_conn_cb = new_conn_cb;

        const signed char * const task_name =
                (const signed char *) _TASK_THREAD_NAME;
        const size_t stack_size = _TASK_STACK_SIZE;

        xTaskCreate(_task, task_name, stack_size, NULL, priority, NULL);

        return true;
}
