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

static struct auto_logger_config alc;
static GpsSample gps_sample;

CPPUNIT_TEST_SUITE_REGISTRATION( AutoLoggerTest );

void AutoLoggerTest::setUp()
{
        auto_logger_reset_config(&alc);
        auto_logger_init(&alc);
}

void AutoLoggerTest::setGpsSpeedAboveStartTrigger()
{
        gps_sample.speed = DEFAULT_START_SPEED_KPH + 1;
}

void AutoLoggerTest::setGpsSpeedBelowStartTrigger()
{
        gps_sample.speed = DEFAULT_START_SPEED_KPH - 1;
}

void AutoLoggerTest::setGpsSpeedAboveStopTrigger()
{
        gps_sample.speed = DEFAULT_STOP_SPEED_KPH + 1;
}

void AutoLoggerTest::setGpsSpeedBelowStopTrigger()
{
        gps_sample.speed = DEFAULT_STOP_SPEED_KPH - 1;
}

tiny_millis_t AutoLoggerTest::getTriggerTimeStart()
{
        return (tiny_millis_t) DEFAULT_START_TIME_SEC * 1000 + 1;
}

tiny_millis_t AutoLoggerTest::getTriggerTimeStop()
{
        return (tiny_millis_t) DEFAULT_STOP_TIME_SEC * 1000 + 1;
}

void AutoLoggerTest::shouldStartLoggingNoTrigTime()
{
        const tiny_millis_t t1 = 1;
        const tiny_millis_t t2 = t1 + getTriggerTimeStart();

        setGpsSpeedAboveStartTrigger();
        auto_logger_state.cfg->start.time = 0;
        auto_logger_state.timestamp_start = t1;

        CPPUNIT_ASSERT_EQUAL(false, should_start_logging(&gps_sample, t1));
        CPPUNIT_ASSERT_EQUAL(false, should_start_logging(&gps_sample, t2));
}

void AutoLoggerTest::shouldStartLoggingLowSpeed()
{
        const tiny_millis_t t1 = 1;
        const tiny_millis_t t2 = t1 + getTriggerTimeStart();

        setGpsSpeedBelowStartTrigger();
        auto_logger_state.timestamp_start = t1;

        CPPUNIT_ASSERT_EQUAL(false, should_start_logging(&gps_sample, t1));
        CPPUNIT_ASSERT_EQUAL(false, should_start_logging(&gps_sample, t2));
}

void AutoLoggerTest::shouldStartLoggingTrigger()
{
        const tiny_millis_t t1 = 1;
        const tiny_millis_t t2 = t1 + getTriggerTimeStart();

        setGpsSpeedAboveStartTrigger();
        auto_logger_state.timestamp_start = t1;

        CPPUNIT_ASSERT_EQUAL(false, should_start_logging(&gps_sample, t1));
        CPPUNIT_ASSERT_EQUAL(true, should_start_logging(&gps_sample, t2));
}

void AutoLoggerTest::shouldStopLoggingNoTrigTime()
{
        const tiny_millis_t t1 = 1;
        const tiny_millis_t t2 = t1 + getTriggerTimeStop();

        setGpsSpeedBelowStopTrigger();
        auto_logger_state.cfg->stop.time = 0;
        auto_logger_state.timestamp_stop = t1;

        CPPUNIT_ASSERT_EQUAL(false, should_stop_logging(&gps_sample, t1));
        CPPUNIT_ASSERT_EQUAL(false, should_stop_logging(&gps_sample, t2));
}

void AutoLoggerTest::shouldStopLoggingHighSpeed()
{
        const tiny_millis_t t1 = 1;
        const tiny_millis_t t2 = t1 + getTriggerTimeStop();

        setGpsSpeedAboveStopTrigger();
        auto_logger_state.timestamp_stop = t1;

        CPPUNIT_ASSERT_EQUAL(false, should_stop_logging(&gps_sample, t1));
        CPPUNIT_ASSERT_EQUAL(false, should_stop_logging(&gps_sample, t2));
}

void AutoLoggerTest::shouldStopLoggingTrigger()
{
        const tiny_millis_t t1 = 1;
        const tiny_millis_t t2 = t1 + getTriggerTimeStop();

        setGpsSpeedBelowStopTrigger();
        auto_logger_state.timestamp_stop = t1;

        CPPUNIT_ASSERT_EQUAL(false, should_stop_logging(&gps_sample, t1));
        CPPUNIT_ASSERT_EQUAL(true, should_stop_logging(&gps_sample, t2));
}
