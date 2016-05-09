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
#include "capabilities.h"
#include "constants.h"
#include "cpu.h"
#include "dateTime.h"
#include "esp8266_drv.h"
#include "macros.h"
#include "messaging.h"
#include "panic.h"
#include "printk.h"
#include "rx_buff.h"
#include "serial.h"
#include "serial_device.h"
#include "semphr.h"
#include "task.h"
#include "taskUtil.h"
#include "wifi.h"
#include <stdbool.h>

/* Time between beacon messages */
#define BEACON_PERIOD_MS	1000
/* Prefix for all log messages */
#define LOG_PFX			"[wifi] "
/* How long to wait between polling our incomming msg Serial */
#define READ_DELAY_MS		3
/* How long to wait before giving up on the message */
#define READ_TIMEOUT_MS		100
/* Max size of an incomming message */
#define RX_BUFF_SIZE		512
/* How much stack does this task deserve */
#define STACK_SIZE		256
/* Make all task names 16 chars including NULL char */
#define THREAD_NAME		"WiFi Task      "

struct rx_msgs {
        struct Serial* serial;
        struct rx_buff* rxb;
        tiny_millis_t timeout;
        xSemaphoreHandle semaphor;
};

struct beacon {
        tiny_millis_t time_last;
        struct Serial* serial;
};

static struct {
        struct rx_msgs rx_msgs;
        struct beacon beacon;
} state;

static void _new_conn_cb(struct Serial *s)
{
        /* Set the incoming connection serial pointer and release the task */
        state.rx_msgs.serial = s;
        xSemaphoreGive(state.rx_msgs.semaphor);
}

static void reset_rx_state()
{
        rx_buff_clear(state.rx_msgs.rxb);
        state.rx_msgs.timeout = 0;
}

/**
 * Code used to process part of a message.
 * @return false if we have timed out waiting for the message, true otherwise.
 */
static bool process_partial_and_continue()
{
        /* If here then no timeout has been set.  Set one */
        if (!state.rx_msgs.timeout)
                state.rx_msgs.timeout = date_time_uptime_now_plus(READ_TIMEOUT_MS);

        if (date_time_is_past(state.rx_msgs.timeout)) {
                /* Then timeout has passed */
                return false;
        }

        /* Timeout hasn't passed, wait a bit and retry */
        delayMs(READ_DELAY_MS);
        return true;
}

/**
 * Code that gets invoked when we have a complete message in our rx_buff
 * that is ready to be processed.
 */
static void process_ready_msg(struct Serial* s)
{
        char *data_in = rx_buff_get_msg(state.rx_msgs.rxb);
        pr_info_str_msg(LOG_PFX "Received CMD: ", data_in);
        process_read_msg(s, data_in, strlen(data_in));
}

/**
 * Handles all of our incoming messages and what we do with them.
 */
static void process_rx_msgs()
{
        struct Serial* s = state.rx_msgs.serial;
        state.rx_msgs.serial = NULL;

        while(true) {
                struct rx_buff* const rxb = state.rx_msgs.rxb;
                rx_buff_read(rxb, s);

                switch (rx_buff_get_status(rxb)) {
                case RX_BUFF_STATUS_EMPTY:
                        /* Read and there was nothing.  We are done */
                        reset_rx_state();
                        return;
                case RX_BUFF_STATUS_PARTIAL:
                        /* Partial message. Wait for reset or timeout */
                        if (process_partial_and_continue())
                                continue;

                        reset_rx_state();
                        return;
                default:
                        /* A message awaits us */
                        process_ready_msg(s);
                        reset_rx_state();
                        return;
                }
        }
}

static tiny_millis_t get_next_beacon_wkup_time()
{
        /* Figure out the next we need to update the beacon */
        return state.beacon.time_last + BEACON_PERIOD_MS;
}

static bool time_for_beacon()
{
        return state.beacon.time_last <= getUptime();
}

