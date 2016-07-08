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

#ifndef _AUTOLOGGERTEST_H_
#define _AUTOLOGGERTEST_H_

#include "dateTime.h"
#include <cppunit/extensions/HelperMacros.h>

class AutoLoggerTest : public CppUnit::TestFixture
{
        CPPUNIT_TEST_SUITE( AutoLoggerTest );
        CPPUNIT_TEST( shouldStartLoggingNoTrigTime );
        CPPUNIT_TEST( shouldStartLoggingLowSpeed );
        CPPUNIT_TEST( shouldStartLoggingTrigger );
        CPPUNIT_TEST( shouldStopLoggingNoTrigTime );
        CPPUNIT_TEST( shouldStopLoggingHighSpeed );
        CPPUNIT_TEST( shouldStopLoggingTrigger );
        CPPUNIT_TEST_SUITE_END();

private:
        void setGpsSpeedAboveStartTrigger();
        void setGpsSpeedBelowStartTrigger();
        void setGpsSpeedAboveStopTrigger();
        void setGpsSpeedBelowStopTrigger();
        tiny_millis_t getTriggerTimeStart();
        tiny_millis_t getTriggerTimeStop();

public:
        void setUp();
        void shouldStartLoggingNoTrigTime();
        void shouldStartLoggingLowSpeed();
        void shouldStartLoggingTrigger();
        void shouldStopLoggingNoTrigTime();
        void shouldStopLoggingHighSpeed();
        void shouldStopLoggingTrigger();
};

#endif /* _AUTOLOGGERTEST_H_ */
