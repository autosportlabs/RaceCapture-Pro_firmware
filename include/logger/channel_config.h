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

#ifndef _CHANNEL_CONFIG_H_
#define _CHANNEL_CONFIG_H_

#include "cpp_guard.h"

CPP_GUARD_BEGIN

#define DEFAULT_CHANNEL_LOGGING_PRECISION	2
#define DEFAULT_CHANNEL_MAX	1000
#define DEFAULT_CHANNEL_MIN	0
#define DEFAULT_CHANNEL_UNITS	""
#define DEFAULT_LABEL_LENGTH	12
#define DEFAULT_UNITS_LENGTH	8

/*
 * These Flags dictate special behavior within the ChannelConfig struct.
 */
#define ALWAYS_SAMPLED 1 << 0

typedef struct _ChannelConfig {
    char label[DEFAULT_LABEL_LENGTH];
    char units[DEFAULT_UNITS_LENGTH];
    float min;
    float max;
    unsigned short sampleRate;
    unsigned char precision;
    unsigned char flags;
} ChannelConfig;

enum chan_cfg_status {
        CHAN_CFG_STATUS_OK,
        CHAN_CFG_STATUS_NULL_CHAN_CFG,
        CHAN_CFG_STATUS_NO_LABEL,
        CHAN_CFG_STATUS_LONG_LABEL,
        CHAN_CFG_STATUS_NO_UNITS,
        CHAN_CFG_STATUS_LONG_UNITS,
        CHAN_CFG_STATUS_MAX_LT_MIN,
        CHAN_CFG_STATUS_INVALID_FLAG,
};

void channel_config_defaults(ChannelConfig *cc);
enum chan_cfg_status validate_channel_config_label(const char *label);
enum chan_cfg_status validate_channel_config_units(const char *units);
enum chan_cfg_status validate_channel_config(const ChannelConfig *cc);
void set_default_channel_config(ChannelConfig *cc);

CPP_GUARD_END

#endif /* _CHANNEL_CONFIG_H_ */
