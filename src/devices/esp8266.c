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
#include "esp8266.h"
#include "macros.h"
#include "mem_mang.h"
#include "net/protocol.h"
#include "printk.h"
#include "serial_buffer.h"
#include "str_util.h"
#include "taskUtil.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define _AT_CMD_DELIM		"\r\n"
#define _AT_DEFAULT_QP_MS	250
#define _AT_QP_PRE_INIT_MS     	500
#define _AT_QP_STANDARD_MS	1	/* Can probably be 0 */
#define _AUTOBAUD_TRIES		3
#define _TIMEOUT_LONG_MS	5000
#define _TIMEOUT_MEDIUM_MS	500
#define _TIMEOUT_SHORT_MS	50
#define _TIMEOUT_SUPER_MS	30000

static const enum log_level serial_dbg_lvl = INFO;

/* STIEG: Temp until we write *_create methods for serial_buff and at_info */
struct serial_buffer _serial_cmd_buff;
struct at_info _ati;

/**
 * Internal state of our driver.
 */
static struct {
        struct serial_buffer *scb;
        struct at_info *ati;
        void (*init_cb)(enum dev_init_state); /* STIEG: Put in own struct? */
        enum dev_init_state init_state;
        struct esp8266_event_hooks hooks;
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
 * Callback that gets invoked when we are unable to handle the URC using
 * the standard URC callbacks.  Used for the silly messages like
 * `0,CONNECT` where there is no prefix for the URC like their should be.
 * Messages this callback handles:
 * * <0-4>,CONNECT
 * * <0-4>,CLOSED
 * * WIFI DISCONNECT
 */
static bool sparse_urc_cb(char* msg)
{
        msg = strip_inline(msg);

        if (STR_EQ(msg, "WIFI CONNECTED")  ||
            STR_EQ(msg, "WIFI DISCONNECT") ||
            STR_EQ(msg, "WIFI GOT IP")) {
                if (state.hooks.client_state_changed_cb)
                        state.hooks.client_state_changed_cb(msg);

                return true;
        }

        /* Now look for a message with a comma */
        char* comma = strchr(msg, ',');
        if (!comma)
                return false;

        *comma = '\0';
        const char* m1 = msg;
        const char* m2 = ++comma;

        enum socket_action action = SOCKET_ACTION_UNKNOWN;
        if (STR_EQ(m2, "CONNECT"))
                action = SOCKET_ACTION_CONNECT;

        if (STR_EQ(m2, "CLOSED"))
                action = SOCKET_ACTION_DISCONNECT;

        if (state.hooks.socket_state_changed_cb)
                state.hooks.socket_state_changed_cb(atoi(m1), action);

        return true;
}

/**
 * Sets up the internal state of the driver.  Must be called before init.
 */
static bool _setup(struct Serial *s, const size_t max_cmd_len)
{
        /* Check if already initialized.  If so, do nothing */
        if (state.ati)
                return true;

        state.ati = &_ati;
        state.scb = &_serial_cmd_buff;

        /*
         * Initialize the serial command buffer.  This buffer is used for
         * bi-directional command level communication between the serial
         * tx/rx buffers and our AT state machine.  Commands may not
         * exceed max_cmd_len.
         */
        if (!serial_buffer_create(state.scb, s, max_cmd_len, NULL))
                return false;

        /* Init our AT engine here */
        if (!init_at_info(state.ati, state.scb, _AT_DEFAULT_QP_MS,
                          _AT_CMD_DELIM, sparse_urc_cb))
                return false;

        return true;
}

void esp8266_do_loop(const size_t timeout)
{
        /* If not initialized, just simulate the delay. */
        if (!state.ati) {
                delayMs(timeout);
                return;
        }

        at_task(state.ati, timeout);
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
        return NULL != at_put_cmd(state.ati, "AT+GMR", _TIMEOUT_MEDIUM_MS,
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
        return NULL != at_put_cmd(state.ati, cmd, _TIMEOUT_SHORT_MS,
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
        return NULL != at_put_cmd(state.ati, cmd_str, _TIMEOUT_SHORT_MS,
                                  set_mux_mode_cb, cb);
}

static void do_remaining_init_tasks()
{
        /* Set normal quiet period since we have auto-bauded */
        at_configure_device(state.ati, _AT_QP_STANDARD_MS,
                            _AT_CMD_DELIM);

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
        /* Check if we got an OK reply.  If so, move to next step */
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

        at_put_cmd(state.ati, "AT", _TIMEOUT_MEDIUM_MS, autobaud_cb, tries);
        return false;
}

/**
 * Kicks off the autobauding process.  We configure the AT engine to use
 * the callback method for both call backs and the initilazation routine.
 * Saves us a method.  Its like recursion, except without the stack overflows.
 */
static void begin_autobaud_cmd()
{
        at_configure_device(state.ati, _AT_QP_PRE_INIT_MS,
                            _AT_CMD_DELIM);
        autobaud_cb(NULL, (void*) _AUTOBAUD_TRIES);
}

static bool is_init_in_progress()
{
        return NULL != state.init_cb;
}

/**
 * Kicks off the initilization process.
 */
bool esp8266_init(struct Serial *s, const size_t max_cmd_len,
                  void (*cb)(enum dev_init_state))
{
        if (!cb || !s || is_init_in_progress())
                return false;

        /* Init objects.  If fail, then nothing we can do. */
        if (!_setup(s, max_cmd_len))
                return false;

        state.init_cb = cb;
        state.init_state = DEV_INIT_INITIALIZING;
        begin_autobaud_cmd();

        return true;
}

/**
 * @return The initialization state of the device.
 */
enum dev_init_state esp8266_get_dev_init_state()
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

        if (!state.ati || !init)
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
        return NULL != at_put_cmd(state.ati, cmd_str, _TIMEOUT_MEDIUM_MS,
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

        return NULL != at_put_cmd(state.ati, "AT+CWMODE?", _TIMEOUT_SHORT_MS,
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

        return NULL != at_put_cmd(state.ati, cmd, _TIMEOUT_SUPER_MS,
                                  join_ap_cb, cb);
}

void esp8266_log_client_info(const struct esp8266_client_info *info)
{
        pr_info("[esp8266] WiFi Client info:\r\n");
        const bool connected = info->has_ap;
        const char* conn_str = connected ? "Connected" : "Disconnected";
        pr_info_str_msg("\tStatus: ", conn_str);

        if (!connected)
                return;

        pr_info_str_msg("\t  SSID: ", info->ssid);
        pr_info_str_msg("\tAP MAC: ", info->mac);
        pr_info_str_msg("\t    IP: ", info->ip);
}

/**
 * Extracts the wanted strings reply from the +CWJAP response.
 */
static bool parse_client_info(char *rsp,
                              struct esp8266_client_info* const info)
{
        info->has_ap = false;

        /*
         * Success case:
         * > +CWJAP:"madworks","2a:a4:3c:6d:46:37",6,-53
         * No AP case:
         * > No AP
         */
        char *toks[6];
        const size_t tok_cnt = at_parse_rsp_line(rsp, toks, ARRAY_LEN(toks));
        switch(tok_cnt) {
        case 1:
                /* No AP */
                return true;
        case 5:
                break;
        default:
                return false;
        }

        info->has_ap = true;
        const char *ssid = at_parse_rsp_str(toks[1]);
        const char *mac = at_parse_rsp_str(toks[2]);
        if (!ssid || !mac)
                return false;

        strncpy(info->ssid, ssid, ARRAY_LEN(info->ssid));
        strncpy(info->mac, mac, ARRAY_LEN(info->mac));
        info->has_ap = true;

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

        struct esp8266_client_info ci;
        char *client_info_rsp = rsp->msgs[rsp->msg_count - 2];
        if (!parse_client_info(client_info_rsp, &ci)) {
                cmd_failure(cmd_name, "Failed to parse AP info");
                status = false;
                goto do_cb;
        }

do_cb:
        if (cb)
                cb(status, &ci);

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

        return NULL != at_put_cmd(state.ati, "AT+CWJAP?", _TIMEOUT_SHORT_MS,
                                  get_client_ap_cb, cb);
}

/**
 * Given an AT response for +CIFSR, this method decodes it and extracts
 * the IP address.
 */
static const char* parse_client_ip(struct at_rsp *rsp)
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

        return ip_at_str ? at_parse_rsp_str(ip_at_str) : NULL;
}

/**
 * Callback that is invoked when the get_client_ip command completes.
 */
static bool get_client_ip_cb(struct at_rsp *rsp, void *up)
{
        static const char *cmd_name = "get_client_ip_cb";
        void (*cb)(bool, const char*) = up;
        bool status = at_ok(rsp);
        const char* ip_str = NULL;

        if (!status) {
                cmd_failure(cmd_name, NULL);
                goto do_cb;
        }

        ip_str = parse_client_ip(rsp);
        if (NULL == ip_str) {
                status = false;
                cmd_failure(cmd_name, "Failed to parse IP");
                goto do_cb;
        }

do_cb:
        if (cb)
                cb(status, ip_str);

        return false;
}

/**
 * Use this to figure out what our IP is as a wireless client.
 * @param cb The callback to be invoked when the method completes.
 */
bool esp8266_get_client_ip(void (*cb)(bool, const char*))
{
        if (!check_initialized("get_ap_mode"))
                return false;

        return NULL != at_put_cmd(state.ati, "AT+CIFSR", _TIMEOUT_SHORT_MS,
                                  get_client_ip_cb, cb);
}

/**
 * The callback invoked by our AT state machine upon the completion
 * of the command
 */
static bool get_ap_info_cb(struct at_rsp *rsp, void *up)
{
        static const char *cmd_name = "get_ap_info_cb";
        esp8266_get_ap_info_cb_t *cb = up;
        struct esp8266_ap_info ap_info;
        memset(&ap_info, 0, sizeof(struct esp8266_ap_info));

        bool status = at_ok(rsp);
        if (!status) {
                cmd_failure(cmd_name, NULL);
                goto do_cb;
        }

        /*
         * AT+CWSAP_DEF=<ssid>,<pwd>,<chl>,<ecn>
         * ssid -> string
         * pwd  -> string
         * chl  -> number
         * ecn  -> number
         */
        char *toks[6];
        const int tok_cnt = at_parse_rsp_line(rsp->msgs[0], toks,
                                              ARRAY_LEN(toks));

        if (tok_cnt != 5) {
                cmd_failure(cmd_name, "Unexpected # of tokens in response");
                status = false;
                goto do_cb;
        }

        strncpy(ap_info.ssid, at_parse_rsp_str(toks[1]),
                ARRAY_LEN(ap_info.ssid));
        strncpy(ap_info.password, at_parse_rsp_str(toks[2]),
                ARRAY_LEN(ap_info.password));
        ap_info.channel = atoi(toks[3]);
        ap_info.encryption = (enum esp8266_encryption) atoi(toks[4]);

do_cb:
        if (cb)
                cb(status, &ap_info);

        return false;
}

bool esp8266_get_ap_info(esp8266_get_ap_info_cb_t *cb)
{
        if (!check_initialized("get_ap_info"))
                return false;

        const char cmd[] = "AT+CWSAP_DEF?";
        return NULL != at_put_cmd(state.ati, cmd, _TIMEOUT_MEDIUM_MS,
                                  get_ap_info_cb, cb);
}

/**
 * Callback that gets invoked when the set_ap_info command completes.
 */
static bool set_ap_info_cb(struct at_rsp *rsp, void *up)
{
        static const char *cmd_name = "set_ap_info_cb";
        esp8266_set_ap_info_cb_t *cb = up;

        bool status = at_ok(rsp);
        if (!status) {
                cmd_failure(cmd_name, NULL);
        }

        if (cb)
                cb(status);

        return false;
}

bool esp8266_set_ap_info(const struct esp8266_ap_info* info,
                         esp8266_set_ap_info_cb_t *cb)
{
        if (!check_initialized("set_ap_info") || NULL == info)
                return false;

        char cmd[64];
        snprintf(cmd, ARRAY_LEN(cmd), "AT+CWSAP_CUR=\"%s\",\"%s\",%d,%d",
                 info->ssid, info->password, (int) info->channel,
                 info->encryption);

        return NULL != at_put_cmd(state.ati, cmd, _TIMEOUT_LONG_MS,
                                  set_ap_info_cb, cb);

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
 * @return true if the request was queued, false otherwise.
 */
bool esp8266_connect(const int chan_id, const enum protocol proto,
                     const char *ip_addr, const int dest_port,
                     void (*cb) (bool, const int))
{
        if (!check_initialized("connect"))
                return false;

        char cmd[64];
        const char* proto_str;
        switch (proto) {
        case PROTOCOL_TCP:
                proto_str = "TCP";
                break;
        case PROTOCOL_UDP:
                proto_str = "UDP";
                break;
        default:
                cmd_failure("esp8266_connect", "Invalid protocol");
                return false;
        }

        snprintf(cmd, ARRAY_LEN(cmd), "AT+CIPSTART=%d,\"%s\",\"%s\",%d",
                 chan_id, proto_str, ip_addr, dest_port);

        return NULL != at_put_cmd(state.ati, cmd, _TIMEOUT_LONG_MS,
                                  connect_cb, cb);
}

struct tx_info {
        struct Serial *serial;
        size_t len;
        void (*cb)(int);
};

/**
 * This call back is special in that it handle two types of response.
 * Since the send_data command for the esp8266 is a two step command,
 * this method needs to be able to handle both reply types from the
 * Esp8266 modem.  The first reply type should be AT_RSP_STATUS_OK,
 * which means we are ready to send the message.  At this point we
 * put the message on the serial line and return `true`, indicating
 * that there are more replies to be had.  When the entierty of the
 * message has been sent, the modem will return AT_RSP_STATUS_SEND_OK
 * and we will be done.  Otherwise it may return some other status, at
 * which point we will deem the attempted failed and escape.
 */
static bool send_data_cb(struct at_rsp *rsp, void *up)
{
        struct tx_info *ti = up;
        int status = -1;

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
                struct Serial *s = state.ati->sb->serial;
                xQueueHandle q = serial_get_tx_queue(ti->serial);
                char c;
                for (size_t i = 0; i < ti->len; ++i) {
                        if (!xQueueReceive(q, &c, 0))
                            c = 0;

                        serial_put_c(s, c);
                }

                return true;
        case AT_RSP_STATUS_SEND_OK:
                /* Then we have successfully sent the message */
                status = ti->len;
                goto fini;
        default:
                /* Then bad things happened */
                cmd_failure("send_data_cb", "Bad response value");
                status = -1;
                goto fini;
        }

fini:
        if (ti->cb)
                ti->cb(status);

        portFree(ti);
        return false;
}

/**
 * Use this to figure out what our IP is as a wireless client.
 * @param cb The callback to be invoked when the method completes.
 */
bool esp8266_send_data(const int chan_id, struct Serial *serial,
                       const size_t len, void (*cb)(int))
{
        const char* cmd_name = "send_data";
        if (!check_initialized(cmd_name))
                return false;

        if (0 == len) {
                cmd_failure(cmd_name, "Can't send empty messages");
                return false;
        }

        /*
         * Set the state before beginning the command.  This is a 2 part
         * command with a fair bit of data, thus we need to use some
         * internal state to handle it all.  This data is free'd at the
         * end of the command in the send_data_cb above when the command
         * is done.
         */
        struct tx_info *ti = portMalloc(sizeof(struct tx_info));
        if (!ti) {
                /* Malloc failure.  Le-sigh */
                cmd_failure(cmd_name, "Malloc failed for send procedure.");
                return false;
        }

        ti->serial = serial;
        ti->len = len;
        ti->cb = cb;

        char cmd[32];
        snprintf(cmd, ARRAY_LEN(cmd),"AT+CIPSEND=%d,%d", chan_id, (int) len);

        return NULL != at_put_cmd(state.ati, cmd, _TIMEOUT_LONG_MS,
                                  send_data_cb, ti);
}


/**
 * Callback that is invoked when the get_client_ip command completes.
 */
static bool server_cmd_cb(struct at_rsp *rsp, void *up)
{
        static const char *cmd_name = "server_cmd_cb";
        void (*cb)(bool) = up;
        bool status = at_ok(rsp);

        if (!status)
                cmd_failure(cmd_name, NULL);

        if (cb)
                cb(status);

        return false;
}

/**
 * Use this command to start up the TCP server on the WiFi chip.
 * @param action Pass in the action enum to control the action taken.
 * @param port The port to listen on.
 * @param cb The callback to be invoked when the method completes.
 */
bool esp8266_server_cmd(const enum esp8266_server_action action, int port,
                        void (*cb)(bool))
{
        if (!check_initialized("server_cmd"))
                return false;

        char cmd[32];
        snprintf(cmd, ARRAY_LEN(cmd),"AT+CIPSERVER=%d,%d", action, port);

        return NULL != at_put_cmd(state.ati, cmd, _TIMEOUT_MEDIUM_MS,
                                  server_cmd_cb, cb);
}

/**
 * This is the callback invoked when an IPD URC is seen.
 */
static bool ipd_urc_cb(struct at_rsp *rsp, void *up)
{
        static const char *cmd_name = "ipd_urc_cb";
        if (AT_RSP_STATUS_NONE != rsp->status) {
                cmd_failure(cmd_name, "Unexpected response status");
                return false;
        }

        /* +IPD,<id>,<len>:<data> */
        char *toks[4];
        const int tok_cnt = at_parse_rsp_line(rsp->msgs[0], toks,
                                              ARRAY_LEN(toks));
        if (tok_cnt != 4) {
                cmd_failure(cmd_name, "Unexpected # of tokens in response");
                return false;
        }

        const int chan_id = atoi(toks[1]);
        const size_t len = atoi(toks[2]);
        const char *msg = toks[3];

        if (state.hooks.data_received_cb)
                state.hooks.data_received_cb(chan_id, len, msg);

        return false;
}

bool esp8266_register_callbacks(const struct esp8266_event_hooks* hooks)
{
        memcpy(&state.hooks, hooks, sizeof(struct esp8266_event_hooks));

        /* Control flags for special handling of response messages */
        const enum at_urc_flags flags =
                AT_URC_FLAGS_NO_RSP_STATUS |
                AT_URC_FLAGS_NO_RSTRIP;

        return NULL != at_register_urc(state.ati, "+IPD,", flags,
                                       ipd_urc_cb, NULL);
}
