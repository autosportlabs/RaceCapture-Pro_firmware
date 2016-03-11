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

#include "at.h"
#include "array_utils.h"
#include "esp8266.h"
#include "printk.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ESP8266_AUTOBAUD_TRIES	20
#define ESP8266_CMD_DELIM	"\r\n"
#define ESP8266_QP_PRE_INIT_MS	500
#define ESP8266_QP_STANDARD_MS	1	/* Can probably be 0 */

#define TIMEOUT_SUPER_MS	30000
#define TIMEOUT_LONG_MS		5000
#define TIMEOUT_MEDIUM_MS	500
#define TIMEOUT_SHORT_MS	50

static const enum log_level serial_dbg_lvl = INFO;

/**
 * Internal state of our driver.
 */
static struct {
        struct at_info *ati;
        enum dev_init_state init_state;
        struct esp8266_client_info client_info;
} state;

static void cmd_failure(const char *cmd_name, const char *msg)
{
        pr_warning("[esp8266] ");
        pr_warning(cmd_name);
        if (msg) {
                pr_info_str_msg("failed with msg: ", msg);
        } else {
                pr_warning(" failed.\r\n");
        }
}

/**
 * Call the if the init routine fails.
 * @param msg The message to print.
 */
static void init_failed(const char *msg)
{
        state.init_state = DEV_INIT_STATE_FAILED;
        cmd_failure("Init", msg);
}

static void init_success()
{
        pr_info("[esp8266] Initialized\r\n");
        state.init_state = DEV_INIT_STATE_READY;
}

static void get_version_cb(struct at_rsp *rsp, void *up)
{
        if (!at_ok(rsp)) {
                /* WTF Mate */
                init_failed("Failed to get version info");
                return;
        }

        /*
         * AT version:0.25.0.0(Jun  5 2015 16:27:16)
         * SDK version:1.1.1
         * Ai-Thinker Technology Co. Ltd.
         * Jun 23 2015 23:23:50
         *
         * Print for now beacuse I am lazy
         */
        pr_info("[esp8266] Version info:\r\n");
        for (size_t i = 0; i < rsp->msg_count - 1; ++i)
                pr_info_str_msg("\t", rsp->msgs[i]);

        init_success();
}

static void get_version()
{
        at_put_cmd(state.ati, "AT+GMR", TIMEOUT_MEDIUM_MS,
                   get_version_cb, NULL);
}

static void set_echo_cb(struct at_rsp *rsp, void *up)
{
        if (at_ok(rsp))
                return;

        /* Failure. :( */
        init_failed("Set echo failed.");
}

static void set_echo(const bool on)
{
        const char *cmd = on ? "ATE1" : "ATE0";
        at_put_cmd(state.ati, cmd, TIMEOUT_SHORT_MS,
                   set_echo_cb, NULL);
}

/**
 * Callback for Autobauding.  Also used when starting
 */
static void autobaud_cb(struct at_rsp *rsp, void *tries)
{
        /* Check if we got an OK reply.  If so, move to next step */
        if (at_ok(rsp)) {
                /* Set normal quiet period since we have auto-bauded */
                at_configure_device(state.ati, ESP8266_QP_STANDARD_MS,
                                    ESP8266_CMD_DELIM);
                set_echo(false);
                get_version();
                return;
        }

        --tries;
        if (!tries) {
                /* Out of tries.  We are done */
                init_failed("Autobaud failed.");
                return;
        }

        at_put_cmd(state.ati, "AT", TIMEOUT_MEDIUM_MS, autobaud_cb, tries);
}

/**
 * Kicks off the autobauding process.  We configure the AT engine to use
 * the callback method for both call backs and the initilazation routine.
 * Saves us a method.  Its like recursion, except without the stack overflows.
 */
static void begin_autobaud_cmd()
{
        at_configure_device(state.ati, ESP8266_QP_PRE_INIT_MS,
                            ESP8266_CMD_DELIM);
        autobaud_cb(NULL, (void*) ESP8266_AUTOBAUD_TRIES);
}

static bool is_init_in_progress()
{
        return state.init_state == DEV_INIT_INITIALIZING;
}

static void serial_tx_cb(const char *data)
{
        static bool pr_tx_pfx = true;

        for(; *data; ++data) {
                if (pr_tx_pfx) {
                        printk(serial_dbg_lvl, "[esp8266] tx: ");
                        pr_tx_pfx = false;
                }

                switch(*data) {
                case('\r'):
                        printk(serial_dbg_lvl, "\\r");
                        break;
                case('\n'):
                        printk(serial_dbg_lvl, "\\n\r\n");
                        pr_tx_pfx = true;
                        break;
                default:
                        printk_char(serial_dbg_lvl, *data);
                        break;
                }
        }
}

