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
#include "constants.h"
#include "dateTime.h"
#include "esp8266.h"
#include "esp8266_drv.h"
#include "macros.h"
#include "loggerConfig.h"
#include "panic.h"
#include "printk.h"
#include "queue.h"
#include "semphr.h"
#include "str_util.h"
#include "task.h"
#include "taskUtil.h"
#include "wifi.h"
#include "wifi_device.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define AT_TASK_TIMEOUT_MS	3
#define CHECK_DONE_SLEEP_MS	3600000
#define CLIENT_BACKOFF_MS	30000
#define AP_BACKOFF_MS		30000
#define INIT_FAIL_SLEEP_MS	10000
#define LOG_PFX			"[ESP8266 Driver] "
#define MAX_CHANNELS		5
#define SERIAL_BAUD		115200
#define SERIAL_BITS		8
#define SERIAL_CMD_MAX_LEN	1024
#define SERIAL_PARITY		0
#define SERIAL_RX_BUFF_SIZE	256
#define SERIAL_STOP_BITS	1
#define SERIAL_TX_BUFF_SIZE	256
#define TASK_STACK_SIZE		256
#define TASK_THREAD_NAME	"ESP8266 Driver"

struct device {
        struct Serial *serial;
        enum dev_init_state init_state;
        enum esp8266_op_mode op_mode;
};

struct client {
        const struct wifi_client_cfg *config;
        struct esp8266_client_info info;
        struct esp8266_ipv4_info ipv4;
        tiny_millis_t info_timestamp;
        tiny_millis_t next_join_attempt;
};

struct ap {
        const struct wifi_ap_cfg *config;
        struct esp8266_ap_info info;
        struct esp8266_ipv4_info ipv4;
        tiny_millis_t info_timestamp;
        tiny_millis_t next_set_attempt;
};

struct server {
        bool listening;
};

struct channel {
        struct Serial *serial;
        size_t tx_chars_buffered;
        bool in_use;
        bool connected;
        bool created_externally;
};

struct comm {
        new_conn_func_t *new_conn_cb;
        struct channel channels[MAX_CHANNELS];
        xSemaphoreHandle connect_semaphore;
        xSemaphoreHandle connect_cb_semaphore;
        bool connect_status;
};

/**
 * These enums define the various checks that we can perform.
 * Checks can lead to action.  Actions lead to updated state,
 * which can then trigger checks.  We go round and round with
 * this methodology to ensure that all decision making is kept
 * in a single location.  Checks stop when nothing needs to
 * be changed.
 *
 * Note that each check represents a tiny state machine.  These
 * state machines have interdependeinces upon one another. In
 * example the client wifi (CHECK_WIFI_CLIENT) requires that the wifi
 * subsystem (CHECK_WIFI_DEVICE) be initialized. Thus we need a priority
 * with the lowest value being the highest priority.
 */
enum check {
        CHECK_WIFI_DEVICE,
        CHECK_WIFI_CLIENT,
        CHECK_WIFI_AP,
        CHECK_SERVER,
        CHECK_DATA,
        __NUM_CHECKS, /* Always the last */
};

struct cmd {
        bool in_progress;
        tiny_millis_t check[__NUM_CHECKS];
};

static struct {
        struct device device;
        struct client client;
        struct ap ap;
        struct server server;
        struct comm comm;
        struct cmd cmd;
} esp8266_state;

static void cmd_started()
{
        esp8266_state.cmd.in_progress = true;
}

static void cmd_completed()
{
        esp8266_state.cmd.in_progress = false;
}

static void cmd_sleep_until(enum check check,
                            const tiny_millis_t time)
{
        esp8266_state.cmd.check[check] = time;
}

static void cmd_sleep(enum check check,
                      const tiny_millis_t sleep_ms)
{
        cmd_sleep_until(check,
                        date_time_uptime_now_plus(sleep_ms));
}

static void cmd_set_check(enum check check)
{
        esp8266_state.cmd.check[check] = 0;
}

/**
 * Returns the next check we need to perform in order of priority.
 * The lower the value, the higher the priority.
 */
static enum check cmd_get_next_check()
{
        for(size_t i = 0; i < __NUM_CHECKS; ++i)
                if (date_time_is_past(esp8266_state.cmd.check[i]))
                        return i;

        return -1;
}

static void cmd_check_complete(enum check check)
{
        esp8266_state.cmd.check[check] =
                date_time_uptime_now_plus(CHECK_DONE_SLEEP_MS);
}

