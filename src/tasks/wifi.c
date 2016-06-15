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
#include "cpu_device.h"
#include "dateTime.h"
#include "esp8266.h"
#include "esp8266_drv.h"
#include "macros.h"
#include "messaging.h"
#include "loggerApi.h"
#include "loggerSampleData.h"
#include "panic.h"
#include "printk.h"
#include "rx_buff.h"
#include "serial.h"
#include "serial_device.h"
#include "semphr.h"
#include "task.h"
#include "taskUtil.h"
#include "timers.h"
#include "queue.h"
#include "wifi.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

/* Time between beacon messages */
#define BEACON_PERIOD_MS	1000
/* Prefix for all log messages */
#define LOG_PFX			"[wifi] "
/* How long to wait between polling our incomming msg Serial */
#define READ_DELAY_MS		1
/* How long to wait before giving up on the message */
#define READ_TIMEOUT_MS		20
/* Max size of an incomming message */
#define RX_BUFF_SIZE		512
/* How much stack does this task deserve */
#define STACK_SIZE		256
/* Make all task names 16 chars including NULL char */
#define THREAD_NAME		"WiFi Task      "
/* How many events can be pending before we overflow */
#define WIFI_EVENT_QUEUE_DEPTH	8
/* The highest channel WiFi can use (USA) */
#define WIFI_MAX_CHANNEL	11

static struct {
        xQueueHandle event_queue;
        struct {
                struct rx_buff* rxb;
                tiny_millis_t timeout;
        } rx_msgs;
        struct {
                struct Serial* serial;
        } beacon;
} state;

static void log_event_overflow(const char* event_name)
{
        pr_warning_str_msg(LOG_PFX "Event overflow: ", event_name);
}

/* *** All methods that are used to generate events *** */

/**
 * Structure used to house sample data for a wifi telemetry stream until
 * the event handler picks it up.
 */
struct wifi_sample_data {
        struct Serial* serial;
        const struct sample* sample;
        size_t tick;
};

/**
 * Event struct used for all events that come into our wifi task
 */
struct wifi_event {
        enum task {
                TASK_BEACON,
                TASK_RX_DATA,
                TASK_SAMPLE,
        } task;
        union {
                struct wifi_sample_data sample;
                struct Serial* serial;
        } data;
};

static void _new_conn_cb(struct Serial *s)
{
        /* Send event message here to wake the task */
        struct wifi_event event = {
                .task = TASK_RX_DATA,
                .data.serial = s,
        };

        if (!xQueueSend(state.event_queue, &event, 0))
                log_event_overflow("New Connection");
}

static void beacon_timer_cb( xTimerHandle xTimer )
{
        /* Send the message here to wake the timer */
        struct wifi_event event = {
                .task = TASK_BEACON,
        };

        if (!xQueueSend(state.event_queue, &event, 0))
                log_event_overflow("Beacon");
}

static void wifi_sample_cb(struct Serial* const serial,
                           const struct sample* sample,
                           const int tick)
{
        /*
         * Gotta malloc a small buff b/c stuff to send. We will free this
         * in the event handler below.
         */
        const struct wifi_sample_data data_sample = {
                .serial = serial,
                .sample = sample,
                .tick = tick,
        };

        struct wifi_event event = {
                .task = TASK_SAMPLE,
                .data.sample = data_sample,
        };

        /* Send the message here to wake the timer */
        if (!xQueueSend(state.event_queue, &event, 0))
                log_event_overflow("Sample CB");
}


/* *** Wifi Serial IOCTL Handlers *** */

static int wifi_serial_ioctl(struct Serial* serial, unsigned long req,
                             void* argp)
{
        switch(req) {
        case SERIAL_IOCTL_TELEMETRY_ENABLE:
        {
                pr_info_int_msg(LOG_PFX "Starting telem stream on serial ",
                                (int) (long) serial);
                const int rate = (int) (long) argp;
                const bool success =
                        logger_sample_register_sample_cb(wifi_sample_cb,
                                                         serial) &&
                        logger_sample_enable_sample_cb(serial, rate);

                return success ? 0 : -2;
        }
        case SERIAL_IOCTL_TELEMETRY_DISABLE:
        {
                pr_info_int_msg(LOG_PFX "Stopping telem stream on serial ",
                                (int) (long) serial);

                const bool success = logger_sample_disable_sample_cb(serial);

                return success ? 0 : -2;
        }
        default:
                pr_warning_int_msg(LOG_PFX "Unhandled ioctl request: ",
                                   (int) req);
                return -1;
        }
}