static void serial_rx_cb(const char *data)
{
        static bool new_line = true;

        if (NULL == data)
                return;

        for(; *data; ++data) {
                if (new_line) {
                        printk(serial_dbg_lvl, "[esp8266] rx: ");
                        new_line = false;
                }

                switch(*data) {
                case('\r'):
                        printk(serial_dbg_lvl, "\\r");
                        break;
                case('\n'):
                        printk(serial_dbg_lvl, "\\n\r\n");
                        new_line = true;
                        break;
                default:
                        printk_char(serial_dbg_lvl, *data);
                        break;
                }
        }
}

/**
 * Kicks off the initilization process.
 */
bool esp8266_begin_init(struct at_info *ati)
{
        if (is_init_in_progress())
                return false;

        if (!ati)
                return false;

        /* HACK: Remove me later */
        ati->sb->serial->tx_callback = serial_tx_cb;
        ati->sb->serial->rx_callback = serial_rx_cb;

        state.init_state = DEV_INIT_INITIALIZING;
        state.ati = ati;
        begin_autobaud_cmd();
        return true;
}

/**
 * @return The initialization state of the device.
 */
enum dev_init_state esp1866_get_dev_init_state()
{
        return state.init_state;
}

/**
 * Allows us to quickly check if the device has successfully initialized.
 * Notifies the log if it isn't initialized.
 * @return true if initialized, false otherwise.
 */
static bool check_initialized(const char *cmd_name)
{
        const bool init = DEV_INIT_STATE_READY == state.init_state;

        if (!init)
                cmd_failure(cmd_name, "Device not initialized");

        return init;
}

static void set_op_mode_cb(struct at_rsp *rsp, void *up)
{
        void (*cb)(bool) = up;

        const bool status = at_ok(rsp);
        if (!status) {
                /* We don't know the mode.  So do nothing */
                cmd_failure("set_op_mode_cb", NULL);
        }

        if (cb)
                cb(status);
}

/**
 * Sets the operational mode of the WiFi device.
 */
bool esp8266_set_op_mode(const enum esp8266_op_mode mode,
                         void (*cb)(bool status))
{
        if (!check_initialized("set_op_mode"))
                return false;

        char cmd_str[16];
        snprintf(cmd_str, ARRAY_LEN(cmd_str), "AT+CWMODE=%d", (int) mode);
        return NULL != at_put_cmd(state.ati, cmd_str, TIMEOUT_MEDIUM_MS,
                                  set_op_mode_cb, cb);
}

static void read_op_mode_cb(struct at_rsp *rsp, void *up)
{
        static const char *cmd_name = "read_op_mode_cb";
        void (*cb)(bool, enum esp8266_op_mode) = up;
        bool status = at_ok(rsp);
        enum esp8266_op_mode mode = ESP8266_OP_MODE_UNKNOWN;

        if (!status) {
                cmd_failure(cmd_name, NULL);
                goto do_cb;
        }

        char *toks[2];
        const size_t tok_cnt =
                at_parse_rsp_line(rsp->msgs[rsp->msg_count - 1],
                                  toks, ARRAY_LEN(toks));
        if (tok_cnt != 2) {
                cmd_failure(cmd_name, "Incorrect number of tokens parsed.");
                status = false;
                goto do_cb;
        }

        /* If here, parse the number.  It should match our enum*/
        const int val = atoi(toks[1]);
        pr_debug_int_msg("[esp8266] op mode: ", val);
        mode = (enum esp8266_op_mode) val;

        /* When we get here, we are done processing the reply*/
do_cb:
        if (cb)
                cb(status, mode);
}

/**
 * Reads the operational mode of the WiFi device.
 */
bool esp8266_get_op_mode(void (*cb)(bool, enum esp8266_op_mode))
{
        if (!check_initialized("read_op_mode"))
                return false;

        return NULL != at_put_cmd(state.ati, "AT+CWMODE?", TIMEOUT_SHORT_MS,
                                  read_op_mode_cb, cb);
}


/**
 * Callback invoked at the end of the join_ap command.
 */
static void join_ap_cb(struct at_rsp *rsp, void *up)
{
        void (*cb)(bool) = up;
        const bool status = at_ok(rsp);

        if (!status)
                cmd_failure("join_ap_cb", NULL);

        if (cb)
                cb(status);
}

/**
 * Instructs the ESP8266 to join the given AP.
 * @param ssid The network Service Set ID (The name)
 * @param pass The network password if there is one.  Use an empty string
 * for no password.
 * @param cb The callback to invoke when done.
 */
bool esp8266_join_ap(const char* ssid, const char* pass, void (*cb)(bool))
{
        if (!check_initialized("join_ap"))
                return false;

        char cmd[64];
        snprintf(cmd, ARRAY_LEN(cmd), "AT+CWJAP=\"%s\",\"%s\"", ssid, pass);
        return NULL != at_put_cmd(state.ati, cmd, TIMEOUT_SUPER_MS,
                                  join_ap_cb, cb);
}