/**
 * Checks if we are ready to execute a command.
 */
static bool cmd_ready() {
        return !esp8266_state.cmd.in_progress;
}

/* Begin code for sending/receiving data */

static bool is_valid_socket_channel_id(const size_t chan_id)
{
        return chan_id < ARRAY_LEN(esp8266_state.comm.channels);
}

static const char* get_channel_name(const size_t chan_id)
{
        static const char* _serial_names[] = {
                "Wifi Chan 0",
                "Wifi Chan 1",
                "Wifi Chan 2",
                "Wifi Chan 3",
                "Wifi Chan 4",
        };

        if (chan_id >= ARRAY_LEN(_serial_names))
                return NULL;

        return _serial_names[chan_id];
}

static void _tx_char_cb(xQueueHandle queue, void *post_tx_arg)
{
        struct channel *ch = post_tx_arg;
        ++ch->tx_chars_buffered;
        cmd_set_check(CHECK_DATA);
}

static struct Serial* setup_channel_serial(const unsigned int i)
{
        const char* name = get_channel_name(i);
        struct channel* ch =esp8266_state.comm.channels + i;
        struct Serial *s = serial_create(name, SERIAL_TX_BUFF_SIZE,
                                         SERIAL_RX_BUFF_SIZE, NULL, NULL,
                                         _tx_char_cb, ch);
        if (s)
                return s;

        /* Fail state */
        pr_warning(LOG_PFX "Failed to create serial port\r\n");
        return NULL;
}

static struct channel* get_channel_for_use(const unsigned int i)
{
        if (!is_valid_socket_channel_id(i))
                return NULL;

        struct channel* ch = esp8266_state.comm.channels + i;
        if (ch->serial)
                return ch;

        /* If here, channel has no serial.  Try to set one up */
        ch->serial = setup_channel_serial(i);
        return ch->serial ? ch : NULL;
}

static int get_next_free_channel_num()
{
        for (size_t i = 0; is_valid_socket_channel_id(i); ++i) {
                struct channel *ch = esp8266_state.comm.channels + i;
                if (!ch->in_use)
                        return i;
        }

        return -1;
}

/**
 * Callback that gets invoked by the device code whenever new data
 * comes in.
 */
static void rx_data_cb(int chan_id, size_t len, const char* data)
{
        if (!is_valid_socket_channel_id(chan_id)) {
                pr_error_int_msg(LOG_PFX "Channel id to big: ", chan_id);
                return;
        }

        struct channel *ch = get_channel_for_use(chan_id);
        if (NULL == ch) {
                pr_error(LOG_PFX "No channel available.  Dropping\r\n");
                return;
        }

        /*
         * Since these connections are created by an external source,
         * we set the created_externally flag here.  This way
         * we know to reap the channel when the connection disappears.
         */
        ch->created_externally = true;
        ch->in_use = true;
        ch->connected = true;

        /*
         * Check that we actually have a call back set to handle the
         * incoming data.  If not then the socket stays open until it is
         * closed by our WiFi host.
         */
        if (!esp8266_state.comm.new_conn_cb) {
                pr_error(LOG_PFX "No Serial callback defined\r\n");
                return;
        }

        esp8266_state.comm.new_conn_cb(ch->serial);

        /*
         * Now that the upper layer has been informed about the incoming
         * data, start sending it said data.  It will unblock and read this
         * data very shortly.
         */
        xQueueHandle q = serial_get_rx_queue(ch->serial);
        for (size_t i = 0; i < len; ++i)
                xQueueSend(q, data + i, portMAX_DELAY);

        cmd_set_check(CHECK_DATA);
}

/**
 * Callback that is invoked when the send_data method completes.
 */
static void _send_data_cb(int bytes_sent)
{
        cmd_completed();
        cmd_set_check(CHECK_DATA);

        if (bytes_sent < 0)
                /* STIEG: Include channel info here somehow */
                pr_warning(LOG_PFX "Failed to send data\r\n");
}

/**
 * Method that processes outgoing data if any.  If there is, this
 * will start a command.
 */