/* *** All methods that are used to handle Rx Msgs *** */

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
static void process_rx_msgs(struct Serial* s)
{
        /* Set the Serial IOCTL handler here b/c this is managing task */
        serial_set_ioctl_cb(s, wifi_serial_ioctl);

        struct rx_buff* const rxb = state.rx_msgs.rxb;

        while(true) {
                /* Never echo while reading in WiFi code */
                rx_buff_read(rxb, s, false);

                switch (rx_buff_get_status(rxb)) {
                case RX_BUFF_STATUS_EMPTY:
                        /* Read and there was nothing.  We are done */
                        goto rx_done;
                case RX_BUFF_STATUS_PARTIAL:
                        /* Partial message. Wait for reset or timeout */
                        if (process_partial_and_continue())
                                continue;

                        /* If here, then rx timeout. */
                        pr_warning(LOG_PFX "Rx message timeout\r\n");
                        goto rx_done;
                case RX_BUFF_STATUS_READY:
                case RX_BUFF_STATUS_OVERFLOW:
                        /* A message (possibly partial) awaits us */
                        process_ready_msg(s);
                        goto rx_done;
                }
        }

rx_done:
        rx_buff_clear(rxb);
        state.rx_msgs.timeout = 0;
}


/* *** All methods that are used to handle sending beacons *** */

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
        put_crlf(serial);
}

static void do_beacon()
{
        const struct esp8266_ipv4_info* client_ipv4 = get_client_ipv4_info();
        const struct esp8266_ipv4_info* softap_ipv4 = get_ap_ipv4_info();
        if (!*client_ipv4->address && !*softap_ipv4->address) {
                /* Then don't bother since we don't have any IP addresses */
                return;
        }

        /*
         * If here then we have at least one IP.  Send the beacon.  Now
         * we need a channel to send the beacon on. Lets grab one if we
         * don't have one yet.
         */
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
                client_ipv4->address,
                softap_ipv4->address,
                NULL,
        };
        send_beacon(serial, ips);

        /* Now flush all incomming data since we don't care about it. */
        serial_purge_rx_queue(serial);
}


static void process_sample(struct wifi_sample_data* data)
{
        struct Serial* serial = data->serial;
        const struct sample* sample = data->sample;
        const size_t ticks = data->tick;
        const bool meta = ticks == 0;

        if (ticks != sample->ticks) {
                /* Then the sample has changed underneath us */
                pr_warning(LOG_PFX "Stale sample.  Dropping \r\n");
                return;
        }

        api_send_sample_record(serial, sample, ticks, meta);
        put_crlf(serial);
}


/* *** Task loop and all public methods *** */

static void _task(void *params)
{
        for(;;) {
                struct wifi_event event;
                if (!xQueueReceive(state.event_queue, &event,
                                   portMAX_DELAY))
                        continue;

                /* If here we have an event. Process it */
                switch (event.task) {
                case TASK_BEACON:
                        do_beacon();
                        break;
                case TASK_RX_DATA:
                        /* The event data will have the serial device */
                        process_rx_msgs(event.data.serial);
                        break;
                case TASK_SAMPLE:
                        process_sample(&event.data.sample);
                        break;
                default:
                        panic(PANIC_CAUSE_UNREACHABLE);
                }
        }

        panic(PANIC_CAUSE_UNREACHABLE);
}

bool wifi_init_task(const int wifi_task_priority,
                    const int wifi_drv_priority)
{
        /* Get our serial port setup */
        struct Serial *s = serial_device_get(SERIAL_WIFI);
        if (!s)
                return false;

        state.event_queue = xQueueCreate(WIFI_EVENT_QUEUE_DEPTH,
                                         sizeof(struct wifi_event));
        if (!state.event_queue)
                return false;

        /* Allocate our RX buffer for incomming data */
        state.rx_msgs.rxb = rx_buff_create(RX_BUFF_SIZE);
        if (!state.rx_msgs.rxb)
                return false;

        if (!esp8266_drv_init(s, wifi_drv_priority, _new_conn_cb))
                return false;

        static const signed char task_name[] = THREAD_NAME;
        const size_t stack_size = STACK_SIZE;
        xTaskCreate(_task, task_name, stack_size, NULL,
                            wifi_task_priority, NULL);

        const size_t timer_ticks = msToTicks(BEACON_PERIOD_MS);
        const signed char* timer_name = (signed char*) "Wifi Beacon Timer";
        xTimerHandle timer_handle = xTimerCreate(timer_name, timer_ticks,
                                                 true, NULL, beacon_timer_cb);
        if (!timer_handle)
                return false;

        /* Start the timer, but delay 1 period to give our WiFi unit time */
        xTimerStart(timer_handle, timer_ticks);

        return true;
}