/**
 * Clears the client_info structure we have.
 */
static void clear_client_info(struct esp8266_client_info *info)
{
        memset(info, 0, sizeof(struct esp8266_client_info));
}

static bool parse_client_info(char *rsp)
{
        clear_client_info(&state.client_info);

        if (!rsp)
                return false;

        /*
         * Success case:
         * > +CWJAP:"madworks","2a:a4:3c:6d:46:37",6,-53
         * No AP case:
         * > No AP
         */
        char *toks[6];
        const size_t tok_cnt = at_parse_rsp_line(rsp, toks, ARRAY_LEN(toks));
        if (tok_cnt != 5)
                return false;

        const char *ssid = at_parse_rsp_str(toks[1]);
        const char *mac = at_parse_rsp_str(toks[2]);
        if (!ssid || !mac)
                return false;

        strncpy(state.client_info.ssid, ssid, ARRAY_LEN(state.client_info.ssid));
        strncpy(state.client_info.mac, mac, ARRAY_LEN(state.client_info.mac));

        state.client_info.has_ap = true;
        return true;
}

/**
 * Callback that is invoked when the get_ap command completes.
 */
static void get_client_ap_cb(struct at_rsp *rsp, void *up)
{
        static const char *cmd_name = "get_ap_cb";
        void (*cb)(bool, const struct esp8266_client_info*) = up;
        bool status = at_ok(rsp);

        if (!status) {
                cmd_failure(cmd_name, NULL);
                goto do_cb;
        }

        char *client_info = rsp->msgs[rsp->msg_count - 1];
        if (!parse_client_info(client_info)) {
                cmd_failure(cmd_name, "Failed to parse AP info");
                status = false;
                goto do_cb;
        }

do_cb:
        if (cb)
                cb(status, &state.client_info);
}

/**
 * Use this to figure out the info about the client AP.  Gets
 */
bool esp8266_get_client_ap(void (*cb)(bool, const struct esp8266_client_info*))
{
        if (!check_initialized("get_client_ap"))
                return false;

        return NULL != at_put_cmd(state.ati, "AT+CWJAP?", TIMEOUT_SUPER_MS,
                                  get_client_ap_cb, cb);
}

static bool parse_client_ip(struct at_rsp *rsp)
{
        /*
         * +CIFSR:STAIP,"192.168.1.94"
         * +CIFSR:STAMAC,"18:fe:34:f4:3a:95"
         */
        char *toks[4];
        char *ip_at_str = NULL;
        for(size_t i = 0; i < rsp->msg_count && NULL == ip_at_str; ++i) {
                const size_t tok_cnt =
                        at_parse_rsp_line(rsp, toks, ARRAY_LEN(toks));
                if (tok_count == 3 && STR_EQ("STAIP", toks[1]))
                        ip_at_str = toks[2];
        }
}

/**
 * Callback that is invoked when the get_client_ip command completes.
 */
static void get_client_ip_cb(struct at_rsp *rsp, void *up)
{
        static const char *cmd_name = "get_client_ip_cb";
        void (*cb)(bool, const char*) = up;
        bool status = at_ok(rsp);

        if (!status) {
                cmd_failure(cmd_name, NULL);
                goto do_cb;
        }

        /*
         * +CIFSR:STAIP,"192.168.1.94"
         * +CIFSR:STAMAC,"18:fe:34:f4:3a:95"
         */
        char *toks[4];
        char *ip_at_str = NULL;
        for(size_t i = 0; i < rsp->msg_count && NULL == ip_at_str; ++i) {
                const size_t tok_cnt =
                        at_parse_rsp_line(rsp, toks, ARRAY_LEN(toks));
                if (tok_count == 3 && STR_EQ("STAIP", toks[1]))
                        ip_at_str = toks[2];
        }

        if (!ip_at_str) {
                cmd_failure(cmd_name, "Failed to parse IP");
                status = false;
                goto do_cb;
        }

        char *ip_str = at_parse_rsp_str(ip_at_str);


do_cb:
        if (cb)
                cb(status, &state.client_info);
}

/**
 * Use this to figure out what our IP is as a wireless client.
 */
bool esp8266_get_client_ip(void (*cb)(bool, const struct esp8266_client_info*))
{
        if (!check_initialized("get_ap_mode"))
                return false;

        char cmd[64];
        snprintf(cmd, ARRAY_LEN(cmd), "AT+CWJAP=\"%s\",\"%s\"", name, pass);
        return NULL != at_put_cmd(state.ati, cmd, TIMEOUT_SUPER_MS,
                                  get_ap_cb, cb);
}
