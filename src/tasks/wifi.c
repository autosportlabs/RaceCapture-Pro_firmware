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
#include "at.h"
#include "esp8266.h"
#include "macros.h"
#include "messaging.h"
#include "printk.h"
#include "ram_serial.h"
#include "rx_buff.h"
#include "serial.h"
#include "serial_device.h"
#include "serial_buffer.h"
#include "str_util.h"
#include "task.h"
#include "wifi.h"

#define WIFI_TASK_THREAD_NAME	"WiFi Task"
#define WIFI_TASK_STACK_SIZE	512
#define WIFI_INIT_FAIL_SLEEP_MS	10000
#define WIFI_CMD_FAIL_SLEEP_MS	3000
#define WIFI_SERIAL_PORT	SERIAL_AUX
#define WIFI_SERIAL_BAUD	115200
#define WIFI_SERIAL_BITS	8
#define WIFI_SERIAL_STOP_BITS	1
#define WIFI_SERIAL_PARITY	0
#define WIFI_SERVER_PORT	7223
#define WIFI_RX_BUFF_SIZE	1024
#define WIFI_SERIAL_BUFF_SIZE	1024
#define WIFI_AT_DEFAULT_QP_MS	250
#define WIFI_AT_DEFAULT_DELIM	"\r\n"
#define WIFI_AT_TASK_TIMEOUT_MS	5

enum wifi_cmd_state {
        WIFI_CMD_STATE_INIT = 0,
        WIFI_CMD_STATE_NOOP,
        WIFI_CMD_STATE_CLIENT_AP_GET,
        WIFI_CMD_STATE_CLIENT_AP_SET,
        WIFI_CMD_STATE_CLIENT_IP_GET,
        WIFI_CMD_STATE_SERVER_CFG,
};

/* Temporary */
static struct client_wifi_cfg {
        char ssid[24];
        char pass[24];
} client_wifi_cfgs [] = {
        {
        .ssid = "LAN Downunder",
        .pass = "HoyHoyHoy!",
        },
};
static struct client_wifi_cfg *clt_wifi_cfg = client_wifi_cfgs;

static struct {
        struct Serial *serial;
        struct serial_buffer serial_buff;
        struct at_info ati;
        struct rx_buff rxb;
        bool cmd_ip;
        tiny_millis_t sleep_until;
        enum wifi_cmd_state cmd_state;
        enum dev_init_state dev_state;
        const struct esp8266_client_info *client_info;
} state;

static void rx_data_cb(int chan_id, size_t len, const char* data)
{
        const bool msg_ready = rx_buff_append(&state.rxb, chan_id,
                                              data, len);
        if (msg_ready) {
                rx_buff_rstrip(&state.rxb);
                pr_info("[Wifi] Rx message ready!\r\n");
                pr_info("\"");
                pr_info(rx_buff_get_buff(&state.rxb));
                pr_info("\"\r\n");
        }
}

static bool init_task_state()
{
        /* Get our serial port setup */
        state.serial = serial_device_get(WIFI_SERIAL_PORT);
        if (!state.serial)
                return false;

        serial_config(state.serial, WIFI_SERIAL_BITS, WIFI_SERIAL_PARITY,
                      WIFI_SERIAL_STOP_BITS, WIFI_SERIAL_BAUD);

        /*
         * Initialize the serial buffer.  This buffer is used for
         * bi-directional communication between the serial tx/rx
         * buffers and our AT state machine.
         */
        if (!serial_buffer_create(&state.serial_buff, state.serial,
                                  WIFI_SERIAL_BUFF_SIZE, NULL))
                return false;

        /* Init our AT engine here */
        if (!init_at_info(&state.ati, &state.serial_buff,
                          WIFI_AT_DEFAULT_QP_MS, WIFI_AT_DEFAULT_DELIM))
                return false;

        /* Allocate our RX buffer for incomming data */
        if (!rx_buff_init(&state.rxb, WIFI_RX_BUFF_SIZE, NULL))
                return false;

        state.cmd_state = WIFI_CMD_STATE_INIT;
        return true;
}

static void set_cmd_in_prog()
{
        state.cmd_ip = true;
}

static void clear_cmd_in_prog()
{
        state.cmd_ip = false;
}

static void wifi_cmd_sleep(tiny_millis_t period)
{
        pr_debug_int_msg("[wifi] Sleeping (ms): ", period);
        state.sleep_until = getUptime() + period;
}

static void init_wifi_cb(enum dev_init_state dev_state)
{
        state.dev_state = dev_state;
        pr_info_int_msg("[wifi] Device state: ", dev_state);

        if (DEV_INIT_STATE_READY != dev_state) {
                /* Then init failed.  We should sleep and try again */
                state.cmd_state = WIFI_CMD_STATE_INIT;
                wifi_cmd_sleep(WIFI_INIT_FAIL_SLEEP_MS);
        } else {
                /* After Init, we check what wifi network we are on */
                /* HACK: Will add server bits here in the future. */
                state.cmd_state = WIFI_CMD_STATE_CLIENT_AP_GET;
        }

        clear_cmd_in_prog();
}

static void init_wifi()
{
        esp8266_init(&state.ati, init_wifi_cb);
        ram_serial_init(WIFI_SERIAL_BUFF_SIZE);

        /*
         * Now register the callback for incoming data.  Safe to do
         * before init has completed b/c this invokes no modem calls.
         */
        if (!esp8266_register_ipd_cb(rx_data_cb)) {
                pr_error("[wifi] Failed to register IPD callback\r\n");
        }

        set_cmd_in_prog();
}

