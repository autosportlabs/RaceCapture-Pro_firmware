/**
 * Race Capture Firmware
 *
 * Copyright (C) 2014 Autosport Labs
 *
 * This file is part of the Race Capture firmware suite
 *
 * This is free software: you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details. You should have received a copy of the GNU
 * General Public License along with this code. If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors: Stieg
 */

#ifndef _DATE_TIME_TEST_H_
#define _DATE_TIME_TEST_H_

#include <cppunit/extensions/HelperMacros.h>

class DateTimeTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE( DateTimeTest );
    CPPUNIT_TEST( testLeapYear );
    CPPUNIT_TEST( testDaysInMonth );
    CPPUNIT_TEST( testIsValidDateTime );
    CPPUNIT_TEST( testGetMillisSinceEpoch );
    CPPUNIT_TEST( testMillisToMinutes );
    CPPUNIT_TEST( testMillisToSeconds );
    CPPUNIT_TEST( testTinyMillisToMinutes );
    CPPUNIT_TEST( testTinyMillisToSeconds );
    CPPUNIT_TEST( testGetDeltaInMillis );
    CPPUNIT_TEST( testUptime );
    CPPUNIT_TEST( testMillisSinceEpoch );
    CPPUNIT_TEST( testDateTimeFromEpochMillis );
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp();
    void tearDown();
    void testLeapYear();
    void testDaysInMonth();
    void testIsValidDateTime();
    void testGetMillisSinceEpoch();
    void testGetDeltaInMillis();
    void testMillisToMinutes();
    void testMillisToSeconds();
    void testTinyMillisToMinutes();
    void testTinyMillisToSeconds();
    void testUptime();
    void testMillisSinceEpoch();
    void testDateTimeFromEpochMillis();
};


#endif /* _DATE_TIME_TEST_H_ */
