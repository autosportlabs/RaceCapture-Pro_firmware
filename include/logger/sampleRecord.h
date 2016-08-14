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

#ifndef SAMPLERECORD_H_
#define SAMPLERECORD_H_

#include "FreeRTOS.h"
#include "channel_config.h"
#include "cpp_guard.h"
#include "dateTime.h"
#include "loggerConfig.h"
#include "queue.h"

#include <stdbool.h>
#include <stddef.h>

CPP_GUARD_BEGIN

enum LoggerMessageType {
    LoggerMessageType_Sample,
    LoggerMessageType_Start,
    LoggerMessageType_Stop,
};

/*
 * Important note here.  Use of Int and LongLong work on 32 bit ARM
 * architectures only in terms of their desired consistency of supplying
 * 32 and 64 bit values respectively.  We should change them to int32_t
 * and int64_t if we want to gaurantee this going forward.
 */
enum SampleData {
    SampleData_Int_Noarg,
    SampleData_Int,
    SampleData_LongLong_Noarg,
    SampleData_LongLong,
    SampleData_Float_Noarg,
    SampleData_Float,
    SampleData_Double_Noarg,
    SampleData_Double,
};

typedef struct _ChannelSample {
    ChannelConfig *cfg;
    size_t channelIndex;
    bool populated;

    enum SampleData sampleData;
    union {
        int (*get_int_sample)(int);
        long long (*get_longlong_sample)(int);
        float (*get_float_sample)(int);
        double (*get_double_sample)(int);
        int (*get_int_sample_noarg)();
        long long (*get_longlong_sample_noarg)();
        float (*get_float_sample_noarg)();
        double (*get_double_sample_noarg)();
    };

    union {
        int valueInt;
        long long valueLongLong;
        float valueFloat;
        double valueDouble;
    };
} ChannelSample;

struct sample {
   size_t ticks;
   size_t channel_count;
   ChannelSample *channel_samples;
};

typedef struct _LoggerMessage {
        enum LoggerMessageType type;
        size_t ticks;
        struct sample *sample;
} LoggerMessage;

/**
 * Initializes the struct sample channel_sample buffer for use.  May be called
 * again to re-initialize the space.
 * @param s Pointer to the struct sample to initialize.
 * @param count Number of channels that we are logging.
 * @return The amount of space allocated.
 */
size_t init_sample_buffer(struct sample *s, const size_t count);

/**
 * Frees the channel_sample buffer assocaited with the struct sample.  Also
 * clears out the struct sample buffer values to indicated that the buffer has
 * been released.  Call this like you would use a free method.
 * @param s Pointer to the struct sample to reap.
 */
void free_sample_buffer(struct sample *s);

/**
 * Creates a LoggerMessage for use in the messaging between threads.
 * @param t The messaget type.
 * @param ticks The logger tick value.
 * @param s The associated sample object (if any).
 */
LoggerMessage create_logger_message(const enum LoggerMessageType t,
                                    const size_t ticks, struct sample *s);

/**
 * Receives and validates a LoggerMessage from the provided queue.  If the
 * LoggerMessage is invalid, then it is ignored.
 * @param queue The Queue containing the message
 * @param lm The LoggerMessage structure to populate.
 * @param timeout The amount of time to wait before timing out.
 * @return The return value of xQueueReceive.  This must be checked before
 * the LoggerMessage can be considered valid.
 */
char receive_logger_message(xQueueHandle queue, LoggerMessage *lm,
                            portTickType timeout);

bool is_sample_data_valid(const LoggerMessage *lm);

/**
 * Creates a brand new LoggerMessage queue.  This is useful for sending
 * LoggerMessage objects to all the little subscribers that need to get
 * them.  Its size is always equal to the number of sample buffers we
 * allocate.  This is because there is little point in allocating more
 * queue space than we have buffers.
 * @return A newly allocated queue.
 */
xQueueHandle create_logger_message_queue();

/**
 * Enqueues a LoggerMessage onto a provided queue.
 * @param queue The queue to append the message to.
 * @param msg The message to put into the queue.
 * @return pdTRUE if successful, or an error code otherwise.
 */
portBASE_TYPE send_logger_message(const xQueueHandle queue,
                                  const LoggerMessage * const msg);

CPP_GUARD_END

#endif /* SAMPLERECORD_H_ */