static void check_data()
{
        cmd_check_complete(CHECK_DATA);

        for (size_t i = 0; i < ARRAY_LEN(esp8266_state.comm.channels); ++i) {
                struct channel *ch = esp8266_state.comm.channels + i;
                const size_t size = ch->tx_chars_buffered;

                /* If the size is 0, nothing to send */
                if (0 == size)
                        continue;

                /*
                 * If here, then we have data to send.  Check that the
                 * connection is still active.  If not, dump the data
                 */
                if (!ch->connected) {
                        serial_purge_tx_queue(ch->serial);
                        ch->tx_chars_buffered = 0;
                        continue;
                }

                /* If here, we have a connection and data to send. Do Eet! */
                const bool cmd_queued =
                        esp8266_send_data(i, ch->serial, size, _send_data_cb);

                if (!cmd_queued) {
                        pr_warning(LOG_PFX "Failed to queue send "
                                   "data command!!!\r\n");
                        /* We will retry */
                        return;
                }

                cmd_started();
                ch->tx_chars_buffered = 0;
                return;
        }
}

/**
 * Callback that gets invoked when our client wifi state changes.
 */
static void client_state_changed_cb(const char* msg)
{
        /* Need to check our client now that a change has occured */
        cmd_set_check(CHECK_WIFI_CLIENT);

        memset(&esp8266_state.client.info, 0,
               sizeof(esp8266_state.client.info));
        memset(&esp8266_state.client.ipv4, 0,
               sizeof(esp8266_state.client.ipv4));

        pr_info_str_msg(LOG_PFX "Client state changed: ", msg);
}

/**
 * Callback that gets invoked when a socket state changes.
 */
static void socket_state_changed_cb(const size_t chan_id,
                                    const enum socket_action action)
{
        if (!is_valid_socket_channel_id(chan_id)) {
                pr_warning_int_msg(LOG_PFX "Invalid socket id: ",
                                   chan_id);
                return;
        }

        struct channel *ch = esp8266_state.comm.channels + chan_id;
        switch (action) {
        case SOCKET_ACTION_DISCONNECT:
                pr_info_int_msg(LOG_PFX "Socket closed on channel ",
                                chan_id);
                /*
                 * If channel created externally, then when closed it
                 * means it is no longer in use.
                 */
                if (ch->created_externally)
                        ch->in_use = false;

                ch->created_externally = false;
                ch->connected = false;

                /*
                 * Clear the serial data out since channel is now closed
                 * we don't want any cruft in there the next time a channel
                 * gets opened.  Also be sure the serial exists, as it could
                 * fail to get created.
                 */
                if (ch->serial)
                        serial_clear(ch->serial);

                break;
        case SOCKET_ACTION_CONNECT:
                pr_info_int_msg(LOG_PFX "Socket connected on channel ",
                                chan_id);
                ch->connected = true;
                break;
        default:
                pr_warning(LOG_PFX "Unknown socket action\r\n");
                break;
        }

        cmd_set_check(CHECK_DATA);
}

/* *** Methods that handle device init and reset *** */

/**
 * Callback that is invoked when the init_wifi command completes.
 */
static void init_wifi_cb(enum dev_init_state dev_state)
{
        cmd_completed();
        cmd_set_check(CHECK_WIFI_DEVICE);

        pr_info_int_msg(LOG_PFX "Device state: ", dev_state);
        esp8266_state.device.init_state = dev_state;

        /* Now that init state has changed, check them */
        cmd_set_check(CHECK_WIFI_DEVICE);
        cmd_set_check(CHECK_WIFI_CLIENT);
        cmd_set_check(CHECK_WIFI_AP);
        cmd_set_check(CHECK_SERVER);
        cmd_set_check(CHECK_DATA);
}

/**
 * Use this to initialize the WiFi device.  Device should be in a reset state.
 * If not, this may fail.
 */
static void init_wifi()
{
        pr_info(LOG_PFX "Initializing Wifi\r\n");

        serial_clear(esp8266_state.device.serial);
        if (!esp8266_init(esp8266_state.device.serial, SERIAL_CMD_MAX_LEN,
                          init_wifi_cb)) {
                /* Failed to init critical bits.  */
                pr_warning(LOG_PFX "Failed to init esp8266 device.\r\n");
                cmd_sleep(CHECK_WIFI_DEVICE, INIT_FAIL_SLEEP_MS);
                return;
        }

        cmd_started();

        /*
         * Now register the callback for incoming data.  Safe to do
         * before init has completed b/c this invokes no modem calls.
         * This should only fail if there was an issue with space or config.
         */
        const struct esp8266_event_hooks hooks = {
                .client_state_changed_cb = client_state_changed_cb,
                .socket_state_changed_cb = socket_state_changed_cb,
                .data_received_cb = rx_data_cb,
        };

        if (!esp8266_register_callbacks(&hooks))
                pr_warning(LOG_PFX "Failed to register callbacks\r\n");
}