/* *** Wifi configuration methods *** */

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

/**
 * Wrapper for the driver to update the ap wifi config
 * settings.
 */
bool wifi_update_ap_config(struct wifi_ap_cfg *wac)
{
        return esp8266_drv_update_ap_cfg(wac);
}

void wifi_reset_config(struct wifi_cfg *cfg)
{
        /* For now simply zero this out */
        memset(cfg, 0, sizeof(struct wifi_cfg));

        /* Enable WiFi by Default */
        cfg->active = true;

        /*
         * Set some sane values for the AP configuration.  We turn on our
         * AP by default because it gives us a way to communicate with the
         * RCT device.  We make the SSID as unique as possible using our
         * serial number.  Assumption is that user will adjust it when they
         * configure it.
         */
        const char* cpu_serial = cpu_device_get_serialnumber();
        const char* prefix = FRIENDLY_DEVICE_NAME" ";
        const int cpu_serial_len = strlen(cpu_serial);
        int offset = cpu_serial_len - ARRAY_LEN(cfg->ap.ssid) +
                strlen(prefix) + 1;
        if (offset < 0)
                offset = 0;
        if (offset > cpu_serial_len)
                offset = cpu_serial_len;
        snprintf(cfg->ap.ssid, ARRAY_LEN(cfg->ap.ssid), "%s%s",
                 prefix, cpu_serial + offset);

        cfg->ap.active = true;
        cfg->ap.channel = 11;
        cfg->ap.encryption = ESP8266_ENCRYPTION_NONE;

        /* Inform the Wifi device that settings may have changed */
        wifi_update_client_config(&cfg->client);
        wifi_update_ap_config(&cfg->ap);
}

/**
 * Validates that a given Wifi Ap Configuration is valid
 * for use.
 * @return true if it is, false otherwise.
 */
bool wifi_validate_ap_config(const struct wifi_ap_cfg *wac)
{
        return NULL != wac &&
                wac->channel > 0 &&
                wac->channel <= WIFI_MAX_CHANNEL &&
                wac->encryption >= 0 &&
                wac->encryption <= __ESP8266_ENCRYPTION_MAX;
}

/**
 * Gets the string representation of the enum esp8266_encryption value.
 * @return The corresponding string if a match, "unknown" otherwise.
 */
const char* wifi_api_get_encryption_str_val(const enum esp8266_encryption enc)
{
        switch(enc) {
        case ESP8266_ENCRYPTION_NONE:
                return "none";
        case ESP8266_ENCRYPTION_WEP:
                return "wep";
        case ESP8266_ENCRYPTION_WPA_PSK:
                return "wpa";
        case ESP8266_ENCRYPTION_WPA2_PSK:
                return "wpa2";
        case ESP8266_ENCRYPTION_WPA_WPA2_PSK:
                return "wpa/wpa2";
        default:
                return "unknown"; /* Put our default here */
        }
}

/**
 * Gets the enum esp8266_encryption representation of the string value.
 * @return The corresponding enum esp8266_encryption value if a match is
 * found, -1 otherwise.
 */
enum esp8266_encryption wifi_api_get_encryption_enum_val(const char* str)
{
        if (STR_EQ(str, "none"))
                return ESP8266_ENCRYPTION_NONE;

        if (STR_EQ(str, "wep"))
                return ESP8266_ENCRYPTION_WEP;

        if (STR_EQ(str, "wpa"))
                return ESP8266_ENCRYPTION_WPA_PSK;

        if (STR_EQ(str, "wpa2"))
                return ESP8266_ENCRYPTION_WPA2_PSK;

        if (STR_EQ(str, "wpa/wpa2"))
                return ESP8266_ENCRYPTION_WPA_WPA2_PSK;

        return ESP8266_ENCRYPTION_INVALID;
}
