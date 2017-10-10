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

#include "AutoLoggerTest.hh"
#include "auto_logger.h"
#include <cppunit/extensions/HelperMacros.h>

/* Inclue the code to test here */
extern "C" {
#include "auto_logger.c"
}

static float current_value;
static struct auto_logger_config alc;

CPPUNIT_TEST_SUITE_REGISTRATION( AutoLoggerTest );

void AutoLoggerTest::setUp()
{
        auto_logger_reset_config(&alc);
        auto_logger_init(&alc);
}

void AutoLoggerTest::setGpsSpeedAboveStartTrigger()
{
        current_value = DEFAULT_AUTO_LOGGER_START_SPEED + 1;
}

void AutoLoggerTest::setGpsSpeedBelowStartTrigger()
{
        current_value = DEFAULT_AUTO_LOGGER_START_SPEED - 1;
}

void AutoLoggerTest::setGpsSpeedAboveStopTrigger()
{
        current_value = DEFAULT_AUTO_LOGGER_STOP_SPEED + 1;
}

void AutoLoggerTest::setGpsSpeedBelowStopTrigger()
{
        current_value = DEFAULT_AUTO_LOGGER_STOP_SPEED - 1;
}

tiny_millis_t AutoLoggerTest::getTriggerTimeStart()
{
        return (tiny_millis_t) DEFAULT_AUTO_LOGGER_START_TIME_SEC * 1000 + 1;
}

tiny_millis_t AutoLoggerTest::getTriggerTimeStop()
{
        return (tiny_millis_t) DEFAULT_AUTO_LOGGER_STOP_TIME_SEC * 1000 + 1;
}

void AutoLoggerTest::shouldStartLoggingNoTrigTime()
{
        const tiny_millis_t t1 = 1;
        const tiny_millis_t t2 = t1 + getTriggerTimeStart();

        setGpsSpeedAboveStartTrigger();
        auto_logger_state.cfg->start.time = 0;
        auto_logger_state.control_state.timestamp_start = t1;

        CPPUNIT_ASSERT_EQUAL(false, auto_control_should_start(current_value, t1, &auto_logger_state.cfg->start, &auto_logger_state.control_state));
        CPPUNIT_ASSERT_EQUAL(false, auto_control_should_start(current_value, t2, &auto_logger_state.cfg->start, &auto_logger_state.control_state));
}

void AutoLoggerTest::shouldStartLoggingLowSpeed()
{
        const tiny_millis_t t1 = 1;
        const tiny_millis_t t2 = t1 + getTriggerTimeStart();

        setGpsSpeedBelowStartTrigger();
        auto_logger_state.control_state.timestamp_start = t1;

        CPPUNIT_ASSERT_EQUAL(false, auto_control_should_start(current_value, t1, &auto_logger_state.cfg->start, &auto_logger_state.control_state));
        CPPUNIT_ASSERT_EQUAL(false, auto_control_should_start(current_value, t2, &auto_logger_state.cfg->start, &auto_logger_state.control_state));
}

void AutoLoggerTest::shouldStartLoggingTrigger()
{
        const tiny_millis_t t1 = 1;
        const tiny_millis_t t2 = t1 + getTriggerTimeStart();

        setGpsSpeedAboveStartTrigger();
        auto_logger_state.control_state.timestamp_start = t1;

        CPPUNIT_ASSERT_EQUAL(false, auto_control_should_start(current_value, t1, &auto_logger_state.cfg->start, &auto_logger_state.control_state));
        CPPUNIT_ASSERT_EQUAL(true, auto_control_should_start(current_value, t2, &auto_logger_state.cfg->start, &auto_logger_state.control_state));
}

void AutoLoggerTest::shouldStopLoggingNoTrigTime()
{
        const tiny_millis_t t1 = 1;
        const tiny_millis_t t2 = t1 + getTriggerTimeStop();

        setGpsSpeedBelowStopTrigger();
        auto_logger_state.cfg->stop.time = 0;
        auto_logger_state.control_state.timestamp_stop = t1;

        CPPUNIT_ASSERT_EQUAL(false, auto_control_should_stop(current_value, t1, &auto_logger_state.cfg->stop, &auto_logger_state.control_state));
        CPPUNIT_ASSERT_EQUAL(false, auto_control_should_stop(current_value, t2, &auto_logger_state.cfg->stop, &auto_logger_state.control_state));
}

void AutoLoggerTest::shouldStopLoggingHighSpeed()
{
        const tiny_millis_t t1 = 1;
        const tiny_millis_t t2 = t1 + getTriggerTimeStop();

        setGpsSpeedAboveStopTrigger();
        auto_logger_state.control_state.timestamp_stop = t1;

        CPPUNIT_ASSERT_EQUAL(false, auto_control_should_stop(current_value, t1, &auto_logger_state.cfg->stop, &auto_logger_state.control_state));
        CPPUNIT_ASSERT_EQUAL(false, auto_control_should_stop(current_value, t2, &auto_logger_state.cfg->stop, &auto_logger_state.control_state));
}

void AutoLoggerTest::shouldStopLoggingTrigger()
{
        const tiny_millis_t t1 = 1;
        const tiny_millis_t t2 = t1 + getTriggerTimeStop();

        setGpsSpeedBelowStopTrigger();
        auto_logger_state.control_state.timestamp_stop = t1;

        CPPUNIT_ASSERT_EQUAL(false, auto_control_should_stop(current_value, t1, &auto_logger_state.cfg->stop, &auto_logger_state.control_state));
        CPPUNIT_ASSERT_EQUAL(true, auto_control_should_stop(current_value, t2, &auto_logger_state.cfg->stop, &auto_logger_state.control_state));
}
