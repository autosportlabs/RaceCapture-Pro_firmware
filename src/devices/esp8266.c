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
#include "at_basic.h"
#include "esp8266.h"
#include "macros.h"
#include "mem_mang.h"
#include "net/protocol.h"
#include "printk.h"
#include "serial.h"
#include "serial_buffer.h"
#include "str_util.h"
#include "taskUtil.h"
#include "wifi_device.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define AT_PROBE_TRIES		3
#define AT_PROBE_DELAY_MS	200
#define LOG_PFX	"[esp8266] "
#define ESP8266_CMD_DELIM      	"\r\n"
#define ESP8266_DEV_FLAGS	(AT_DEV_CFG_FLAG_RUDE)
#define ESP8266_QP_MS	1
#define _TIMEOUT_LONG_MS	5000
#define _TIMEOUT_MEDIUM_MS	500
#define _TIMEOUT_SHORT_MS	50
#define _TIMEOUT_SUPER_MS	30000


/* STIEG: Temp until we write *_create methods for serial_buff and at_info */
struct serial_buffer _serial_cmd_buff;
struct at_info _ati;

/**
 * Internal state of our driver.
 */
static struct {
        struct serial_buffer *scb;
        struct at_info *ati;
	struct {
		esp8266_init_cb_t* cb;
		enum dev_init_state state;
	} init;
        struct esp8266_event_hooks hooks;
} state;

static void cmd_failure(const char *cmd_name, const char *msg)
{
        pr_warning(LOG_PFX);
        pr_warning(cmd_name);
        if (msg) {
                pr_info_str_msg(" failed with msg: ", msg);
        } else {
                pr_warning(" failed.\r\n");
        }
}

/**
 * This is the callback invoked when an IPD URC is seen.
 */
static void ipd_urc_cb(char *msg)
{
	if (!state.hooks.data_received_cb)
		return;

	static const char *cmd_name = "ipd_urc_cb";

        /* +IPD,<id>,<len>:<data> */
        char *toks[4];
        const int tok_cnt = at_parse_rsp_line(msg, toks, ARRAY_LEN(toks));
        if (tok_cnt != 4) {
                cmd_failure(cmd_name, "Unexpected # of tokens in response");
                return;
        }

        const int chan_id = atoi(toks[1]);
        const size_t len = atoi(toks[2]);
        char *data = toks[3];
	const size_t serial_len = strlen(data);


	/*
	 * Check that we at least got the number of characters expected.
	 * Clip the message at the given len b/c sometimes esp8266 adds an
	 * extra \r\n at the end.  This is actually a bug in how we process
	 * messages from esp8266 since we always expect AT messages to end
	 * in \r\n and the IPD URC does not always do this (the extra \r\n
	 * comes from the command messages.
	 */
	if (serial_len < len) {
		pr_error(LOG_PFX "IPD Length Mismatch.  Dropping. \r\n");
		pr_info_int_msg(LOG_PFX "Length Expected: ", (int) len);
		pr_info_int_msg(LOG_PFX "Length Actual: ", (int) serial_len);
		pr_info_str_msg(LOG_PFX "Data: ", data);
		return;
	}
	data[len] = 0;


	/* Check twice to ensure that it wasn't unset after first check */
	if (state.hooks.data_received_cb)
		state.hooks.data_received_cb(chan_id, len, data);
}

static void wifi_action_callback(const char* msg)
{
	/* Check for the hook */
	if (!state.hooks.client_state_changed_cb)
		return;

	enum client_action action;
	/* Look at our 5th character to determin our action */
	switch (msg[5]) {
	case 'C':
		/* WIFI CONNECT */
		action = CLIENT_ACTION_CONNECT;
		break;
	case 'D':
		/* WIFI DISCONNECT */
		action = CLIENT_ACTION_DISCONNECT;
		break;
	case 'G':
		/* WIFI GOT IP */
		action = CLIENT_ACTION_GOT_IP;
		break;
	default:
		pr_warning_str_msg(LOG_PFX "Unknown Wifi Action: ", msg);
		action = CLIENT_ACTION_UNKNOWN;
		break;
	}

	state.hooks.client_state_changed_cb(action);
}