/**
 * Callback used by the get_op_mode call.
 */
static void get_op_mode_cb(bool status, enum esp8266_op_mode mode)
{
        cmd_completed();
        cmd_set_check(CHECK_WIFI_DEVICE);

        if (!status) {
                pr_warning(LOG_PFX "Get OP mode failed!\r\n");
                esp8266_state.device.op_mode = ESP8266_OP_MODE_UNKNOWN;
        } else {
                esp8266_state.device.op_mode = mode;
        }
}

/**
 * Command that gets the wifi device operational mode.  This can be
 * client, soft_ap, or both.
 */
static void get_op_mode()
{
        if (!esp8266_get_op_mode(get_op_mode_cb)) {
                pr_warning(LOG_PFX "Failed to get OP mode\r\n");
                return;
        }

        cmd_started();
}

/**
 * Callback used by the set_op_mode call.
 */
static void set_op_mode_cb(const bool status)
{
        cmd_completed();
        cmd_set_check(CHECK_WIFI_DEVICE);

        /*
         * Clear out our state and read from device again to ensure that
         * the value actually took
         */
        esp8266_state.device.op_mode = ESP8266_OP_MODE_UNKNOWN;

        if (!status)
                pr_warning(LOG_PFX "Set OP mode failed!\r\n");
}

/**
 * Command that sets the wifi device operational mode. Useful for changing
 * between modes after actions like a user changing the config.
 */
static void set_op_mode(const enum esp8266_op_mode mode)
{
        if (!esp8266_set_op_mode(mode, set_op_mode_cb)) {
                pr_warning(LOG_PFX "Failed to invoke set OP mode\r\n");
                return;
        }

        cmd_started();
}

/**
 * Method that checks the device state.  Handles all aspects including
 * initialization, reset, mode, and whatever else may need handling
 * down the road.
 */
static void check_wifi_device()
{
        cmd_check_complete(CHECK_WIFI_DEVICE);
        pr_info(LOG_PFX "Checking WiFi Device...\r\n");

        switch(esp8266_state.device.init_state) {
        case DEV_INIT_STATE_NOT_READY:
                init_wifi();
                return;
        case DEV_INIT_STATE_READY:
                /* Then we are where we want to be */
                break;
        default:
                /* STIEG: TODO Handle reset here when implemented. */
                delayMs(CLIENT_BACKOFF_MS);
        }

        /* Check if we know our device mode.  If not, get it */
        if (ESP8266_OP_MODE_UNKNOWN == esp8266_state.device.op_mode) {
                /* Get OP mode */
                get_op_mode();
                return;
        }

        /* Now check device mode to ensure its correct */
        enum esp8266_op_mode exp_mode;
        const bool ap_active = esp8266_state.ap.config->active;
        const bool client_active = esp8266_state.client.config->active;
        if (ap_active) {
                /* AP should be active.  Is the client? */
                exp_mode = client_active ?
                        ESP8266_OP_MODE_BOTH : ESP8266_OP_MODE_AP;
        } else {
                /*
                 * AP should be inactive.  We just set client here since
                 * we can't disable both client and AP.  We effectively do
                 * this by simply not having the client associate with any
                 * AP.
                 */
                exp_mode = ESP8266_OP_MODE_CLIENT;
        }

        if (exp_mode != esp8266_state.device.op_mode) {
                /* Then set the correct mode */
                set_op_mode(exp_mode);
                return;
        }

        /* If here, WiFi Device is in the correct state */
        return;
}

static bool device_initialized()
{
        return esp8266_state.device.init_state == DEV_INIT_STATE_READY;
}


/* *** Methods that handle the client WiFi state and actions *** */

/**
 * Callback that gets invoked with the get_client_ap command completes.
 */
static void get_client_ap_cb(bool status, const struct esp8266_client_info *ci)
{
        cmd_completed();
        cmd_set_check(CHECK_WIFI_CLIENT);
        esp8266_state.client.info_timestamp = getUptime();

        memcpy(&esp8266_state.client.info, ci, sizeof(struct esp8266_client_info));
}

/**
 * Command that will populate our wifi client info with useful data.
 */
static void get_client_ap()
{
        pr_info(LOG_PFX "Retrieving Wifi Client Info\r\n");
        esp8266_get_client_ap(get_client_ap_cb);
        cmd_started();
}

