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

#include "channel_config.h"
#include "loggerConfig.h"
#include <string.h>
#include <string.h>

/**
 * Creates a new default ChannelConfig.  It is not ready to use because
 * you need to populate the label, but all default values that can be
 * set have been.
 * @param cc A ChannelConfig structure to populate.
 */
void channel_config_defaults(ChannelConfig *cc)
{
        memset(cc, 0, sizeof(ChannelConfig));
        cc->min = DEFAULT_CHANNEL_MIN;
        cc->max = DEFAULT_CHANNEL_MAX;
}

/**
 * Validates that the proposed label field is valid for a ChannelConfig
 * struct.
 * @param label The label to test.
 * @return CHAN_CFG_STATUS_OK if the data is good, otherwise will return
 *         the appropriate enum chan_cfg_status value to indicate the first
 *         issue found.
 */
enum chan_cfg_status validate_channel_config_label(const char *label)
{
        if (!label || !*label)
                return CHAN_CFG_STATUS_NO_LABEL;

        if (strlen(label) >= DEFAULT_LABEL_LENGTH)
                return CHAN_CFG_STATUS_LONG_LABEL;

        return CHAN_CFG_STATUS_OK;
}

/**
 * Validates that the proposed units field is valid for a ChannelConfig
 * struct.
 * @param label The label to test.
 * @return CHAN_CFG_STATUS_OK if the data is good, otherwise will return
 *         the appropriate enum chan_cfg_status value to indicate the first
 *         issue found.

 */
enum chan_cfg_status validate_channel_config_units(const char *units)
{
        if (!units)
                return CHAN_CFG_STATUS_NO_UNITS;

        if (strlen(units) >= DEFAULT_UNITS_LENGTH)
                return CHAN_CFG_STATUS_LONG_UNITS;

        return CHAN_CFG_STATUS_OK;
}

/**
 * Validates a ChannelConfig structure.
 * @return CHAN_CFG_STATUS_OK if the data is good, otherwise will return
 *         the appropriate enum chan_cfg_status value to indicate the first
 *         issue found.
 */
enum chan_cfg_status validate_channel_config(const ChannelConfig *cc)
{
        if (!cc)
                return CHAN_CFG_STATUS_NULL_CHAN_CFG;

        enum chan_cfg_status s;
        s = validate_channel_config_label(cc->label);
        if (CHAN_CFG_STATUS_OK != s)
                return s;

        s = validate_channel_config_units(cc->units);
        if (CHAN_CFG_STATUS_OK != s)
                return s;

        if (cc->min > cc->max)
                return CHAN_CFG_STATUS_MAX_LT_MIN;

        /* Logically or all valid flags here */
        const unsigned char valid_flags = ALWAYS_SAMPLED;
        if (cc->flags & ~valid_flags)
                return CHAN_CFG_STATUS_INVALID_FLAG;

        return CHAN_CFG_STATUS_OK;
}

void set_default_channel_config(ChannelConfig *cc)
{
        memset(cc, 0, sizeof(ChannelConfig));
        cc->sampleRate = SAMPLE_DISABLED;
}
