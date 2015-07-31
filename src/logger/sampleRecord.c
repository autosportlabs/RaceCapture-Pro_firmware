/*
 * sampleRecord.c
 *
 *  Created on: Feb 29, 2012
 *      Author: brent
 */
#include "sampleRecord.h"
#include "loggerConfig.h"
#include "mem_mang.h"
#include "FreeRTOS.h"
#include "taskUtil.h"
#include "loggerSampleData.h"

#include <stdbool.h>

size_t init_sample_buffer(struct sample *s, const size_t count)
{
        if (s->channel_samples)
                free_sample_buffer(s);

        const size_t size = sizeof(ChannelSample[count]);
        s->channel_samples = (ChannelSample *) portMalloc(size);

        if (NULL == s->channel_samples)
                return 0;

        s->ticks = 0;
        s->channel_count = count;
        init_channel_sample_buffer(getWorkingLoggerConfig(), s);

        return size;
}

void free_sample_buffer(struct sample *s)
{
        portFree(s->channel_samples);
        s->channel_samples = NULL;
}

bool is_sample_data_valid(const LoggerMessage *lm)
{
        /* Only validate messages with non-null samples */
        return NULL == lm->sample ? true : lm->ticks == lm->sample->ticks;
}

portBASE_TYPE send_logger_message(const xQueueHandle queue,
                                  const LoggerMessage * const msg)
{
        return NULL == queue ? errQUEUE_EMPTY : xQueueSend(queue, msg, 0);
}


char receive_logger_message(xQueueHandle queue, LoggerMessage *lm,
                            portTickType timeout)
{
        char res;

        do {
                res = xQueueReceive(queue, lm, timeout);
        } while (pdTRUE == res && !is_sample_data_valid(lm));

        return res;
}

xQueueHandle create_logger_message_queue(const size_t len)
{
        return xQueueCreate(len, sizeof(LoggerMessage));
}

LoggerMessage create_logger_message(const enum LoggerMessageType t,
                                    struct sample *s)
{
        const size_t ticks = getCurrentTicks();
        LoggerMessage msg;

        msg.type = t;
        msg.ticks = ticks;
        msg.sample = s;

        /* Set matching timestamps.  Needed for validation */
        if (s)
                s->ticks = ticks;

        return msg;
}