static bool is_client_wifi_on_desired_network()
{
        const struct esp8266_client_info *ci = state.client_info;

        if (!ci || !ci->has_ap)
                return false;

        return STR_EQ(ci->ssid, clt_wifi_cfg->ssid);
}

static void get_client_ap_cb(bool status, const struct esp8266_client_info *ci)
{
        state.client_info = ci;

        if (!status) {
                /* Command failed */
                state.cmd_state = WIFI_CMD_STATE_CLIENT_AP_GET;
                wifi_cmd_sleep(WIFI_CMD_FAIL_SLEEP_MS);
        } else if (is_client_wifi_on_desired_network()) {
                /* On the network we want to be on */
                pr_info("[wifi] On desired client network\r\n");
                esp8266_log_client_info(ci);

                /* HACK FOR NOW */
                state.cmd_state = WIFI_CMD_STATE_SERVER_CFG;
        } else {
                /* Not on the network we want to be on.  Set it */
                state.cmd_state = WIFI_CMD_STATE_CLIENT_AP_SET;
                pr_info("[wifi] Not on correct client network\r\n");
        }

        clear_cmd_in_prog();
}

static void get_client_ap_info()
{
        esp8266_get_client_info(get_client_ap_cb);
        set_cmd_in_prog();
}

static void set_client_ap_cb(bool status)
{
        if (!status) {
                /* Failed.  Sleep a bit then try again */
                state.cmd_state = WIFI_CMD_STATE_CLIENT_AP_SET;
                wifi_cmd_sleep(WIFI_CMD_FAIL_SLEEP_MS);
        } else {
                /* If here, we were successful.  Now get client wifi info */
                state.cmd_state = WIFI_CMD_STATE_CLIENT_AP_GET;
        }

        clear_cmd_in_prog();
}

static void set_client_ap()
{
        pr_info_str_msg("[wifi] Joining network: ", clt_wifi_cfg->ssid);
        esp8266_join_ap(clt_wifi_cfg->ssid, clt_wifi_cfg->pass,
                        set_client_ap_cb);
        set_cmd_in_prog();
}

static void process_rx_msgs_cb(bool status)
{
        pr_info_int_msg("[wifi] Msg tx status: ", status);

        ram_serial_clear();

        clear_cmd_in_prog();
}

/**
 * Handles all of our incoming messages and what we do with them.
 */
static void process_rx_msgs()
{
        if (!rx_buff_is_msg_ready(&state.rxb))
                return; /* Nothing to do */

        /* If here, we have a message to handle */
        char *data_in = state.rxb.buff; /* HACK */
        const size_t len_in = strlen(data_in);
        struct Serial *serial = ram_serial_get_serial();
        process_read_msg(serial, data_in, len_in);

        const char *data_out = ram_serial_get_buff();
        const size_t len_out = ram_serial_get_len();
        const int chan_id = rx_buff_get_chan_id(&state.rxb);
        esp8266_send_data(chan_id, data_out, len_out,
                          process_rx_msgs_cb);

        rx_buff_clear(&state.rxb);

        set_cmd_in_prog();
}

static void server_cmd_cb(bool status)
{
        if (!status)
                pr_warning("[WiFi] Failed to setup server\r\n");

        state.cmd_state = WIFI_CMD_STATE_NOOP;
        clear_cmd_in_prog();
}

static void setup_server()
{
        pr_info_int_msg("[wifi] Starting server on port: ", WIFI_SERVER_PORT);
        esp8266_server_cmd(ESP8266_SERVER_ACTION_CREATE, WIFI_SERVER_PORT,
                           server_cmd_cb);
        set_cmd_in_prog();
}

static void wifi_task_loop()
{
        at_task(&state.ati, WIFI_AT_TASK_TIMEOUT_MS);

        /* If there is a command in progress, then wait */
        if (state.cmd_ip)
                return;

        process_rx_msgs();

        if (state.sleep_until > getUptime())
                /* Then we are sleeping */
                return;

        switch(state.cmd_state) {
        case WIFI_CMD_STATE_INIT:
                return init_wifi();
        case WIFI_CMD_STATE_CLIENT_AP_GET:
                return get_client_ap_info();
        case WIFI_CMD_STATE_CLIENT_AP_SET:
                return set_client_ap();
        case WIFI_CMD_STATE_SERVER_CFG:
                return setup_server();
        case WIFI_CMD_STATE_NOOP:
                return wifi_cmd_sleep(1000);
        default:
                pr_warning_int_msg("[wifi] CMD State Unhandled: ",
                                   state.cmd_state);
                return wifi_cmd_sleep(3000);
        }
}

static void wifi_task(void *params)
{
        if (!init_task_state()) {
                pr_error("[wifi] Failed to init\r\n");
                vTaskDelete(NULL);
                return; /* Panic! */
        }

        for(;;)
                wifi_task_loop();

        /* Panic! */
}

void start_wifi_task(const int priority)
{
        const signed char * const task_name =
                (const signed char *) WIFI_TASK_THREAD_NAME;
        const size_t stack_size = WIFI_TASK_STACK_SIZE;
        xTaskCreate(wifi_task, task_name, stack_size, NULL, priority, NULL);
}
