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

#include "ChannelConfigTest.hh"
#include "channel_config.h"
#include <string.h>

/* Inclue the code to test here */
extern "C" {
#include "channel_config.c"
}

CPPUNIT_TEST_SUITE_REGISTRATION( ChannelConfigTest );

ChannelConfig cc;
ChannelConfig *ccp = &cc;

void ChannelConfigTest::setUp()
{
        channel_config_defaults(ccp);
}

void ChannelConfigTest::test_defaults()
{
        CPPUNIT_ASSERT_EQUAL(CHAN_CFG_STATUS_NO_LABEL,
                             validate_channel_config(ccp));

        strcpy(cc.label, "FOO");
        CPPUNIT_ASSERT_EQUAL(CHAN_CFG_STATUS_OK,
                             validate_channel_config(ccp));
}

void ChannelConfigTest::test_validate_label()
{
        CPPUNIT_ASSERT_EQUAL(CHAN_CFG_STATUS_NO_LABEL,
                             validate_channel_config_label(NULL));
        CPPUNIT_ASSERT_EQUAL(CHAN_CFG_STATUS_NO_LABEL,
                             validate_channel_config_label(""));

        char label[DEFAULT_LABEL_LENGTH + 1] = {};

        memset(label, 'A', DEFAULT_LABEL_LENGTH - 1);
        CPPUNIT_ASSERT_EQUAL(CHAN_CFG_STATUS_OK,
                             validate_channel_config_label(label));

        memset(label, 'A', DEFAULT_LABEL_LENGTH);
        CPPUNIT_ASSERT_EQUAL(CHAN_CFG_STATUS_LONG_LABEL,
                             validate_channel_config_label(label));
}

void ChannelConfigTest::test_validate_units()
{
        CPPUNIT_ASSERT_EQUAL(CHAN_CFG_STATUS_NO_UNITS,
                             validate_channel_config_units(NULL));

        char units[DEFAULT_UNITS_LENGTH + 1] = {};

        memset(units, 'A', DEFAULT_UNITS_LENGTH - 1);
        CPPUNIT_ASSERT_EQUAL(CHAN_CFG_STATUS_OK,
                             validate_channel_config_units(units));

        memset(units, 'A', DEFAULT_UNITS_LENGTH);
        CPPUNIT_ASSERT_EQUAL(CHAN_CFG_STATUS_LONG_UNITS,
                             validate_channel_config_units(units));
}

void ChannelConfigTest::test_validate()
{
        CPPUNIT_ASSERT_EQUAL(CHAN_CFG_STATUS_NULL_CHAN_CFG,
                             validate_channel_config(NULL));

        CPPUNIT_ASSERT_EQUAL(CHAN_CFG_STATUS_NO_LABEL,
                             validate_channel_config(ccp));

        memset(cc.label, 'A', DEFAULT_LABEL_LENGTH - 1);
        CPPUNIT_ASSERT_EQUAL(CHAN_CFG_STATUS_OK,
                             validate_channel_config(ccp));

        memset(cc.units, 'B', DEFAULT_UNITS_LENGTH);
        CPPUNIT_ASSERT_EQUAL(CHAN_CFG_STATUS_LONG_UNITS,
                             validate_channel_config(ccp));

        cc.units[DEFAULT_UNITS_LENGTH - 1] = '\0';
        CPPUNIT_ASSERT_EQUAL(CHAN_CFG_STATUS_OK,
                             validate_channel_config(ccp));

        cc.max = 0;
        cc.min = 1;
        CPPUNIT_ASSERT_EQUAL(CHAN_CFG_STATUS_MAX_LT_MIN,
                             validate_channel_config(ccp));

        cc.max = 1;
        CPPUNIT_ASSERT_EQUAL(CHAN_CFG_STATUS_OK,
                             validate_channel_config(ccp));

        cc.flags = 2;
        CPPUNIT_ASSERT_EQUAL(CHAN_CFG_STATUS_INVALID_FLAG,
                             validate_channel_config(ccp));
}
