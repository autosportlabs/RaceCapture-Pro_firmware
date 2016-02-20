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


#include "virtual_channel.h"
#include "mem_mang.h"
#include "mod_string.h"
#include "printk.h"
#include "loggerTaskEx.h"
#include "capabilities.h"

static size_t g_virtualChannelCount = 0;
static VirtualChannel g_virtualChannels[MAX_VIRTUAL_CHANNELS];

VirtualChannel * get_virtual_channel(size_t id)
{
    if (id < g_virtualChannelCount)
        return g_virtualChannels + id;

    return NULL;
}

int find_virtual_channel(const char * channel_name)
{
    for (size_t i = 0; i < g_virtualChannelCount; i++) {
        if (strcmp(channel_name, g_virtualChannels[i].config.label) == 0) return i;
    }
    return INVALID_VIRTUAL_CHANNEL;
}

int create_virtual_channel(const ChannelConfig chCfg)
{
        /* If the channel exists, return it and be done */
        if (find_virtual_channel(chCfg.label) != INVALID_VIRTUAL_CHANNEL)
                return id;

        /*
         * Here we actually try to create a new channel.  But only if we
         * have the room for it.
         */
        if (g_virtualChannelCount >= MAX_VIRTUAL_CHANNELS) {
                pr_error_int_msg("[vchan] Limit reached: ",
                                 g_virtualChannelCount);
                return INVALID_VIRTUAL_CHANNEL;
        }


        VirtualChannel * channel = g_virtualChannels + g_virtualChannelCount;
        channel->config = chCfg;
        channel->currentValue = 0;
        configChanged();

        return g_virtualChannelCount++;
}

void set_virtual_channel_value(size_t id, float value)
{
    if (id < g_virtualChannelCount)	g_virtualChannels[id].currentValue = value;
}

float get_virtual_channel_value(int id)
{
    if (((size_t) id) >= g_virtualChannelCount) return 0.0;
    return g_virtualChannels[id].currentValue;
}

size_t get_virtual_channel_count(void)
{
    return g_virtualChannelCount;
}

void reset_virtual_channels(void)
{
    g_virtualChannelCount = 0;
}
