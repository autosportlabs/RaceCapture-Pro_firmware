/*
 * sampleRecord.h
 *
 *  Created on: Feb 29, 2012
 *      Author: brent
 */

#ifndef SAMPLERECORD_H_
#define SAMPLERECORD_H_

#include "dateTime.h"
#include "FreeRTOS.h"
#include "loggerConfig.h"
#include "queue.h"

#include <stdbool.h>
#include <stddef.h>

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
 * @param s The associated sample object (if any).
 */
LoggerMessage create_logger_message(const enum LoggerMessageType t,
                                    struct sample *s);

/**
 * Tests if the given LoggerMessage points to valid data by comparing
 * timestamps.
 * @param lm The LoggerMessage to validate
 * @return true if valid, false otherwise
 */
bool is_sample_data_valid(const LoggerMessage *lm);

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

/**
 * Checks to ensure that the LoggerMessage object is pointing to a usable
 * data_sample structure.  This is needed because while the LoggerMessages are
 * deeply copied to their respective queues, the data_sample structures are not
 * (they are a part of a ring buffer).  So we must validate that the timestamp
 * on the LoggerMessage matches that of the data_sample object.
 * @param lm The LoggerMessage object.
 * @return True if the data_sample is usable, false otherwise.
 */
bool is_data_sample_valid(const LoggerMessage *lm);

/**
 * Creates a brand new LoggerMessage queue.  This is useful for sending
 * LoggerMessage objects to all the little subscribers that need to get
 * them.
 * @return A newly allocated queue.
 */
xQueueHandle create_logger_message_queue(const size_t len);

/**
 * Enqueues a LoggerMessage onto a provided queue.
 * @param queue The queue to append the message to.
 * @param msg The message to put into the queue.
 * @return pdTRUE if successful, or an error code otherwise.
 */
portBASE_TYPE send_logger_message(const xQueueHandle queue,
                                  const LoggerMessage * const msg);


#endif /* SAMPLERECORD_H_ */