/**
 * Callback that gets invoked when the get_ip_info call completes.
 */
static void get_ip_info_cb(const bool status,
                           const struct esp8266_ipv4_info* client,
                           const struct esp8266_ipv4_info* station)
{
        cmd_completed();
        cmd_set_check(CHECK_WIFI_CLIENT);
        cmd_set_check(CHECK_WIFI_AP);

        if (!status) {
                pr_info(LOG_PFX "Failed to obtain IP info\r\n");
        }

        /*
         * Update IP information for both station and client.
         * This is sane in the failure case since the driver will
         * return zeroed out objects.
         */
        const size_t size = sizeof(struct esp8266_ipv4_info);
        memcpy(&esp8266_state.client.ipv4, client, size);
        memcpy(&esp8266_state.ap.ipv4, station, size);
}

/**
 * Command that will get the ip information about our system and
 * update both our AP and Client structs with data.
 */
static void get_ip_info()
{
        pr_info(LOG_PFX "Retrieving Wifi IP info\r\n");
        esp8266_get_ip_info(get_ip_info_cb);
        cmd_started();
}

/**
 * Callback that gets invoked upon completion of set_client_ap command.
 */
static void set_client_ap_cb(bool status)
{
        cmd_completed();
        cmd_set_check(CHECK_WIFI_CLIENT);

        if (!status) {
                /* Failed. */
                pr_info(LOG_PFX "Failed to join network\r\n");
        } else {
                /* If here, we were successful. */
                pr_info(LOG_PFX "Successfully joined network\r\n");
        }

        /* Clear the client info since it has changed. */
        esp8266_state.client.info_timestamp = 0;
        memset(&esp8266_state.client.info, 0,
               sizeof(esp8266_state.client.info));
}

/**
 * Command that will set the wifi client settings as specified in
 * our settings.
 */
static void set_client_ap()
{
        pr_info(LOG_PFX "Setting Wifi Client Info\r\n");
        const struct wifi_client_cfg *cc = esp8266_state.client.config;
        pr_info_str_msg(LOG_PFX "Joining network: ", cc->ssid);
        esp8266_join_ap(cc->ssid, cc->passwd, set_client_ap_cb);
        cmd_started();
}

/**
 * Helper method to check_wifi_client.  This method tries to join an AP, but
 * also handles cases where its not reachable or we have other issues
 * (like an incorrect password).
 */
static void client_try_join_ap()
{
        if (!date_time_is_past(esp8266_state.client.next_join_attempt)){
                /* Then we need to backoff */
                const tiny_millis_t sleep_len =
                        esp8266_state.client.next_join_attempt - getUptime();
                cmd_sleep(CHECK_WIFI_CLIENT, sleep_len);
        } else {
                /* If here, then its time to do work */
                esp8266_state.client.next_join_attempt =
                        date_time_uptime_now_plus(CLIENT_BACKOFF_MS);
                set_client_ap();
        }
}

/**
 * This is the block of logic that manages our WiFi client state.
 * Its responsible for keeping the client status as close to the
 * configuration as is reasonably possible.
 */
static void check_wifi_client()
{
        cmd_check_complete(CHECK_WIFI_CLIENT);

        pr_info(LOG_PFX "Checking Client\r\n");

        /* First check that we are initialized */
        if (!device_initialized()) {
                cmd_sleep(CHECK_WIFI_CLIENT, CLIENT_BACKOFF_MS);
                return;
        }

        /*
         * First check if we have reasonably fresh client info. Have to
         * have it before we can make any decisions.
         */
        if (0 == esp8266_state.client.info_timestamp) {
                get_client_ap();
                return;
        }

        /* If here, we have fresh client info.  Use it to make decisions. */
        const struct wifi_client_cfg *cfg = esp8266_state.client.config;
        const struct esp8266_client_info *ci = &esp8266_state.client.info;
        if (cfg->active) {
                /*
                 * Config says client should be active. Make it so.
                 * First check that we have an AP and that we are on
                 * the correct client network.  If not, try to get on
                 * the correct network.
                 */
                if (!ci->has_ap || !STR_EQ(ci->ssid, cfg->ssid)) {
                        /* Then we need to try and join the AP */
                        client_try_join_ap();
                        return;
                }

                /* If here, then on correct AP.  Do we have IP info? */
                if (!*esp8266_state.client.ipv4.address) {
                        /* Then we need to get IP Information */
                        get_ip_info();
                        return;
                }

                /* If here, we are done */
                pr_info(LOG_PFX "Client configured correctly\r\n");
                pr_info_str_msg(LOG_PFX "Client IP: ",
                        esp8266_state.client.ipv4.address);
                return;
        } else {
                /* Client should be inactive.  Disable as needed */
                if (ci->has_ap) {
                        /*
                         * If here, client is active.  Leave AP.
                         * STIEG: TODO: Implement Leave AP call. We are done
                         * for now since code not implemented.
                         */
                        return;
                }

                /* Then config and reality align.  Done */
                pr_info(LOG_PFX "Client is inactive.\r\n");
                return;
        }
}


