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
#include "at_basic.h"
#include "capabilities.h"
#include "constants.h"
#include "dateTime.h"
#include "esp8266.h"
#include "esp8266_drv.h"
#include "net/ipv4.h"
#include "macros.h"
#include "led.h"
#include "loggerConfig.h"
#include "panic.h"
#include "printk.h"
#include "queue.h"
#include "semphr.h"
#include "str_util.h"
#include "task.h"
#include "taskUtil.h"
#include "timers.h"
#include "wifi.h"
#include "wifi_device.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#define AP_BACKOFF_MS		30000
#define AT_TASK_TIMEOUT_MS	3
/* MS to wait before checking/updating wifi configs */
#define CFG_UPDATE_SLEEP_MS	1000
#define CHECK_DONE_SLEEP_MS	3600000
#define CLIENT_BACKOFF_MS	3000
#define CMD_MAX_FAILURES	3
#define INIT_FAIL_SLEEP_MS	10000
#define INVALID_CHANNEL_ID	-1
#define LED_PERIOD_MS		25
#define LOG_PFX			"[ESP8266 Driver] "
#define MAX_CHANNELS		5
#define RX_DATA_TIMEOUT_TICKS	1
#define SERIAL_BUFF_DEF_RX_SIZE	RX_MAX_MSG_LEN
#define SERIAL_BUFF_DEF_TX_SIZE	512
#define SERIAL_CMD_MAX_LEN	(RX_MAX_MSG_LEN + 16)
#define TASK_STACK_SIZE		320
#define TASK_THREAD_NAME	"ESP8266 Driver"

enum init_state {
        INIT_STATE_UNINITIALIZED = 0,
        INIT_STATE_READY,
        INIT_STATE_FAILED,
        INIT_STATE_RESET,
        INIT_STATE_RESET_HARD,
};

struct device {
        struct Serial *serial;
        enum init_state init_state;
        enum esp8266_op_mode op_mode;
};

struct client {
        struct wifi_client_cfg config;
        struct esp8266_client_info info;
        struct esp8266_ipv4_info ipv4;
        tiny_millis_t info_timestamp;
        tiny_millis_t next_join_attempt;
	bool configured;
};

struct ap {
        struct wifi_ap_cfg config;
        struct esp8266_ap_info info;
        struct esp8266_ipv4_info ipv4;
        tiny_millis_t info_timestamp;
        tiny_millis_t next_set_attempt;
};

struct server {
        bool listening;
};

struct channel {
	bool in_use;
        struct Serial *serial;
};

struct channel_sync_op {
        xSemaphoreHandle op_semaphore;
        xSemaphoreHandle cb_semaphore;
        int chan_id;
};

struct comm {
        new_conn_func_t *new_conn_cb;
        struct channel channels[MAX_CHANNELS];
        struct channel_sync_op connect_op;
        struct channel_sync_op close_op;
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
	unsigned int failures;
};

static struct {
        struct device device;
        struct client client;
        struct ap ap;
        struct server server;
        struct comm comm;
        struct cmd cmd;
        struct {
                tiny_millis_t backoff;
                xTimerHandle timer;
        } led;
} esp8266_state;

static void led_timer_cb( xTimerHandle xTimer )
{
        led_set(LED_WIFI, false);
}

