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

#include "api_event.h"
#include "macros.h"
#include "loggerApi.h"
#include "printk.h"

#define API_EVENT_CB_REGISTRY_SIZE 8

#define LOG_PFX "[api_event] "

struct api_event_cb_registry {
        api_event_cb_t* cb;
        void* data;
} api_event_cb_registry[API_EVENT_CB_REGISTRY_SIZE] = {0};

static bool is_valid_registry_index(const size_t idx)
{
        return idx < ARRAY_LEN(api_event_cb_registry);
}

/**
 * Processes callbacks for api_events
 * @param api_event the api_event object to be processed
 */
void api_event_process_callbacks(const struct api_event * api_event)
{
        for (size_t i = 0; is_valid_registry_index(i); ++i) {
                struct api_event_cb_registry* slot = api_event_cb_registry + i;
                if (slot->cb) {
                        slot->cb(api_event, slot->data);
                }
        }
}

/**
 * Sets up a callback for api events
 * @param cb The method to call back.
 * @param data User defined data that will be provided to the callback.
 * @return A handle that references this callback's registration id; else -1
 * if there was an error.
 */
int api_event_create_callback(api_event_cb_t* cb, void* data)
{
        for (size_t i = 0; cb && is_valid_registry_index(i); ++i) {
                struct api_event_cb_registry* slot = api_event_cb_registry + i;
                if (slot->cb)
                        continue;

                /* If here then we found a slot */
                slot->cb = cb;
                slot->data = data;
                return i;
        }

        return -1;
}

/**
 * Destroys a callback created by the #api_event_create_callback method.
 * Requires the handle returned by the creation process to destroy it.
 */
bool api_event_destroy_callback(const int handle)
{
        if (!is_valid_registry_index(handle))
                return false;

        struct api_event_cb_registry* slot = api_event_cb_registry + handle;
        memset(slot, 0, sizeof(struct api_event_cb_registry));
        return true;
}

void process_api_event(const struct api_event * api_event, struct Serial * serial)
{
        /* Dispatch API messages to connected devices */

        /* Only try to send if our Serial device is connected */
        if (!serial_is_connected(serial))
                return;

        /* Don't send API events to the originating device */
        if (serial == api_event->source) {
                pr_trace(LOG_PFX " Skipping sending to same source\r\n");
                return;
        }
        switch (api_event->type) {
        case ApiEventType_Alertmessage:
                api_send_alertmessage(serial, &api_event->data.alertmsg);
                put_crlf(serial);
                break;
        case ApiEventType_AlertmsgReply:
                api_send_alertmsg_reply(serial, &api_event->data.alertmsg);
                put_crlf(serial);
                break;
        case ApiEventType_AlertmsgAck:
                api_send_alertmsg_ack(serial, &api_event->data.alertmsg_ack);
                put_crlf(serial);
                break;
        case ApiEventType_ButtonState:
                api_send_button_state(serial, &api_event->data.butt_state);
                put_crlf(serial);
                break;
        case ApiEventType_LoggerStatus:
                api_send_logging_status(serial);
                put_crlf(serial);
                break;

        default:
                pr_warning_int_msg(LOG_PFX "Unknown ApiEvent type ", api_event->type);
                break;
        }
}