/* *** Methods that handle the Wifi AP and its actions *** */

/**
 * Callback that gets invoked when the get_ap_info command completes.
 */
static void get_ap_info_cb(const bool status,
                           const struct esp8266_ap_info* info)
{
        cmd_completed();
        cmd_set_check(CHECK_WIFI_AP);
        esp8266_state.ap.info_timestamp = getUptime();

        const size_t ap_info_size = sizeof(esp8266_state.ap.info);
        if (!status) {
                pr_warning(LOG_PFX "Failed to read AP info\r\n");
                memset(&esp8266_state.ap.info, 0, ap_info_size);
                return;
        }

        memcpy(&esp8266_state.ap.info, info, ap_info_size);

        /* Print out our AP Info here */
        pr_info(LOG_PFX "AP info:\r\n");
        pr_info_str_msg("\t SSID      : ", info->ssid);
        pr_info_int_msg("\t Channel   : ", info->channel);
        pr_info_int_msg("\t Encryption: ", info->encryption);
}

/**
 * Command that will retrieve the Soft AP configuration settings
 * from the WiFi device.
 */
static bool get_ap_info()
{
        pr_info(LOG_PFX "Retrieving Wifi AP Info\r\n");

        const bool queued = esp8266_get_ap_info(get_ap_info_cb);
        if (queued)
                cmd_started();

        return queued;
}

/**
 * Callback that is invoked when the set_ap command completes.
 */
static void set_ap_cb(const bool status)
{
        cmd_completed();
        cmd_set_check(CHECK_WIFI_AP);

        /* Update our backoff timer to prevent busy loops */
        esp8266_state.ap.next_set_attempt =
                date_time_uptime_now_plus(AP_BACKOFF_MS);

        /* Clear out the info since it has changed */
        const size_t ap_info_size = sizeof(esp8266_state.ap.info);
        memset(&esp8266_state.ap.info, 0, ap_info_size);
        esp8266_state.ap.info_timestamp = 0;

        if (!status)
                pr_warning(LOG_PFX "Failed to read AP info\r\n");
}

/**
 * Command that will set the configuration of the soft AP of the
 * esp8266 device.
 */
static bool set_ap(struct esp8266_ap_info* ap_info)
{
        pr_info(LOG_PFX "Updating Wifi AP Config.\r\n");
        const bool queued = esp8266_set_ap_info(ap_info, set_ap_cb);
        if (queued)
                cmd_started();

        return queued;
}

/**
 * Helper method to check_wifi_ap.  This method will attempt to adjust the
 * wifi ap settings if it deems that enough time has passed.  Otherwise it
 * will cause a back-off to occur.  This is necessary so that a bad config
 * doesn't cause a busy wait in the system.
 */
static void ap_check_try_set()
{
        if (!date_time_is_past(esp8266_state.ap.next_set_attempt)){
                /* Then we need to backoff */
                const tiny_millis_t sleep_len =
                        esp8266_state.ap.next_set_attempt - getUptime();
                cmd_sleep(CHECK_WIFI_AP, sleep_len);
                return;
        }

        /* Setup our configuration structure to pass in */
        const struct wifi_ap_cfg* cfg = esp8266_state.ap.config;
        struct esp8266_ap_info ap_info;
        strncpy(ap_info.ssid, cfg->ssid, ARRAY_LEN(ap_info.ssid));
        strncpy(ap_info.password, cfg->password, ARRAY_LEN(ap_info.password));
        ap_info.channel = cfg->channel;
        ap_info.encryption = cfg->encryption;

        if (!set_ap(&ap_info))
                pr_warning(LOG_PFX "Failed to issue set_ap command\r\n");
}

/**
 * This is the block of logic that manages our WiFi client state.
 * Its responsible for keeping the client status as close to the
 * configuration as is reasonably possible.
 */