static bool channel_action_cb(const char* msg)
{
        /* Now look for a message format <0-4>,MSG */
        char* comma = strchr(msg, ',');
        if (comma != msg + 1)
                return false;

        *comma = '\0';
        const char* m1 = msg;
        const char* m2 = comma + 1;

        enum socket_action action = SOCKET_ACTION_UNKNOWN;
        if (STR_EQ(m2, "CONNECT\r\n"))
                action = SOCKET_ACTION_CONNECT;

        if (STR_EQ(m2, "CLOSED\r\n"))
                action = SOCKET_ACTION_DISCONNECT;

	if (STR_EQ(m2, "CONNECT FAIL\r\n"))
                action = SOCKET_ACTION_CONNECT_FAIL;

        if (state.hooks.socket_state_changed_cb &&
	    action != SOCKET_ACTION_UNKNOWN) {
                state.hooks.socket_state_changed_cb(atoi(m1), action);
		return true;
	}

	return false;

}

/**
 * Callback that gets invoked when we are unable to handle the URC using
 * the standard URC callbacks.  Used for the silly messages like
 * `0,CONNECT` where there is no prefix for the URC like their should be.
 */
static bool sparse_urc_cb(char* msg)
{
	/* + IPD,... - Incoming data. */
	if (strncmp(msg, "+IPD,", 5) == 0) {
		ipd_urc_cb(msg);
		return true;
	}

	/* WIFI ... - Device actions */
        if (strncmp(msg, "WIFI ", 5) == 0) {
		wifi_action_callback(msg);
		return true;
	}

	/* <0-4>,... - Socket actions */
	return channel_action_cb(msg);
}

/**
 * Sets up the internal state.  Must be called first and only once. All other
 * methods will fail if this is not called.
 * @param serial The serial device to use.
 * @param mad_cmd_length Maximum command length we shall see.
 * @param true if successful setup occurred, false otherwise.
 */
bool esp8266_setup(struct Serial *serial, const size_t max_cmd_len)
{
        /* Check if already initialized. */
        if (state.ati)
                return false;

        state.ati = &_ati;
        state.scb = &_serial_cmd_buff;

        /*
         * Initialize the serial command buffer.  This buffer is used for
         * bi-directional command level communication between the serial
         * tx/rx buffers and our AT state machine.  Commands may not
         * exceed max_cmd_len.
         */
        if (!serial_buffer_create(state.scb, serial, max_cmd_len, NULL))
                return false;

        /* Init our AT engine here */
        if (!at_info_init(state.ati, state.scb))
                return false;

	at_configure_device(state.ati, ESP8266_QP_MS, ESP8266_CMD_DELIM,
			    ESP8266_DEV_FLAGS);
	at_set_sparse_urc_cb(state.ati, sparse_urc_cb);

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

bool esp8266_set_default_serial_params(struct Serial* serial)
{
	return serial_config(serial, ESP8266_SERIAL_DEF_BITS,
			     ESP8266_SERIAL_DEF_PARITY,
			     ESP8266_SERIAL_DEF_STOP,
			     ESP8266_SERIAL_DEF_BAUD);
}

static void init_complete(const bool success)
{
	/* Only do the callback once */
	esp8266_init_cb_t* cb = state.init.cb;
	state.init.cb = NULL;

	if (success) {
		pr_info(LOG_PFX "Initialized\r\n");
		state.init.state = DEV_INIT_STATE_READY;
	} else {
		state.init.state = DEV_INIT_STATE_FAILED;
	}

	if (cb)
		cb(success);
}

/**
 * Call the if the init routine fails.
 * @param msg The message to print.
 */
static void init_failed(const char *msg)
{
	cmd_failure("Init", msg);
        init_complete(false);
}

static bool init_get_version_cb(struct at_rsp *rsp, void *up)
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
        pr_info(LOG_PFX "Version info:\r\n");
        for (size_t i = 0; i < rsp->msg_count - 1; ++i)
                pr_info_str_msg("\t", rsp->msgs[i]);

        return false;
}

static bool init_get_version()
{
        return NULL != at_put_cmd(state.ati, "AT+GMR", _TIMEOUT_MEDIUM_MS,
                                  init_get_version_cb, NULL);
}

static bool init_set_echo_cb(struct at_rsp *rsp, void *up)
{
        if (!at_ok(rsp))
                init_failed("Set echo failed.");

        return false;
}

static bool init_set_echo()
{
        const char *cmd = "ATE0";
        return NULL != at_put_cmd(state.ati, cmd, _TIMEOUT_SHORT_MS,
                                  init_set_echo_cb, NULL);
}

