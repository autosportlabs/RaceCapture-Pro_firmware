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

#ifndef API_EVENT_H_
#define API_EVENT_H_
#include "cpp_guard.h"
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include "serial.h"
CPP_GUARD_BEGIN

#define MAX_ALERTMESSAGE_LENGTH 16

enum ApiEventType {
        ApiEventType_Alertmessage,
        ApiEventType_AlertmsgAck,
        ApiEventType_AlertmsgReply,
        ApiEventType_ButtonState
};

struct alertmessage {
        uint8_t priority;
        uint32_t id;
        char message[MAX_ALERTMESSAGE_LENGTH + 1];
};

struct alertmessage_ack {
        struct Serial *source;
        uint32_t id;
};

struct button_state {
        uint8_t button_id;
        uint8_t state;
};

struct api_event {
        enum ApiEventType type;
        struct Serial *source;
        union {
                struct alertmessage alertmsg;
                struct alertmessage_ack alertmsg_ack;
                struct button_state butt_state;
        } data;
};

typedef void api_event_cb_t(const struct api_event * api_event, void* data);

void api_event_process_callbacks(const struct api_event * api_event);

int api_event_create_callback(api_event_cb_t* cb, void* data);

bool api_event_destroy_callback(const int handle);

void process_api_event(const struct api_event * api_event, struct Serial * serial);

CPP_GUARD_END

#endif /* API_EVENT_H_ */