static void check_wifi_ap()
{
        cmd_check_complete(CHECK_WIFI_AP);
        pr_info(LOG_PFX "Checking WiFi AP\r\n");

        /* First check that we are initialized */
        if (!device_initialized()) {
                cmd_sleep(CHECK_WIFI_AP, AP_BACKOFF_MS);
                return;
        }

        /*
         * First check if we have reasonably fresh AP info. Have to
         * have it before we can make any decisions.
         */
        if (0 == esp8266_state.ap.info_timestamp) {
                get_ap_info();
                return;
        }

        /* If here, we have fresh AP info.  Use it to make decisions. */
        const struct wifi_ap_cfg* cfg = esp8266_state.ap.config;
        if (!cfg->active) {
                /*
                 * AP should be inactive. This is controlled by the device
                 * state machine because managing the esp8266
                 * device mode effects both the wifi client and the soft
                 * AP.  Hence this is a NO OP here.
                 */
                pr_info(LOG_PFX "AP is inactive.\r\n");
                return;
        }

        /* Config says AP should be active. Make it so. */
        const struct esp8266_ap_info* info = &esp8266_state.ap.info;
        if (!STR_EQ(cfg->ssid, info->ssid) ||
            !STR_EQ(cfg->password, info->password) ||
            cfg->channel != info->channel ||
            cfg->encryption != info->encryption) {
                /* Then we need to re-setup our AP */
                ap_check_try_set();
                return;
        }

        /* If here, then our AP setup is correct.  Do we have IP info? */
        if (!*esp8266_state.ap.ipv4.address) {
                /* Then we need to get IP Information */
                get_ip_info();
                return;
        }

        /* If here, we are setup properly and are done */
        pr_info(LOG_PFX "AP configured properly\r\n");
        pr_info_str_msg(LOG_PFX "Station IP: ",
                        esp8266_state.ap.ipv4.address);
        return;
}


/* *** Methods that handle the Server and its actions *** */

static void server_cmd_cb(bool status)
{
        cmd_completed();
        esp8266_state.server.listening = status;
        if (!status)
                pr_warning(LOG_PFX "Failed to setup server\r\n");
}

static void setup_server()
{
        pr_info_int_msg(LOG_PFX "Starting server on port: ",
                        RCP_SERVICE_PORT);
        esp8266_server_cmd(ESP8266_SERVER_ACTION_CREATE, RCP_SERVICE_PORT,
                           server_cmd_cb);
        cmd_started();
}

/**
 * This logic is responsible for checking the server and ensuring that it
 * is in its correct state.  If its not, then this code will get it there.
 */
static void check_server()
{
        cmd_check_complete(CHECK_SERVER);

        pr_info(LOG_PFX "Checking Server\r\n");

        /* First check that we are initialized */
        if (!device_initialized()) {
                cmd_sleep(CHECK_SERVER, CLIENT_BACKOFF_MS);
                return;
        }

        if (!esp8266_state.server.listening) {
                /* Then we need to activate it */
                setup_server();
                return;
        }

        /* Then its listening.  We are done */
        return;
}


/* *** The all important task loop *** */

static void task_loop()
{
        esp8266_do_loop(AT_TASK_TIMEOUT_MS);

        /* If there is a command in progress, no commands */
        if (!cmd_ready())
                return;

        switch(cmd_get_next_check()) {
        case CHECK_WIFI_DEVICE:
                check_wifi_device();
                break;
        case CHECK_WIFI_CLIENT:
                check_wifi_client();
                break;
        case CHECK_WIFI_AP:
                check_wifi_ap();
                break;
        case CHECK_SERVER:
                check_server();
                break;
        case CHECK_DATA:
                check_data();
                break;
        default:
                /* Nothing to do. */
                break;
        }
}

static void task(void *params)
{
        for(;;)
                task_loop();

        panic(PANIC_CAUSE_UNREACHABLE);
}

bool esp8266_drv_update_client_cfg(const struct wifi_client_cfg *cc)
{
        if (NULL == cc)
                return false;

        esp8266_state.client.config = cc;

        /* Set flags for what states need checking */
        cmd_set_check(CHECK_WIFI_DEVICE);
        cmd_set_check(CHECK_WIFI_CLIENT);

        return true;
}

