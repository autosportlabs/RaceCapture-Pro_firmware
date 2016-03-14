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
#include "macros.h"
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

struct tx_info {
        const char *data;
        size_t len;
        void (*cb) (bool);
};

/**
 * Internal state of our driver.
 */
static struct {
        struct at_info *ati;
        void (*init_cb)(enum dev_init_state); /* STIEG: Put in own struct? */
        enum dev_init_state init_state;
        struct esp8266_client_info client_info;
        struct tx_info tx_info;
} state;

static void cmd_failure(const char *cmd_name, const char *msg)
{
        pr_warning("[esp8266] ");
        pr_warning(cmd_name);
        if (msg) {
                pr_info_str_msg(" failed with msg: ", msg);
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
        /* If we aren't initializing, then don't do the callback */
        if (state.init_state != DEV_INIT_INITIALIZING)
                return;

        state.init_state = DEV_INIT_STATE_FAILED;
        cmd_failure("Init", msg);
        state.init_cb(state.init_state); /* Will always be defined */
}

static void init_complete(bool status)
{
        /* If we aren't initializing, then don't do the callback */
        if (state.init_state != DEV_INIT_INITIALIZING)
                return;

        if (!status) {
                init_failed("set_mux_mode failed");
                return;
        }

        pr_info("[esp8266] Initialized\r\n");
        state.init_state = DEV_INIT_STATE_READY;
        state.init_cb(state.init_state); /* Will always be defined */
}

static bool get_version_cb(struct at_rsp *rsp, void *up)
{
        if (!at_ok(rsp)) {
                /* WTF Mate */
                init_failed("Failed to get version info");
                return false;
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

        return false;
}

static bool get_version()
{
        return NULL != at_put_cmd(state.ati, "AT+GMR", TIMEOUT_MEDIUM_MS,
                                  get_version_cb, NULL);
}

static bool set_echo_cb(struct at_rsp *rsp, void *up)
{
        if (!at_ok(rsp)) {
                /* Failure. :( */
                init_failed("Set echo failed.");
        }

        return false;
}

static bool set_echo(const bool on)
{
        const char *cmd = on ? "ATE1" : "ATE0";
        return NULL != at_put_cmd(state.ati, cmd, TIMEOUT_SHORT_MS,
                                  set_echo_cb, NULL);
}

static bool set_mux_mode_cb(struct at_rsp *rsp, void *up)
{
        void (*cb)(bool) = up;

        const bool status = at_ok(rsp);
        if (!status)
                cmd_failure("set_mux_mode_cb", NULL);

        if (cb)
                cb(status);

        return false;
}

/**
 * Sets whether or not we are in multiplexing mode.
 * @param mux True if we want to, false otherwise.
 * @cb The callback to invoke when complete.
 * @return True if the command was queued, false otherwise.
 */
static bool set_mux_mode(const bool mux, void (*cb)(bool status))
{
        char cmd_str[12];
        snprintf(cmd_str, ARRAY_LEN(cmd_str), "AT+CIPMUX=%d", mux ? 1 : 0);
        return NULL != at_put_cmd(state.ati, cmd_str, TIMEOUT_SHORT_MS,
                                  set_mux_mode_cb, cb);
}

static void do_remaining_init_tasks()
{
        /* Set normal quiet period since we have auto-bauded */
        at_configure_device(state.ati, ESP8266_QP_STANDARD_MS,
                            ESP8266_CMD_DELIM);

        /* Ensure we don't put down more tasks than we have */
        const bool res = set_echo(false) && get_version() &&
                set_mux_mode(true, init_complete);

        if (!res)
                cmd_failure("do_remaining_init_tasks", "One or more init "
                            "tasks failed to get scheduled");
        /* May need to do more here, probably not */
}

/**
 * Callback for Autobauding.  Also used when starting
 */
static bool autobaud_cb(struct at_rsp *rsp, void *tries)
{
        /* Check if we5B got an OK reply.  If so, move to next step */
        if (at_ok(rsp)) {
                do_remaining_init_tasks();
                return false;
        }

        --tries;
        if (!tries) {
                /* Out of tries.  We are done */
                init_failed("Autobaud failed.");
                return false;
        }

        at_put_cmd(state.ati, "AT", TIMEOUT_MEDIUM_MS, autobaud_cb, tries);
        return false;
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
        return NULL != state.init_cb;
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
bool esp8266_init(struct at_info *ati, void (*cb)(enum dev_init_state))
{
        if (is_init_in_progress() || !cb || !ati)
                return false;

        state.init_cb = cb;
        state.init_state = DEV_INIT_INITIALIZING;
        state.ati = ati;

        /* SERIAL HACK: Remove me later */
        ati->sb->serial->tx_callback = serial_tx_cb;
        ati->sb->serial->rx_callback = serial_rx_cb;

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

static bool set_op_mode_cb(struct at_rsp *rsp, void *up)
{
        void (*cb)(bool) = up;

        const bool status = at_ok(rsp);
        if (!status) {
                /* We don't know the mode.  So do nothing */
                cmd_failure("set_op_mode_cb", NULL);
        }

        if (cb)
                cb(status);

        return false;
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

static bool read_op_mode_cb(struct at_rsp *rsp, void *up)
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

        return false;
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
static bool join_ap_cb(struct at_rsp *rsp, void *up)
{
        void (*cb)(bool) = up;
        const bool status = at_ok(rsp);

        if (!status)
                cmd_failure("join_ap_cb", NULL);

        if (cb)
                cb(status);

        return false;
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
        if (pass && *pass)
                snprintf(cmd, ARRAY_LEN(cmd), "AT+CWJAP=\"%s\",\"%s\"",
                         ssid, pass);
        else
                snprintf(cmd, ARRAY_LEN(cmd), "AT+CWJAP=\"%s\",", ssid);

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

/**
 * Extracts the wanted strings reply from the +CWJAP response.
 */
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
static bool get_client_ap_cb(struct at_rsp *rsp, void *up)
{
        static const char *cmd_name = "get_ap_cb";
        void (*cb)(bool, const struct esp8266_client_info*) = up;
        bool status = at_ok(rsp);

        if (!status) {
                cmd_failure(cmd_name, NULL);
                goto do_cb;
        }

        char *client_info = rsp->msgs[rsp->msg_count - 2];
        if (!parse_client_info(client_info)) {
                cmd_failure(cmd_name, "Failed to parse AP info");
                status = false;
                goto do_cb;
        }

do_cb:
        if (cb)
                cb(status, &state.client_info);

        return false;
}

/**
 * Use this to figure out the info about the client AP.  Gets both the
 * client network SSID and the AP MAC address.
 * @param cb The callback to be invoked when the method completes.
 */
bool esp8266_get_client_ap(void (*cb)(bool, const struct esp8266_client_info*))
{
        if (!check_initialized("get_client_ap"))
                return false;

        return NULL != at_put_cmd(state.ati, "AT+CWJAP?", TIMEOUT_SHORT_MS,
                                  get_client_ap_cb, cb);
}

/**
 * Given an AT response for +CIFSR, this method decodes it and extracts
 * the IP address.
 */
static bool parse_client_ip(struct at_rsp *rsp)
{
        /*
         * +CIFSR:STAIP,"192.168.1.94"
         * +CIFSR:STAMAC,"18:fe:34:f4:3a:95"
         */
        char *toks[4];
        char *ip_at_str = NULL;
        for(size_t i = 0; i < rsp->msg_count && NULL == ip_at_str; ++i) {
                char *at_msg_ln = rsp->msgs[i];
                const size_t tok_count =
                        at_parse_rsp_line(at_msg_ln, toks, ARRAY_LEN(toks));

                if (tok_count == 3 && STR_EQ("STAIP", toks[1]))
                        ip_at_str = toks[2];
        }

        if (!ip_at_str)
                return false;

        char *ip_str = at_parse_rsp_str(ip_at_str);
        if (!ip_str)
                return false;

        strncpy(state.client_info.ip, ip_str, ARRAY_LEN(state.client_info.ip));
        return true;
}

/**
 * Callback that is invoked when the get_client_ip command completes.
 */
static bool get_client_ip_cb(struct at_rsp *rsp, void *up)
{
        static const char *cmd_name = "get_client_ip_cb";
        void (*cb)(bool, const struct esp8266_client_info*) = up;
        bool status = at_ok(rsp);

        if (!status) {
                cmd_failure(cmd_name, NULL);
                goto do_cb;
        }

        if (!parse_client_ip(rsp)) {
                status = false;
                cmd_failure(cmd_name, "Failed to parse IP");
                goto do_cb;
        }

do_cb:
        if (cb)
                cb(status, &state.client_info);

        return false;
}

/**
 * Use this to figure out what our IP is as a wireless client.
 * @param cb The callback to be invoked when the method completes.
 */
bool esp8266_get_client_ip(void (*cb)(bool, const struct esp8266_client_info*))
{
        if (!check_initialized("get_ap_mode"))
                return false;

        return NULL != at_put_cmd(state.ati, "AT+CIFSR", TIMEOUT_SHORT_MS,
                                  get_client_ip_cb, cb);
}

/**
 * Gets both the client AP and IP info from the WiFi chip.  Populates
 * the struct and pases back a pointer.
 * @param cb The callback to be invoked when the method completes.
 */
bool esp8266_get_client_info(void (*cb)
                             (bool, const struct esp8266_client_info*))
{
        return esp8266_get_client_ap(NULL) && esp8266_get_client_ip(cb);
}

/**
 *
 * @param cb The callback to be invoked when the method completes.
 */
static bool connect_cb(struct at_rsp *rsp, void *up)
{
        static const char *cmd_name = "connect_cb";
        void (*cb)(bool, const int) = up;
        bool status = at_ok(rsp);
        int chan_id = -1;

        if (!status) {
                cmd_failure(cmd_name, NULL);
                goto do_cb;
        }

        /*
         * <chan_id>,<connection status>
         * 0,CONNECT
         */
        char *toks[3];
        const int tok_cnt = at_parse_rsp_line(rsp->msgs[0], toks,
                                              ARRAY_LEN(toks));

        if (tok_cnt != 2) {
                cmd_failure(cmd_name, "Unexpected # of tokens in response");
                status = false;
                goto do_cb;
        }
        chan_id = atoi(toks[0]);

do_cb:
        if (cb)
                cb(status, chan_id);

        return false;
}

/**
 * Opens a connection to a given destination.
 * @param chan_id The channel ID to use.  0 - 4
 * @param proto The Network protocol to use.
 * @param dest_port The destination port to send data to.
 * @param udp_port The source port from which data originates (UDP ONLY).
 * @param udp_mode Tells the device whether or not the destination of the
 * UDP packets are allowed to change.  0 means it won't change.  1 means it
 * will change up to 1 time.  2 means it can change any number of times.
 * @return true if the request was queued, false otherwise.
 */
bool esp8266_connect(const int chan_id, const enum esp8266_net_proto proto,
                     const char *ip_addr, const int dest_port,
                     const int udp_port, const int udp_mode,
                     void (*cb) (bool, const int))
{
        if (!check_initialized("connect"))
                return false;

        char cmd[64];
        switch (proto) {
        case ESP8266_NET_PROTO_TCP:
                snprintf(cmd, ARRAY_LEN(cmd),
                         "AT+CIPSTART=%d,\"TCP\",\"%s\",%d",
                         chan_id, ip_addr, dest_port);
                break;
        case ESP8266_NET_PROTO_UDP:
                snprintf(cmd, ARRAY_LEN(cmd),
                         "AT+CIPSTART=%d,\"UDP\",\"%s\",%d,%d,%d",
                         chan_id, ip_addr, dest_port, udp_port, udp_mode);
                break;
        }

        return NULL != at_put_cmd(state.ati, cmd, TIMEOUT_LONG_MS,
                                  connect_cb, cb);
}

/**
 * This call back is designed to handle two types of response (since that is the
 * way this wonkey device works).  So based on the state and reply we figure out
 * how to handle things.
 */
static bool send_data_cb(struct at_rsp *rsp, void *up)
{
        const struct tx_info *ti = up;

        switch(rsp->status) {
        case AT_RSP_STATUS_OK:
                /*
                 * If here, then we are ready to send.  We copy straight from
                 * the given pointer instead of copying the message to the
                 * at_cmd struct because the message can be larger than what
                 * that tiny struct can handle.  In a way, this is a poor man's
                 * DMA, but it works well and should allow us to have the same
                 * impact without loosing anything along the way.  We also return
                 * true at the end of this method to indicate to the AT command
                 * state machine that there is still more data to come from this
                 * command.
                 */
                ; /* Silly C legacy issues */
                const Serial *s = state.ati->sb->serial;
                for (size_t i = 0; i < ti->len; ++i)
                        s->put_c(ti->data[i]);

                return true;
        case AT_RSP_STATUS_SEND_OK:
                /* Then we have successfully sent the message */
                if (ti->cb)
                        ti->cb(true);

                return false;
        default:
                /* Then bad things happened */
                cmd_failure("send_data_cb", "Bad response value");
                if (ti->cb)
                        ti->cb(false);

                return false;
        }
}

/**
 * Use this to figure out what our IP is as a wireless client.
 * @param cb The callback to be invoked when the method completes.
 */
bool esp8266_send_data(const int chan_id, const char *data,
                       const size_t len, void (*cb)(bool))
{
        if (!check_initialized("send_data"))
                return false;

        /*
         * Set the state before beginning the command.  This is a 2 part
         * command with a fair bit of data, thus we need to use some
         * internal state to handle it all.
         */
        struct tx_info *ti = &state.tx_info;
        ti->data = data;
        ti->len = len;
        ti->cb = cb;

        char cmd[32];
        snprintf(cmd, ARRAY_LEN(cmd),"AT+CIPSEND=%d,%d", chan_id, len);

        return NULL != at_put_cmd(state.ati, cmd, TIMEOUT_LONG_MS,
                                  send_data_cb, ti);
}
