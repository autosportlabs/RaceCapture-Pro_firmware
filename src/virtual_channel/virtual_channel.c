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

#include "capabilities.h"
#include "channel_config.h"
#include "loggerTaskEx.h"
#include "macros.h"
#include "mem_mang.h"
#include <string.h>
#include "printk.h"
#include "virtual_channel.h"
#include "semphr.h"

static size_t g_virtualChannelCount = 0;
static VirtualChannel g_virtualChannels[MAX_VIRTUAL_CHANNELS];
static xSemaphoreHandle vchan_mutex = NULL;

void init_virtual_channels(void)
{
        vchan_mutex = xSemaphoreCreateMutex();
}

VirtualChannel* get_virtual_channel(size_t id)
{
        if (id < g_virtualChannelCount)
                return g_virtualChannels + id;

        return NULL;
}

int find_virtual_channel(const char * channel_name)
{
        for (size_t i = 0; i < g_virtualChannelCount; i++) {
                if (STR_EQ(channel_name, g_virtualChannels[i].config.label))
                        return i;
        }

        return INVALID_VIRTUAL_CHANNEL;
}

int create_virtual_channel(const ChannelConfig chCfg)
{
        xSemaphoreTake(vchan_mutex, portMAX_DELAY);
        /* If the channel exists, return it and be done */
        const int id = find_virtual_channel(chCfg.label);
        if (id != INVALID_VIRTUAL_CHANNEL) {
                xSemaphoreGive(vchan_mutex);
                return id;
        }

        /*
         * Here we actually try to create a new channel.  But only if we
         * have the room for it.
         */
        int new_channel_id = INVALID_VIRTUAL_CHANNEL;

        if (g_virtualChannelCount < MAX_VIRTUAL_CHANNELS) {
                VirtualChannel * channel = g_virtualChannels + g_virtualChannelCount;
                channel->config = chCfg;
                channel->currentValue = 0;
                configChanged();
                new_channel_id = g_virtualChannelCount++;
        } else {
                pr_error_int_msg("[vchan] Limit reached: ", g_virtualChannelCount);
        }
        xSemaphoreGive(vchan_mutex);
        return new_channel_id;
}

void set_virtual_channel_value(size_t id, float value)
{
        if (id >= g_virtualChannelCount || id < 0 ) {
                pr_error_int_msg("[vchan] set_channel out of range ", id);
                return;
        }
        g_virtualChannels[id].currentValue = value;
}

float get_virtual_channel_value(int id)
{
        if (id >= g_virtualChannelCount || id < 0 ) {
                pr_error_int_msg("[vchan] get_channel out of range ", id);
                return 0.0f;
        }

        return g_virtualChannels[id].currentValue;
}

size_t get_virtual_channel_count(void)
{
        return g_virtualChannelCount;
}

void reset_virtual_channels(void)
{
        xSemaphoreTake(vchan_mutex, portMAX_DELAY);
        g_virtualChannelCount = 0;
        xSemaphoreGive(vchan_mutex);
        configChanged();
}

int get_virtual_channel_high_sample_rate(void)
{
        int sr = 0;

        for (size_t i = 0; i < g_virtualChannelCount; ++i) {
                const int vc_sr = g_virtualChannels[i].config.sampleRate;
                sr = getHigherSampleRate(sr, vc_sr);
        }

        return sr;
}

