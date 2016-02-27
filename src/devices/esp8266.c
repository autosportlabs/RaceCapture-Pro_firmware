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
#include "printk.h"

#include <stdbool.h>

#define ESP8266_AUTOBAUD_TRIES	20
#define ESP8266_CMD_DELIM	"\r\n"
#define ESP8266_QP_PRE_INIT_MS	500
#define ESP8266_QP_STANDARD_MS	1	/* Can probably be 0 */
#define TIMEOUT_LONG_MS		100
#define TIMEOUT_SHORT_MS	10

static const enum log_level serial_dbg_lvl = INFO;

/**
 * Internal state of our driver.
 */
static struct {
        enum dev_init_state init_state;
        struct at_info *ati;
} state;

/**
 * Call the if the init routine fails.
 * @param msg The message to print.
 */
static void init_failed(const char *msg)
{
        state.init_state = DEV_INIT_STATE_FAILED;

        if (msg)
                pr_info_str_msg("[esp8266] Init failure: ", msg);
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
        at_put_cmd(state.ati, "AT+GMR", TIMEOUT_LONG_MS,
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

        at_put_cmd(state.ati, "AT", TIMEOUT_LONG_MS, autobaud_cb, tries);
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