static bool init_set_mux_mode_cb(struct at_rsp *rsp, void *up)
{
        const bool status = at_ok(rsp);
        if (!status) {
                init_failed("set_mux_mode_cb");
	} else {
		init_complete(true);
	}

        return false;
}

static bool init_set_mux_mode()
{
        const char cmd_str[] = "AT+CIPMUX=1";
        return NULL != at_put_cmd(state.ati, cmd_str, _TIMEOUT_SHORT_MS,
                                  init_set_mux_mode_cb, NULL);
}

/**
 * Callback that is invoked upon command completion.
 */
static bool init_soft_reset_cb(struct at_rsp *rsp, void *up)
{
        const bool status = at_ok(rsp);
        if (!status) {
		init_failed("Soft reset failed.");
		return false;
	}

	/* Reset Serial to default values first */
	struct Serial* serial = state.ati->sb->serial;
	serial_clear(serial);
	esp8266_set_default_serial_params(serial);

	/* Wait for the ready notice from the modem and ping test. */
	esp8266_wait_for_ready(serial);
	if (!at_basic_ping(serial, AT_PROBE_TRIES, AT_PROBE_DELAY_MS)) {
		init_failed("Post reset ping failed\r\n");
		return false;
	}


	/*
	 * If here, queue up reset of init tasks.  Use single &
	 * to prevent short-circuiting of commands should one fail
	 * to queue.
	 */
	const bool queued =
		init_set_echo() &&
		init_get_version() &&
                init_set_mux_mode();
	if (!queued)
		init_failed("Failed to queue up all init tasks\r\n");

        return false;
}

/**
 * Soft reset the device back to its initial state.
 * @param cb The callback to be invoked when the method completes.
 * @return true if the request was queued, false otherwise.
 */
static bool init_soft_reset()
{
        const char cmd[] = "AT+RST";
        return NULL != at_put_cmd(state.ati, cmd, _TIMEOUT_LONG_MS,
                                  init_soft_reset_cb, NULL);
}

/**
 * Resets and initializes our WiFi device to a known sane state where
 * the driver can then take over and control sanely.
 * @param cb Callback to invoke when init is complete.
 */
bool esp8266_init(esp8266_init_cb_t* cb)
{
	const char task_name[] = "esp8266_init";
        if (!state.ati) {
		cmd_failure(task_name, "AT subsys not initialized");
                return false;
	}

	if (state.init.cb) {
		cmd_failure(task_name, "Init already in progress");
		return false;
	}

	state.init.cb = cb;

	/*
	 * To ensure our device is initialized properly we do the
	 * following in this order:
	 *
	 * + Soft Reset
	 * + Print Version Info
	 * + Disable Cmd Echo
	 * + Setup Muxing
	 *
	 * This gaurantees sane initialized state for the WiFi device.
	 */
        return init_soft_reset();
}

/**
 * @return The initialization state of the device.
 */
enum dev_init_state esp8266_get_dev_init_state()
{
        return state.init.state;
}

/**
 * Allows us to quickly check if the device has successfully initialized.
 * Notifies the log if it isn't initialized.
 * @return true if initialized, false otherwise.
 */
static bool check_initialized(const char *cmd_name)
{
        const bool init = DEV_INIT_STATE_READY == state.init.state;

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

        char cmd_str[24];
        snprintf(cmd_str, ARRAY_LEN(cmd_str), "AT+CWMODE_DEF=%d", (int) mode);
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

        char *toks[3];
        const size_t tok_cnt =
                at_parse_rsp_line(rsp->msgs[0], toks, ARRAY_LEN(toks));
        if (tok_cnt != 2) {
                cmd_failure(cmd_name, "Incorrect number of tokens parsed.");
                status = false;
                goto do_cb;
        }

        /* If here, parse the number.  It should match our enum*/
        const int val = atoi(toks[1]);
        pr_debug_int_msg(LOG_PFX "OP mode: ", val);
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

        return NULL != at_put_cmd(state.ati, "AT+CWMODE_DEF?",
                                  _TIMEOUT_SHORT_MS, read_op_mode_cb, cb);
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
        const char pfx[] = "AT+CWJAP_DEF";
        if (pass && *pass) {
                snprintf(cmd, ARRAY_LEN(cmd), "%s=\"%s\",\"%s\"",
                         pfx, ssid, pass);
        } else {
                snprintf(cmd, ARRAY_LEN(cmd), "%s=\"%s\",", pfx, ssid);
        }

        return NULL != at_put_cmd(state.ati, cmd, _TIMEOUT_SUPER_MS,
                                  join_ap_cb, cb);
}

