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
        s->ticks = 0;
        s->channel_count = count;
        s->channel_samples = (ChannelSample *) portMalloc(size);

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
        /* Only LoggerMessageType_Sample will have sample data to validate */
        if (lm->type != LoggerMessageType_Sample)
                return true;

        return lm->ticks == lm->sample->ticks;
}

char receive_logger_message(xQueueHandle queue, LoggerMessage *lm,
                            portTickType timeout)
{
        char res;

        do {
                res = xQueueReceive(queue, lm, timeout);
        } while (pdPASS == res && !is_sample_data_valid(lm));

        return res;
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