static void trigger_led()
{
        const tiny_millis_t now = getUptime();

        /* Backoff until backoff is expired */
        if (esp8266_state.led.timer && now < esp8266_state.led.backoff)
                return;

        /* If here, set the light, and set the timer to turn it off */
        esp8266_state.led.backoff = now + msToTicks(LED_PERIOD_MS) * 2;
        xTimerStart(esp8266_state.led.timer, 0);
        led_set(LED_WIFI, true);
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

static void cmd_started()
{
        esp8266_state.cmd.in_progress = true;
}

static void cmd_completed(const bool success)
{
        trigger_led();
        esp8266_state.cmd.in_progress = false;

	/*
	 * If command failed, then we increment our failure count.
	 * If command succeeded, decrement our failure count unless
	 * it is 0.  If we hit the threshold of failures, trigger
	 * a reset.
	 */
	if (!success)
		++esp8266_state.cmd.failures;
	else if (esp8266_state.cmd.failures)
		--esp8266_state.cmd.failures;

	if (esp8266_state.cmd.failures >= CMD_MAX_FAILURES) {
		pr_warning(LOG_PFX "Too many failures.  Resetting...\r\n");
		esp8266_state.device.init_state = INIT_STATE_RESET_HARD;
		cmd_set_check(CHECK_WIFI_DEVICE);
	}
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

/* Begin code for sending/receiving data using our internal channels */

static bool channel_is_valid_id(const size_t chan_id)
{
        return chan_id < ARRAY_LEN(esp8266_state.comm.channels);
}

static bool channel_is_open(struct channel* ch)
{
	return ch->in_use;
}

/**
 * Called when we close a channel.
 */
static void channel_close(struct channel* ch)
{
	if (!channel_is_open(ch))
		return;

	serial_close(ch->serial);
	ch->in_use = false;
}

static const char* channel_get_name(const size_t chan_id)
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
        cmd_set_check(CHECK_DATA);
}

/**
 * Sets up a channel by allocating a channel and then creating a Serial
 * device that backs it.
 * @param index Channel ID/index.
 * @param rx_size Size of the Rx Buffer
 * @param tx_size Size of the Tx Buffer
 * @param pointer to the channel structure that was setup, NULL if failure.
 */
static struct channel* channel_setup(const unsigned int index,
				     size_t rx_size, size_t tx_size)
{
        if (!channel_is_valid_id(index))
                return NULL;

        struct channel* ch = esp8266_state.comm.channels + index;
        if (channel_is_open(ch)) {
                pr_warning_int_msg(LOG_PFX "Setup called on allocated "
				   "channel ", index);
		pr_warning(LOG_PFX "Closing stale Serial object\r\n");
		channel_close(ch);
	}

	/*
	 * HACK:
	 * Because I can no longer destory Serial devices due to the
	 * chance of hard fault (issue #847), I must instead reopen
	 * the Serial device if it has already been allocated. This means
	 * that rx_size and tx_size have to be the default sizes for now.
	 * This should get fixed when issue #542 is resolved.  Sorry.
	 */
	/* if (0 == rx_size) */
	/* 	rx_size = SERIAL_BUFF_DEF_RX_SIZE; */

	/* if (0 == tx_size) */
	/* 	tx_size = SERIAL_BUFF_DEF_TX_SIZE; */
	rx_size = SERIAL_BUFF_DEF_RX_SIZE;
	tx_size = SERIAL_BUFF_DEF_TX_SIZE;

	if (NULL == ch->serial) {
		const char* name = channel_get_name(index);
		ch->serial = serial_create(name, tx_size, rx_size, NULL,
					   NULL, _tx_char_cb, ch);
	} else {
		serial_reopen(ch->serial);
	}

	if (NULL == ch->serial) {
		/* Fail state */
		pr_warning(LOG_PFX "Insufficient resources for new channel\r\n");
		return NULL;
	}

	/* Set our channel to open now */
	ch->in_use = true;
        return ch;
}

static int channel_find_serial(struct Serial *serial)
{
        for (size_t i = 0; channel_is_valid_id(i); ++i) {
                struct channel *ch = esp8266_state.comm.channels + i;
                if (serial == ch->serial)
                        return i;
        }

        return INVALID_CHANNEL_ID;
}

static int channel_get_next_available()
{
	for (int i = 0; channel_is_valid_id(i); ++i)
                if (!channel_is_open(esp8266_state.comm.channels + i))
                        return i;

        return INVALID_CHANNEL_ID;

}

static void socket_connect_handler(const size_t chan_id)
{
	/* Check if the connect op is in progress.  If so, take no action */
	struct channel_sync_op *cso = &esp8266_state.comm.connect_op;
	if (cso->chan_id == chan_id)
		return;

	struct channel *ch = esp8266_state.comm.channels + chan_id;
	/* Use Defaults for rx and tx buff sizes */
	if (!channel_setup(chan_id, 0, 0)) {
		/* Close the channel.  Best effort here. */
		channel_close(ch);
		esp8266_close(chan_id, NULL);
		return;
	}

	if (esp8266_state.comm.new_conn_cb) {
		esp8266_state.comm.new_conn_cb(ch->serial);
	} else {
		pr_warning(LOG_PFX "No incoming server callback "
			   "defined.\r\n");
	}
}

static void socket_disconnect_handler(const size_t chan_id)
{
	/* Check if the close op is in progress.  If so, take no action */
	struct channel_sync_op *cso = &esp8266_state.comm.close_op;
	if (cso->chan_id == chan_id)
		return;

	struct channel *ch = esp8266_state.comm.channels + chan_id;
	channel_close(ch);
}

/**
 * Callback that gets invoked when a socket state changes.
 */
static void socket_state_changed_cb(const size_t chan_id,
				    const enum socket_action action)
{
	if (!channel_is_valid_id(chan_id)) {
		pr_warning_int_msg(LOG_PFX "Invalid socket id: ",
				   chan_id);
		return;
	}

	cmd_set_check(CHECK_DATA);
	switch (action) {
	case SOCKET_ACTION_DISCONNECT:
	case SOCKET_ACTION_CONNECT_FAIL:
		pr_info_int_msg(LOG_PFX "Socket closed on channel ",
				chan_id);
		socket_disconnect_handler(chan_id);
		break;
	case SOCKET_ACTION_CONNECT:
		pr_info_int_msg(LOG_PFX "Socket connected on channel ",
				chan_id);
		socket_connect_handler(chan_id);
		break;
	default:
		pr_warning_int_msg(LOG_PFX "Unknown socket action: ",
				   action);
		break;
	}
}

/**
 * Callback that gets invoked by the device code whenever new data
 * comes in.
 */
static void rx_data_cb(int chan_id, size_t len, const char* data)
{
        trigger_led();
        pr_trace_str_msg(LOG_PFX "Rx: ", data);

        if (!channel_is_valid_id(chan_id)) {
                pr_error_int_msg(LOG_PFX "Invalid Channel ID: ", chan_id);
                return;
        }

	/*
	 * Check that we have a backing serial device before we read in
	 * the data. If none, gotta dump it.
	 */
	struct channel *ch = esp8266_state.comm.channels + chan_id;
	if (!channel_is_open(ch)) {
		pr_error(LOG_PFX "Channel is not open.  Dropping data\r\n");
		return;
	}

        xQueueHandle q = serial_get_rx_queue(ch->serial);
        bool data_dropped = false;
        for (size_t i = 0; i < len && !data_dropped; ++i)
                if (!xQueueSend(q, data + i, RX_DATA_TIMEOUT_TICKS))
                        data_dropped = true;

        if (data_dropped)
                pr_warning(LOG_PFX "Rx Buffer Overflow Detected\r\n");

        cmd_set_check(CHECK_DATA);
}

/**
 * Callback that is invoked when the send_data method completes.
 */
static void _send_data_cb(const bool status, const size_t bytes,
			  const unsigned int chan)
{
	/* Failure to send data should not lead to WiFi Reset */
	cmd_completed(true);
	cmd_set_check(CHECK_DATA);

	if (!status) {
		pr_warning_int_msg(LOG_PFX "Failed to send data on "
				   "channel ", chan);

		/*
		 * Hack: Close channel when error b/c we have lost data.
		 * Issue #807
		 */
		struct channel *ch = esp8266_state.comm.channels + chan;
		channel_close(ch);
		esp8266_close(chan, NULL);
		return;
	}
}

/**
 * Method that processes outgoing data if any.	If there is, this
 * will start a command.
 */
static void check_data()
{
	cmd_check_complete(CHECK_DATA);

	for (size_t i = 0; i < ARRAY_LEN(esp8266_state.comm.channels); ++i) {
		struct channel *ch = esp8266_state.comm.channels + i;
		struct Serial* serial = ch->serial;

		/*
		 * Check if connection is still active.	 If not then
		 * there is nothing for us to do.
		 */
		if (!channel_is_open(ch))
			continue;

		/* If the size is 0, nothing to send */
		xQueueHandle q = serial_get_tx_queue(ch->serial);
		const size_t size = uxQueueMessagesWaiting(q);
		if (0 == size)
			continue;

		/* If here, we have a connection and data to send. Do Eet! */
		if (!esp8266_send_data(i, serial, size, _send_data_cb)) {
			pr_warning(LOG_PFX "Failed to queue send "
				   "data command!!!\r\n");
			/* We will retry */
			return;
		}

		cmd_started();
		return;
	}
}

/**
 * Callback that gets invoked when our client wifi state changes.
 */
static void client_state_changed_cb(const enum client_action action)
{
        /* Need to check our client now that a change has occured */
        cmd_set_check(CHECK_WIFI_CLIENT);

	switch (action) {
	case CLIENT_ACTION_CONNECT:
		pr_info(LOG_PFX "Client connected\r\n");
		esp8266_state.client.info.has_ap = true;
		break;
	case CLIENT_ACTION_DISCONNECT:
		pr_info(LOG_PFX "Client disconnected\r\n");
		memset(&esp8266_state.client.info, 0,
		       sizeof(esp8266_state.client.info));
		memset(&esp8266_state.client.ipv4, 0,
		       sizeof(esp8266_state.client.ipv4));
		break;
	case CLIENT_ACTION_GOT_IP:
		pr_info(LOG_PFX "Client got IPv4 address\r\n");
		memset(&esp8266_state.client.ipv4, 0,
		       sizeof(esp8266_state.client.ipv4));

		/*
		 * Don't update client info until we get an IP.  Because
		 * if you try to do this it will fail.  Blah.
		 */
		esp8266_state.client.info_timestamp = 0;
		break;
	default:
		pr_warning(LOG_PFX "BUG: Unknown client action.\r\n");
		break;
	}
}

/* *** Methods that handle device init and reset *** */

/**
 * Callback that is invoked when the init_wifi command completes.
 */
static void init_wifi_cb(const bool status)
{
        cmd_completed(status);
        cmd_set_check(CHECK_WIFI_DEVICE);

        if (!status) {
                /* Then failed to init wifi */
                pr_warning(LOG_PFX "Initialization failed\r\n");
                esp8266_state.device.init_state = INIT_STATE_FAILED;
                return;
        }

        pr_info(LOG_PFX "Initialization successful\r\n");

	/*
	 * Clear out the timestamps and set the checks on all the
	 * subsystems since they will all change after an init.
	 */
	esp8266_state.client.info_timestamp = 0;
	esp8266_state.ap.info_timestamp = 0;
	esp8266_state.cmd.failures = 0;
        esp8266_state.device.init_state = INIT_STATE_READY;

	/* Close all channels if open since we just reset */
	for (unsigned int i = 0; channel_is_valid_id(i); ++i) {
                struct channel *ch = esp8266_state.comm.channels + i;
                channel_close(ch);
        }

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
        if (!esp8266_init(init_wifi_cb)) {
                /* Failed to init critical bits.  */
                pr_warning(LOG_PFX "Failed to init esp8266 device.\r\n");
                cmd_sleep(CHECK_WIFI_DEVICE, INIT_FAIL_SLEEP_MS);
                esp8266_state.device.init_state = INIT_STATE_FAILED;
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
        cmd_completed(status);
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
        cmd_completed(status);
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

static void set_fast_baud()
{
	pr_info_int_msg(LOG_PFX "Increasing baud to ", WIFI_MAX_BAUD);
	cmd_started();

	const bool ok = esp8266_set_uart_config_raw(
		WIFI_MAX_BAUD, ESP8266_SERIAL_DEF_BITS,
		ESP8266_SERIAL_DEF_PARITY, ESP8266_SERIAL_DEF_STOP);

	if (ok) {
		serial_config(esp8266_state.device.serial,
			      ESP8266_SERIAL_DEF_BITS,
			      ESP8266_SERIAL_DEF_PARITY,
			      ESP8266_SERIAL_DEF_STOP,
			      WIFI_MAX_BAUD);
		serial_clear(esp8266_state.device.serial);
	} else {
		pr_warning(LOG_PFX "Set fast baud failed!\r\n");
	}

	cmd_completed(ok);
	cmd_set_check(CHECK_WIFI_DEVICE);
}

/**
 * Method that checks the device state.	 Handles all aspects including
 * initialization, reset, mode, and whatever else may need handling
 * down the road.
 */
static void check_wifi_device()
{
	cmd_check_complete(CHECK_WIFI_DEVICE);
	pr_info(LOG_PFX "Checking WiFi Device...\r\n");

	switch(esp8266_state.device.init_state) {
	case INIT_STATE_FAILED:
		delayMs(CLIENT_BACKOFF_MS);
		/* Fall into hard reset */
	case INIT_STATE_RESET_HARD:
		wifi_device_reset();
	case INIT_STATE_RESET:
	case INIT_STATE_UNINITIALIZED:
		init_wifi();
		return;
	case INIT_STATE_READY:
		/* Then we are where we want to be */
		break;
	}


	/* Check our baud rate and ensure it maxed out */
	const struct serial_cfg* serial_cfg =
		serial_get_config(esp8266_state.device.serial);
	if (serial_cfg->baud != WIFI_MAX_BAUD) {
		set_fast_baud();
		return;
	}


        /* Check if we know our device mode.  If not, get it */
        if (ESP8266_OP_MODE_UNKNOWN == esp8266_state.device.op_mode) {
                /* Get OP mode */
                get_op_mode();
                return;
        }

        /* Now check device mode to ensure its correct */
        enum esp8266_op_mode exp_mode;
        const bool ap_active = esp8266_state.ap.config.active;
        const bool client_active = esp8266_state.client.config.active;
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
        return esp8266_state.device.init_state == INIT_STATE_READY;
}


/* *** Methods that handle the client WiFi state and actions *** */

/**
 * Callback that gets invoked with the get_client_ap command completes.
 */
static void get_client_ap_cb(bool status, const struct esp8266_client_info *ci)
{
        cmd_completed(status);
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
        cmd_completed(status);
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
        cmd_completed(status);
        cmd_set_check(CHECK_WIFI_CLIENT);

        if (!status) {
                /* Failed. */
                pr_info(LOG_PFX "Failed to join network\r\n");
        } else {
                /* If here, we were successful. */
                pr_info(LOG_PFX "Successfully joined network\r\n");
		esp8266_state.client.info_timestamp = 0;
        }

	/* Even if we fail to connect, we have still configured esp8266 */
	esp8266_state.client.configured = true;
}

/**
 * Command that will set the wifi client settings as specified in
 * our settings.
 */
static void set_client_ap()
{
        pr_info(LOG_PFX "Setting Wifi Client Info\r\n");
        const struct wifi_client_cfg *cc = &esp8266_state.client.config;
        pr_info_str_msg(LOG_PFX "Joining network: ", cc->ssid);
        esp8266_join_ap(cc->ssid, cc->passwd, set_client_ap_cb);
        cmd_started();
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
        const struct wifi_client_cfg *cfg = &esp8266_state.client.config;
        const struct esp8266_client_info *ci = &esp8266_state.client.info;
        if (cfg->active) {
                /*
                 * Config says client should be active. Make it so.
                 * First check that we have an AP and that we are on
                 * the correct client network.  If not, try to get on
                 * the correct network.
                 */
                if (!esp8266_state.client.configured) {
                        /* Then we need to try and join the AP */
			set_client_ap();
                        return;
                }

		if (!esp8266_drv_client_connected()) {
			/* Then no client connection yet.  Done */
			pr_info(LOG_PFX "Can't connect to client Wifi\r\n");
			return;
		}

                /* If here, then on client network.  Do we have IP info? */
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
        cmd_completed(status);
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
        cmd_completed(status);
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
        const struct wifi_ap_cfg* cfg = &esp8266_state.ap.config;
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
        const struct wifi_ap_cfg* cfg = &esp8266_state.ap.config;
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
        cmd_completed(status);
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

bool esp8266_drv_update_client_cfg(const struct wifi_client_cfg *wcc)
{
        if (NULL == wcc)
                return false;

	/*
	 * Only trigger the update of a Wifi device if the config settings
	 * have changed. Otherwise no-op.
	 */
	struct wifi_client_cfg* lwcc = &esp8266_state.client.config;
	if (!memcmp(lwcc, wcc, sizeof(*lwcc))) {
		pr_info(LOG_PFX "Client config unchanged by update\r\n");
		return true;
	}

	memcpy(lwcc, wcc, sizeof(*wcc));

        /* Set flags for what states need checking */
	esp8266_state.client.configured = false;
        cmd_sleep(CHECK_WIFI_DEVICE, CFG_UPDATE_SLEEP_MS);
        cmd_sleep(CHECK_WIFI_CLIENT, CFG_UPDATE_SLEEP_MS);

        return true;
}

const struct wifi_client_cfg* esp8266_drv_get_client_config()
{
        return &esp8266_state.client.config;
}

bool esp8266_drv_update_ap_cfg(const struct wifi_ap_cfg *wac)
{
        if (NULL == wac)
                return false;

	/*
	 * Only trigger the update of a Wifi device if the config settings
	 * have changed. Otherwise no-op.
	 */
	struct wifi_ap_cfg* lwac = &esp8266_state.ap.config;
	if (!memcmp(lwac, wac, sizeof(*lwac))) {
		pr_info(LOG_PFX "AP config unchanged by update\r\n");
		return true;
	}

	memcpy(lwac, wac, sizeof(*wac));

        /* Zero this value out so we will forego any backoff attempts */
        esp8266_state.ap.next_set_attempt = 0;

        /* Set flags for what states need checking */
        cmd_sleep(CHECK_WIFI_DEVICE, CFG_UPDATE_SLEEP_MS);
        cmd_sleep(CHECK_WIFI_AP, CFG_UPDATE_SLEEP_MS);

        return true;
}

const struct wifi_ap_cfg* esp8266_drv_get_ap_config()
{
        return &esp8266_state.ap.config;
}

static bool init_channel_sync_op(struct channel_sync_op* op)
{
        if (op->op_semaphore || op->cb_semaphore) {
                pr_error(LOG_PFX "Sync op already initialized\r\n");
                return false;
        }

        op->op_semaphore = xSemaphoreCreateBinary();
        op->cb_semaphore = xSemaphoreCreateBinary();

        if (!op->op_semaphore || !op->cb_semaphore)
                return false;

        xSemaphoreGive(op->op_semaphore);
        return true;
}

bool esp8266_drv_init(struct Serial *s, const int priority,
                      new_conn_func_t new_conn_cb)
{
        /* Initialize the esp8266 hardware */
        if (!wifi_device_init()) {
            pr_error(LOG_PFX "Failed to init WiFi device\r\n");
            goto init_failed;
        }

        if (esp8266_state.device.serial)
                goto init_failed; /* Already setup */

        if (!s) {
                pr_error(LOG_PFX "NULL serial\r\n");
                goto init_failed;
        }
        esp8266_state.device.serial = s;

	/* Probe for our serial device and adjust baud. */
	esp8266_wait_for_ready(s);
	if (!esp8266_probe_device(s, WIFI_MAX_BAUD)) {
		pr_warning(LOG_PFX "Failed to probe WiFi device\r\n");
		goto init_failed;
	}

        /* Create and setup semaphores for connections */
        const bool init_sync_ops =
                init_channel_sync_op(&esp8266_state.comm.connect_op) &&
                init_channel_sync_op(&esp8266_state.comm.close_op);
        if (!init_sync_ops) {
                pr_error(LOG_PFX "Failed to initialize sync op structs\r\n");
                goto init_failed;
        }

        /* Initialize our WiFi configs here */
        LoggerConfig *lc = getWorkingLoggerConfig();
        const struct wifi_client_cfg *cfg =
                &lc->ConnectivityConfigs.wifi.client;
	const struct wifi_ap_cfg* wac =
                &lc->ConnectivityConfigs.wifi.ap;
        if (!esp8266_drv_update_client_cfg(cfg) ||
	    !esp8266_drv_update_ap_cfg(wac)) {
                pr_error(LOG_PFX "Failed to set WiFi client cfg\r\n");
                goto init_failed;
        }

	/* Initialize the esp8266 AT subsystem here */
	if (!esp8266_setup(esp8266_state.device.serial, SERIAL_CMD_MAX_LEN)) {
		pr_warning(LOG_PFX "Failed to setup WiFi AT subsys\r\n");
		goto init_failed;
	}

	/* Setup callback for new connections */
	esp8266_state.comm.new_conn_cb = new_conn_cb;

	/* Set the task loop to check the wifi_device first */
        cmd_set_check(CHECK_WIFI_DEVICE);

        static const signed char task_name[] = TASK_THREAD_NAME;
        const size_t stack_size = TASK_STACK_SIZE;
        xTaskCreate(task, task_name, stack_size, NULL, priority, NULL);

        const signed char* timer_name = (signed char*) "Wifi LED Timer";
        const size_t period = msToTicks(LED_PERIOD_MS);
        esp8266_state.led.timer = xTimerCreate(timer_name, period, false,
                                               NULL, led_timer_cb);
	if (!esp8266_state.led.timer)
		goto init_failed;

        return true;

init_failed:
	pr_warning(LOG_PFX "Init failed\r\n");
	return false;
}

/**
 * Callback method invoked when a callback completes.
 */
static void connect_cb(const bool status, const bool already_connected)
{
        struct channel_sync_op *cso = &esp8266_state.comm.connect_op;
        struct channel *ch = esp8266_state.comm.channels + cso->chan_id;

        if (already_connected) {
		pr_info_int_msg(LOG_PFX "State mismatch. Channel already "
				"connected: ", cso->chan_id);
                channel_close(ch);
        } else if (!status) {
		pr_info_int_msg(LOG_PFX "Failed to connect channel: ",
				cso->chan_id);
		channel_close(ch);
	}

        cmd_set_check(CHECK_DATA);
        xSemaphoreGive(cso->cb_semaphore);
}

/**
 * Synchronous command used to open a new connection to a given destinataion
 * @param proto The protocol to use.
 * @param addr The destination address (either IP or DNS name).
 * @param port The destination port to connect to.
 * @param rx_size The size of the Rx buffer. 0 will yield the default value.
 * @param tx_size The size of the Tx buffer. 0 will yield the default value.
 * @return A valid Serial object if a connection was made, NULL otherwise.
 */
struct Serial* esp8266_drv_connect(const enum protocol proto,
                                   const char* addr,
                                   const unsigned int port,
				   const size_t rx_size,
				   const size_t tx_size)
{
        /* This is synchronous code for now */
        struct channel_sync_op *cso = &esp8266_state.comm.connect_op;
        xSemaphoreTake(cso->op_semaphore, portMAX_DELAY);

        struct Serial* serial = NULL;
        cso->chan_id = channel_get_next_available();
        if (cso->chan_id < 0) {
                pr_warning(LOG_PFX "Failed to acquire a free channel\r\n");
                goto done;
        }

        struct channel *ch = channel_setup(cso->chan_id, rx_size, tx_size);
        if (NULL == ch) {
                pr_warning(LOG_PFX "Can't allocate resources for channel\r\n");
                goto done;
        }

        bool connect_result = false;
        switch(proto) {
        case PROTOCOL_TCP:
                connect_result = esp8266_connect_tcp(cso->chan_id, addr,
                                                     port, -1, connect_cb);
                break;
        case PROTOCOL_UDP:
	{
                const int src_port = 0;
                const enum esp8266_udp_mode udp_mode = ESP8266_UDP_MODE_NONE;
                connect_result = esp8266_connect_udp(cso->chan_id, addr,
                                                     port, src_port,
                                                     udp_mode, connect_cb);
                break;
	}
        }

        if (!connect_result) {
                pr_warning(LOG_PFX "Failed to issue connect command\r\n");
                goto done;
        }

        /* Now wait for our callback to come */
        if (!xSemaphoreTake(cso->cb_semaphore, portMAX_DELAY)) {
                pr_warning(LOG_PFX "Timed out waiting for connect "
                           "response\r\n");
                goto done;
        }

	/* If unsuccessful, the callback will close the channel */
        if (!channel_is_open(ch)) {
                pr_info_str_msg(LOG_PFX "Failed to connect: ", addr);
                goto done;
        }

        serial = ch->serial;
        pr_info_int_msg(LOG_PFX "Connected on comm channel ", cso->chan_id);

done:
	cso->chan_id = INVALID_CHANNEL_ID;
        xSemaphoreGive(cso->op_semaphore);
        return serial;
}

/**
 * Callback invoked when the close command completes.
 */
static void close_cb(const bool status)
{
        struct channel_sync_op *cso = &esp8266_state.comm.close_op;
        cmd_set_check(CHECK_DATA);

	if (!status)
		pr_warning_int_msg(LOG_PFX "Failed to close channel ",
				   cso->chan_id);

        xSemaphoreGive(cso->cb_semaphore);
}

/**
 * Closes an open connection given the Serial object returned by
 * esp8266_drv_connect
 * @param serial Serial object returned by esp8266_drv_connect
 * @return True if the operation was successful, false otherwise.
 */
bool esp8266_drv_close(struct Serial* serial)
{
        /* This is synchronous code for now */
        struct channel_sync_op *cso = &esp8266_state.comm.close_op;
        xSemaphoreTake(cso->op_semaphore, portMAX_DELAY);
        bool status = false;

        cso->chan_id = channel_find_serial(serial);
        if (cso->chan_id < 0) {
                pr_warning(LOG_PFX "Unable to find channel with associated "
                           "serial device.\r\n");
                goto done;
        }

        struct channel *ch = esp8266_state.comm.channels + cso->chan_id;
        if (channel_is_open(ch)) {
		channel_close(ch);
	}

        if (!esp8266_close(cso->chan_id, close_cb)) {
                pr_warning(LOG_PFX "Failed to issue close command\r\n");
                goto done;
        }

        /* Now wait for our callback to come */
        if (!xSemaphoreTake(cso->cb_semaphore, portMAX_DELAY)) {
                pr_warning(LOG_PFX "Timed out waiting for close "
                           "response\r\n");
                goto done;
        }

done:
	cso->chan_id = INVALID_CHANNEL_ID;
        xSemaphoreGive(cso->op_semaphore);
        return status;
}

const struct esp8266_ipv4_info* get_client_ipv4_info()
{
        return &esp8266_state.client.ipv4;
}

const struct esp8266_ipv4_info* get_ap_ipv4_info()
{
        return &esp8266_state.ap.ipv4;
}

/**
 * Tells us if the WiFi client is connected to the configured wireless network.
 * @return true if it is, false otherwise.
 */
bool esp8266_drv_client_connected()
{
        return esp8266_state.client.info.has_ap;
}

/**
 * Tells us if the driver is in an initialized and ready state.
 * @return true if it is, false otherwise.
 */
bool esp8266_drv_is_initialized()
{
	return esp8266_state.device.init_state == INIT_STATE_READY;
}