static void send_beacon(struct Serial* serial, const char* ips[])
{
        json_objStart(serial);
        json_objStartString(serial, "beacon");

        json_string(serial, "name", DEVICE_NAME, true);
        json_int(serial, "port", RCP_SERVICE_PORT, true);
        json_string(serial, "serial", cpu_get_serialnumber(), true);

        json_arrayStart(serial, "ip");
        while(*ips) {
                const char* ip = *ips;
                /* Advance to next non-zero len string or end or array */
                for(++ips; *ips && 0 == **ips; ++ips);
                const bool more = !!*ips;
                /* Don't add empty strings to the array */
                if (0 != *ip)
                        json_arrayElementString(serial, ip, more);
        }
        json_arrayEnd(serial, false);

        json_objEnd(serial, false);
        json_objEnd(serial, false);
}

static void do_beacon()
{
        /* Update the time the beacon last went out */
        state.beacon.time_last = getUptime();

        const struct esp8266_client_info* ci = esp8266_drv_get_client_info();
        if (!ci->has_ap) {
                /* Then don't bother since we don't have a connection */
                return;
        }

        /* If here then we have at least one connection.  Send the beacon */
        if (NULL == state.beacon.serial) {
                state.beacon.serial =
                        esp8266_drv_connect(PROTOCOL_UDP, "255.255.255.255",
                                            RCP_SERVICE_PORT);
        }

        struct Serial* serial = state.beacon.serial;
        if (!serial) {
                pr_warning("Unable to create Serial for Beacon\r\n");
                return;
        }

        const char* ips[] = {
                ci->ip,
                NULL,
        };
        send_beacon(serial, ips);
}

static tiny_millis_t get_task_sleep_time()
{
        const tiny_millis_t next = get_next_beacon_wkup_time();
        const tiny_millis_t now = getUptime();

        return now >= next ? 0 : next - now;
}

static void _task_loop()
{
        const tiny_millis_t sleep_time = get_task_sleep_time();
        const bool awoke_by_rx =
                pdTRUE == xSemaphoreTake(state.rx_msgs.semaphor, sleep_time);

        if (awoke_by_rx)
                process_rx_msgs();

        if (time_for_beacon())
                do_beacon();
}

static void _task(void *params)
{
        for(;;)
                _task_loop();

        panic(PANIC_CAUSE_UNREACHABLE);
}

bool wifi_init_task(const int wifi_task_priority,
                    const int wifi_drv_priority)
{
        /* Get our serial port setup */
        struct Serial *s = serial_device_get(SERIAL_WIFI);
        if (!s)
                return false;

        state.rx_msgs.semaphor = xSemaphoreCreateBinary();
        if (!state.rx_msgs.semaphor)
                return false;

        /* Allocate our RX buffer for incomming data */
        state.rx_msgs.rxb = rx_buff_create( RX_BUFF_SIZE);
        if (!state.rx_msgs.rxb)
                return false;

        if (!esp8266_drv_init(s, wifi_drv_priority, _new_conn_cb))
                return false;

        static const signed char task_name[] = THREAD_NAME;
        const size_t stack_size = STACK_SIZE;
        xTaskCreate(_task, task_name, stack_size, NULL,
                            wifi_task_priority, NULL);

        return true;
}

void wifi_reset_config(struct wifi_cfg *cfg)
{
        /* For now simply zero this out */
        memset(cfg, 0, sizeof(struct wifi_cfg));

        /* Inform the Wifi device that settings may have changed */
        wifi_update_client_config(&cfg->client);
}

/**
 * Wrapper for the driver to update the client wifi config
 * settings.  This is here because it makes sense for these
 * calls to enter the Wifi subsystem here instead of through
 * the driver directly.
 */
bool wifi_update_client_config(struct wifi_client_cfg *wcc)
{
        return esp8266_drv_update_client_cfg(wcc);
}