bool esp8266_drv_update_ap_cfg(const struct wifi_ap_cfg *wac)
{
        if (NULL == wac)
                return false;

        esp8266_state.ap.config = wac;

        /* Zero this value out so we will forego any backoff attempts */
        esp8266_state.ap.next_set_attempt = 0;

        /* Set flags for what states need checking */
        cmd_set_check(CHECK_WIFI_DEVICE);
        cmd_set_check(CHECK_WIFI_AP);

        return true;
}

bool esp8266_drv_init(struct Serial *s, const int priority,
                      new_conn_func_t new_conn_cb)
{
        /* Initialize the esp8266 hardware */
        if (!wifi_device_init()) {
            pr_error(LOG_PFX "Failed to init WiFi device\r\n");
            return false;
        }

        if (esp8266_state.device.serial)
                return false; /* Already setup */

        esp8266_state.device.serial = s;

        if (!esp8266_state.device.serial) {
                pr_error(LOG_PFX "NULL serial\r\n");
                return false;
        }

        serial_config(esp8266_state.device.serial, SERIAL_BITS, SERIAL_PARITY,
                      SERIAL_STOP_BITS, SERIAL_BAUD);

        /* Create and setup semaphores for connections */
        esp8266_state.comm.connect_semaphore = xSemaphoreCreateBinary();
        xSemaphoreGive(esp8266_state.comm.connect_semaphore);
        esp8266_state.comm.connect_cb_semaphore = xSemaphoreCreateBinary();


        /* Initialize our WiFi configs here */
        LoggerConfig *lc = getWorkingLoggerConfig();
        const struct wifi_client_cfg *cfg =
                &lc->ConnectivityConfigs.wifi.client;
        if (!esp8266_drv_update_client_cfg(cfg)) {
                pr_error(LOG_PFX "Failed to set WiFi client cfg\r\n");
                return false;
        }

        const struct wifi_ap_cfg* wac =
                &lc->ConnectivityConfigs.wifi.ap;
        esp8266_drv_update_ap_cfg(wac);

        esp8266_state.comm.new_conn_cb = new_conn_cb;

        /* Set the task loop to check the wifi_device first */
        cmd_set_check(CHECK_WIFI_DEVICE);

        static const signed char task_name[] = TASK_THREAD_NAME;
        const size_t stack_size = TASK_STACK_SIZE;
        xTaskCreate(task, task_name, stack_size, NULL, priority, NULL);

        return true;
}

/**
 * Callback method invoked when a callback completes.
 */
static void connect_cb(const bool status)
{
        struct comm *comm = &esp8266_state.comm;
        comm->connect_status = status;

        cmd_set_check(CHECK_DATA);

        xSemaphoreGive(comm->connect_cb_semaphore);
}

struct Serial* esp8266_drv_connect(const enum protocol proto,
                                   const char* dst_ip,
                                   const unsigned int dst_port)
{
        /* This is synchronous code for now */
        struct comm *comm = &esp8266_state.comm;
        xSemaphoreTake(comm->connect_semaphore, portMAX_DELAY);

        struct Serial* serial = NULL;
        const int chan_id = get_next_free_channel_num();
        if (chan_id < 0) {
                pr_warning(LOG_PFX "Failed to acquire a free channel\r\n");
                goto done;
        }

        struct channel *ch = get_channel_for_use(chan_id);
        if (NULL == ch) {
                pr_warning(LOG_PFX "Can't allocate resources for channel\r\n");
                goto done;
        }

        if (!esp8266_connect(chan_id, proto, dst_ip, dst_port, connect_cb)) {
                pr_warning(LOG_PFX "Failed to issue connect command\r\n");
                goto done;
        }

        /* Now wait for our callback to come */
        if (!xSemaphoreTake(comm->connect_cb_semaphore, portMAX_DELAY)) {
                pr_warning(LOG_PFX "Timed out waiting for connect "
                           "response\r\n");
                goto done;
        }

        ch->connected = comm->connect_status;
        if (!ch->connected) {
                pr_info_str_msg(LOG_PFX "Failed to connect: ", dst_ip);

                goto done;
        }

        ch->in_use = true;
        serial = ch->serial;
        pr_info_int_msg(LOG_PFX "Connected on comm channel ", chan_id);

done:
        xSemaphoreGive(comm->connect_semaphore);
        return serial;
}

const struct esp8266_ipv4_info* get_client_ipv4_info()
{
        return &esp8266_state.client.ipv4;
}

const struct esp8266_ipv4_info* get_ap_ipv4_info()
{
        return &esp8266_state.ap.ipv4;
}
