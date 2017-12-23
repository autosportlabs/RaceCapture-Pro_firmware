/*
 * Race Capture Pro Firmware
 *
 * Copyright (C) 2015 Autosport Labs
 *
 * This file is part of the Race Capture Pro fimrware suite
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
#include "loggerConfig.h"
#include "loggerSampleData.h"
#include "mem_mang.h"
#include "sampleRecord.h"
#include "taskUtil.h"
#include "macros.h"
#include <stdbool.h>
#include "printk.h"

#define LOG_PFX "[sampleRecord] "
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

bool get_sample_value_by_name(const struct sample *s, const char * name, double *value)
{
    if (!s || !value || !name) return false;

    for (size_t i = 0; i < s->channel_count; i++){
            ChannelSample *sam = (s->channel_samples + i);
            if (!STR_EQ(name, sam->cfg->label)) continue;

            if (!sam->populated) return false;

            switch(sam->sampleData) {
                    case SampleData_Float:
                    case SampleData_Float_Noarg:
                            *value = (double)sam->valueFloat;
                            return true;
                    case SampleData_Int:
                    case SampleData_Int_Noarg:
                            *value = (double)sam->valueInt;
                            return true;
                    case SampleData_Double:
                    case SampleData_Double_Noarg:
                            *value = sam->valueDouble;
                            return true;
                    case SampleData_LongLong:
                    case SampleData_LongLong_Noarg:
                            /* risk of overflow here - specifically pertains to the UTC milliseconds channel */
                            pr_warning_str_msg(LOG_PFX "Data type not supported for channel: ", name);
                            return false;
                    default:
                            pr_warning_int_msg(LOG_PFX "Unknown channel sample type", sam->sampleData);
                            return false;
            }
    }
    pr_trace_str_msg(LOG_PFX "Unknown channel name: ", name);
    return false;
}

/**
 * Checks to ensure that the LoggerMessage object is pointing to a
 * usable data_sample structure.  This is needed because while the
 * LoggerMessages are deeply copied to their respective queues, the
 * data_sample structures are not (they are a part of a ring buffer).
 * So we must validate that the timestamp on the LoggerMessage matches
 * that of the data_sample object.
 * @param lm The LoggerMessage object.
 * @return True if the data_sample is usable, false otherwise.
 */
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

xQueueHandle create_logger_message_queue()
{
        return xQueueCreate(LOGGER_MESSAGE_BUFFER_SIZE, sizeof(LoggerMessage));
}

LoggerMessage create_logger_message(const enum LoggerMessageType t,
                                    const size_t ticks, struct sample *s)
{
        LoggerMessage msg;

        msg.type = t;
        msg.ticks = ticks;
        msg.sample = s;

        return msg;
}