void esp8266_log_client_info(const struct esp8266_client_info *info)
{
        pr_info(LOG_PFX "WiFi Client info:\r\n");
        const bool connected = info->has_ap;
        const char* conn_str = connected ? "Connected" : "Disconnected";
        pr_info_str_msg("\tStatus: ", conn_str);

        if (!connected)
                return;

        pr_info_str_msg("\t  SSID: ", info->ssid);
        pr_info_str_msg("\tAP MAC: ", info->mac);
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
        case 0:
        case 2: /* Then we fail */
                return false;
        }

        /* If here, we have 3 or more tokens. */
        info->has_ap = true;
        const char *ssid = at_parse_rsp_str(toks[1]);
        const char *mac = at_parse_rsp_str(toks[2]);
        if (!ssid || !mac)
                return false;

        strntcpy(info->ssid, ssid, ARRAY_LEN(info->ssid));
        strntcpy(info->mac, mac, ARRAY_LEN(info->mac));
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

        return NULL != at_put_cmd(state.ati, "AT+CWJAP_DEF?",
                                  _TIMEOUT_SHORT_MS, get_client_ap_cb, cb);
}

/**
 * Callback that is invoked when the get_client_ip command completes.
 */
static bool get_ip_info_cb(struct at_rsp *rsp, void *up)
{
        static const char *cmd_name = "get_ip_info_cb";
        get_ip_info_cb_t* cb = up;
        bool status = at_ok(rsp);

        struct esp8266_ipv4_info client_info;
        struct esp8266_ipv4_info station_info;
        memset(&client_info, 0, sizeof(struct esp8266_ipv4_info));
        memset(&station_info, 0, sizeof(struct esp8266_ipv4_info));

        if (!status) {
                cmd_failure(cmd_name, NULL);
                goto do_cb;
        }

        /*
         * +CIFSR:APIP,"192.168.4.1"
         * +CIFSR:APMAC,"1a:fe:34:f4:3a:95"
         * +CIFSR:STAIP,"192.168.1.94"
         * +CIFSR:STAMAC,"18:fe:34:f4:3a:95"
         */
        char *toks[4];
        for(size_t i = 0; i < rsp->msg_count; ++i) {
                const size_t tok_count = at_parse_rsp_line(rsp->msgs[i], toks,
                                                           ARRAY_LEN(toks));
                if (tok_count != 3)
                        continue;

                struct esp8266_ipv4_info* info = NULL;
                if (STR_EQ("STAIP", toks[1]))
                        info = &client_info;

                if (STR_EQ("APIP", toks[1]))
                        info = &station_info;

                if (!info)
                        continue;

                const char* ip_str = at_parse_rsp_str(toks[2]);
                strncpy(info->address, ip_str, ARRAY_LEN(info->address));
        }

do_cb:
        if (cb)
                cb(status, &client_info, &station_info);

        return false;
}

/**
 * Use this to figure out IP information of our
 * @param cb The callback to be invoked when the method completes.
 */
bool esp8266_get_ip_info(get_ip_info_cb_t callback)
{
        if (!check_initialized("get_ip_info"))
                return false;

        return NULL != at_put_cmd(state.ati, "AT+CIFSR", _TIMEOUT_SHORT_MS,
                                  get_ip_info_cb, callback);
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
         * AT+CWSAP=<ssid>,<pwd>,<chl>,<ecn>
         * ssid -> string
         * pwd  -> string
         * chl  -> number
         * ecn  -> number
         * [max conn] -> number
         * [ssid hidden] -> number
         */
        char *toks[8];
        const int tok_cnt = at_parse_rsp_line(rsp->msgs[0], toks,
                                              ARRAY_LEN(toks));

        if (tok_cnt < 5 || tok_cnt > 7) {
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

        char cmd[80];
        snprintf(cmd, ARRAY_LEN(cmd), "AT+CWSAP_DEF=\"%s\",\"%s\",%d,%d",
                 info->ssid, info->password, (int) info->channel,
                 info->encryption);

        return NULL != at_put_cmd(state.ati, cmd, _TIMEOUT_LONG_MS,
                                  set_ap_info_cb, cb);

}

/**
 * The callback to be invoked when a connect method completes.
 */
static bool connect_cb(struct at_rsp *rsp, void *up)
{
        static const char *cmd_name = "connect_cb";
	const bool in_use = STR_EQ(rsp->msgs[0], "ALREADY CONNECTED");
	const bool status = at_ok(rsp);
        esp8266_connect_cb_t *cb = up;

        if (!status)
                cmd_failure(cmd_name, rsp->msgs[0]);

        if (cb)
                cb(status, in_use);

        return false;
}

/**
 * Opens a TCP connection to a given destination.
 * @param chan_id The channel ID to use.  0 - 4
 * @param addr The destination IP/Name for the connection.
 * @param port The destination port to send data to.
 * @param keepalive The keepalive timeout for TCP. Set to -1 for defualt.
 * @return true if the request was queued, false otherwise.
 */
bool esp8266_connect_tcp(const int chan_id, const char* addr,
                         const int port, const int keepalive,
                         esp8266_connect_cb_t* cb)
{
        if (!check_initialized("connect_tcp"))
                return false;

        char cmd[64];
        if (0 < keepalive) {
                snprintf(cmd, ARRAY_LEN(cmd), "AT+CIPSTART=%d,\"TCP\","
                         "\"%s\",%d,%d",chan_id, addr, port, keepalive);
        } else {
                snprintf(cmd, ARRAY_LEN(cmd), "AT+CIPSTART=%d,\"TCP\","
                         "\"%s\",%d",chan_id, addr, port);
        }

        return NULL != at_put_cmd(state.ati, cmd, _TIMEOUT_LONG_MS,
                                  connect_cb, cb);
}

/**
 * Opens a UDP connection to a given destination.
 * @param chan_id The channel ID to use.  0 - 4
 * @param addr The destination IP/Name for the connection.
 * @param port The destination port.
 * @param src_port Optional source port for UDP.  Set to 0 for default.
 * @param udp_mode Optional UDP mode.  Set to ESP8266_UDP_MODE_NONE to
 * ignore this.  If set, requires src_port be defined.
 * @return true if the request was queued, false otherwise.
 */
bool esp8266_connect_udp(const int chan_id, const char* addr,
                         const int port, const int src_port,
                         const enum esp8266_udp_mode udp_mode,
                         esp8266_connect_cb_t* cb)
{
        if (!check_initialized("connect"))
                return false;

        char cmd[64];
        if (ESP8266_UDP_MODE_NONE != udp_mode) {
                snprintf(cmd, ARRAY_LEN(cmd), "AT+CIPSTART=%d,\"UDP\","
                         "\"%s\",%d,%d,%d", chan_id, addr, port,
                         src_port, udp_mode);
        } else if (src_port) {
                snprintf(cmd, ARRAY_LEN(cmd), "AT+CIPSTART=%d,\"UDP\","
                         "\"%s\",%d,%d", chan_id, addr, port, src_port);
        } else {
                snprintf(cmd, ARRAY_LEN(cmd), "AT+CIPSTART=%d,\"UDP\","
                         "\"%s\",%d", chan_id, addr, port);
        }

        return NULL != at_put_cmd(state.ati, cmd, _TIMEOUT_LONG_MS,
                                  connect_cb, cb);
}

/**
 * Method that is invoked upon the completion of the close method.
 */
static bool close_cb(struct at_rsp *rsp, void *up)
{
        static const char *cmd_name = "close_cb";
        esp8266_close_cb_t* cb = up;
        bool status = at_ok(rsp);

        if (!status)
                cmd_failure(cmd_name, NULL);

        if (cb)
                cb(status);

        return false;
}

/**
 * Closes a connection
 * @param chan_id The channel ID to use.  0 - 4
 * @return true if the request was queued, false otherwise.
 */
bool esp8266_close(const int chan_id, esp8266_close_cb_t* cb)
{
        if (!check_initialized("close"))
                return false;

        char cmd[16];
        snprintf(cmd, ARRAY_LEN(cmd), "AT+CIPCLOSE=%d", chan_id);
        return NULL != at_put_cmd(state.ati, cmd, _TIMEOUT_LONG_MS,
                                  close_cb, cb);
}

struct tx_info {
        struct Serial *serial;
        size_t len;
	size_t sent;
	unsigned int chan_id;
        esp8266_send_data_cb_t* cb;
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
        bool status = false;

        switch(rsp->status) {
        case AT_RSP_STATUS_OK: {
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
		 *
		 * TODO: Make this non-destructive if the send failed. We should
		 * be able to peek at all items on the queue and not remove them
		 * until the send operation was successful. Issue #807
                 */
                struct Serial *s = state.ati->sb->serial;
                xQueueHandle q = serial_get_tx_queue(ti->serial);
		bool underrun = false;

                for (; ti->sent < ti->len; ++ti->sent) {
			char c;
                        if (!xQueueReceive(q, &c, 0)) {
				underrun = true;
				c = INVALID_CHAR; /* Invalid UTF-8 Byte */
			}

                        serial_write_c(s, c);
                }

		if (underrun)
			pr_error(LOG_PFX "BUG: Tx underrun!\r\n");

                return true;
	}
        case AT_RSP_STATUS_SEND_OK:
                /* Then we have successfully sent the message */
		status = true;
                goto fini;
        default:
                /* Then bad things happened */
                cmd_failure("send_data_cb", "Bad response value");
                goto fini;
        }

fini:
        if (ti->cb)
                ti->cb(status, ti->sent, ti->chan_id);

        portFree(ti);
        return false;
}

/**
 * Use this to figure out what our IP is as a wireless client.
 * @param cb The callback to be invoked when the method completes.
 */
bool esp8266_send_data(const unsigned int chan_id, struct Serial *serial,
                       const size_t len, esp8266_send_data_cb_t* cb)
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
        struct tx_info *ti = calloc(sizeof(struct tx_info), 1);
        if (!ti) {
                /* Malloc failure.  Le-sigh */
                cmd_failure(cmd_name, "Malloc failed for send procedure.");
                return false;
        }

        ti->serial = serial;
        ti->len = len;
        ti->cb = cb;
	ti->chan_id = chan_id;

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
 * Use this method to update or unset registered callbacks against this
 * device.
 */
bool esp8266_register_callbacks(const struct esp8266_event_hooks* hooks)
{
        memcpy(&state.hooks, hooks, sizeof(struct esp8266_event_hooks));
	return true;
}

/**
 * Sets the uart configuration on the esp8266 device. This is a raw &
 * synchronous operation since the ESP8266 module has a bug in some of its
 * firmware versions where, upon completion of the command, will not return
 * a proper serial EOL sequence (\r\n). Instead it only returns a carriage
 * return character (\r).  Thus we have to manually do the operation here
 * to account for that anomoly.
 * @note This method does not adjust anything else besides the esp8266
 * device. That is left to the caller to handle.
 * @return True if the operation was successful, false otherwise.
 */
bool esp8266_set_uart_config_raw(const size_t baud, const size_t bits,
				 const size_t parity, const size_t stop_bits)
{
        const char cmd_name[] = "set_uart_config";
        if (!state.ati) {
                cmd_failure(cmd_name, "AT subsys not initialized.");
                return false;
        }

        /* ESP8266 says value of 2 stop bits is 3.  2 is 1.5 stop bits. WTF */
        int stop_bits_adj = (int) stop_bits;
        if (2 == stop_bits_adj)
                stop_bits_adj = 3;

        /*
	 * AT+UART_CUR=<baudrate>,<databits>,<stopbits>,<parity>,<flowctrl>
	 */
	char cmd[48];
        snprintf(cmd, ARRAY_LEN(cmd), "AT+UART_CUR=%d,%d,%d,%d,0%s", (int) baud,
		 (int) bits, stop_bits_adj, (int) parity, ESP8266_CMD_DELIM);

	/* Do it manually because of the ESP8266 Bug */
	struct Serial* serial = state.scb->serial;
	serial_flush(serial);
	serial_write_s(serial, cmd);
	const bool done = at_basic_wait_for_msg(serial, "OK",
						_TIMEOUT_MEDIUM_MS);
	serial_flush(serial);

	return done;
}

bool esp8266_probe_device(struct Serial* serial, const int fast_baud)
{
	const int bauds[] = {fast_baud, ESP8266_SERIAL_DEF_BAUD};
	return at_basic_probe(serial, bauds, ARRAY_LEN(bauds),
			      AT_PROBE_TRIES, AT_PROBE_DELAY_MS,
			      ESP8266_SERIAL_DEF_BITS,
			      ESP8266_SERIAL_DEF_PARITY,
			      ESP8266_SERIAL_DEF_STOP);
}

/**
 * Waits for the "ready" string that comes from the wifi module when
 * it is ready to receive commands.  This is useful to wait for after
 * a reset so we know the module is ready to go.
 */
bool esp8266_wait_for_ready(struct Serial* serial)
{
	return at_basic_wait_for_msg(serial, "ready",
				     ESP8266_INIT_TIMEOUT_MS);
}
