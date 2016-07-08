/**
 * Race Capture Pro Firmware
 *
 * Copyright (C) 2014 Autosport Labs
 *
 * This file is part of the Race Capture Pro fimrware suite
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

#include "FreeRTOS.h"
#include "capabilities.h"
#include "dateTime.h"
#include "date_time_test.h"
#include "gps.h"
#include "gps.testing.h"
#include "mock_serial.h"
#include "task.h"
#include "task_testing.h"
#include "serial.h"

#include <cppunit/extensions/HelperMacros.h>

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( DateTimeTest );

void DateTimeTest::setUp() {
        reset_ticks();
        setupMockSerial();
        GPS_init(10, getMockSerial());
}

void DateTimeTest::tearDown() {}

void DateTimeTest::testIsValidDateTime()
{
   const DateTime invalid_dt = {};
   CPPUNIT_ASSERT(!isValidDateTime(invalid_dt));

   // Jan 1, 2000.  Happy New Year!
   const DateTime new_millenium = {0, 0, 0, 0, 1, 1, 2000};
   CPPUNIT_ASSERT(isValidDateTime(new_millenium));
}

void DateTimeTest::testLeapYear()
{
   CPPUNIT_ASSERT_EQUAL(true, isLeapYear(1968));
   CPPUNIT_ASSERT_EQUAL(false, isLeapYear(1969));
   CPPUNIT_ASSERT_EQUAL(false, isLeapYear(1970));
   CPPUNIT_ASSERT_EQUAL(false, isLeapYear(1971));
   CPPUNIT_ASSERT_EQUAL(true, isLeapYear(1972));
   CPPUNIT_ASSERT_EQUAL(true, isLeapYear(2000));
   CPPUNIT_ASSERT_EQUAL(false, isLeapYear(2100));
}


void DateTimeTest::testDaysInMonth()
{
   // Non Leap Year
   CPPUNIT_ASSERT_EQUAL(31u, getDaysInMonth(1u, false));
   CPPUNIT_ASSERT_EQUAL(28u, getDaysInMonth(2u, false));
   CPPUNIT_ASSERT_EQUAL(31u, getDaysInMonth(3u, false));
   CPPUNIT_ASSERT_EQUAL(30u, getDaysInMonth(4u, false));
   CPPUNIT_ASSERT_EQUAL(31u, getDaysInMonth(5u, false));
   CPPUNIT_ASSERT_EQUAL(30u, getDaysInMonth(6u, false));
   CPPUNIT_ASSERT_EQUAL(31u, getDaysInMonth(7u, false));
   CPPUNIT_ASSERT_EQUAL(31u, getDaysInMonth(8u, false));
   CPPUNIT_ASSERT_EQUAL(30u, getDaysInMonth(9u, false));
   CPPUNIT_ASSERT_EQUAL(31u, getDaysInMonth(10u, false));
   CPPUNIT_ASSERT_EQUAL(30u, getDaysInMonth(11u, false));
   CPPUNIT_ASSERT_EQUAL(31u, getDaysInMonth(12u, false));

   // Leap Year
   CPPUNIT_ASSERT_EQUAL(31u, getDaysInMonth(1u, true));
   CPPUNIT_ASSERT_EQUAL(29u, getDaysInMonth(2u, true));
   CPPUNIT_ASSERT_EQUAL(31u, getDaysInMonth(3u, true));
   CPPUNIT_ASSERT_EQUAL(30u, getDaysInMonth(4u, true));
   CPPUNIT_ASSERT_EQUAL(31u, getDaysInMonth(5u, true));
   CPPUNIT_ASSERT_EQUAL(30u, getDaysInMonth(6u, true));
   CPPUNIT_ASSERT_EQUAL(31u, getDaysInMonth(7u, true));
   CPPUNIT_ASSERT_EQUAL(31u, getDaysInMonth(8u, true));
   CPPUNIT_ASSERT_EQUAL(30u, getDaysInMonth(9u, true));
   CPPUNIT_ASSERT_EQUAL(31u, getDaysInMonth(10u, true));
   CPPUNIT_ASSERT_EQUAL(30u, getDaysInMonth(11u, true));
   CPPUNIT_ASSERT_EQUAL(31u, getDaysInMonth(12u, true));
}

void DateTimeTest::testGetMillisSinceEpoch() {
   // Invalid date.  Should be 0.
   const DateTime invalid = {};
   CPPUNIT_ASSERT_EQUAL( (millis_t) 0,
                        getMillisecondsSinceUnixEpoch(invalid));


   // 1970, Jan 1 @ 00:00:00.000 == 0
   const DateTime epoch = {0, 0, 0, 0, 1, 1, 1970};
   CPPUNIT_ASSERT_EQUAL( (millis_t) 0,
                        getMillisecondsSinceUnixEpoch(epoch));

   // 1970, Jan 1 @ 00:16:40.000 == 1000000
   const DateTime d1000000 = {0, 40, 16, 0, 1, 1, 1970};
   CPPUNIT_ASSERT_EQUAL( (millis_t) 1000000ll,
                        getMillisecondsSinceUnixEpoch(d1000000));

   // 1970, Jan 1 @ 12:00:00.000 == 43200000
   const DateTime d43200000 = {0, 0, 0, 12, 1, 1, 1970};
   CPPUNIT_ASSERT_EQUAL( (millis_t) 43200000ll,
                        getMillisecondsSinceUnixEpoch(d43200000));

   // 1970, Jan 2 @ 12:00:00.000 == 129600000
   const DateTime d129600000 = {0, 0, 0, 12, 2, 1, 1970};
   CPPUNIT_ASSERT_EQUAL( (millis_t) 129600000ll,
                        getMillisecondsSinceUnixEpoch(d129600000));

   // 1970, Jan 31 @ 12:00:00.000 == 2635200000
   const DateTime d2635200000 = {0, 0, 0, 12, 31, 1, 1970};
   CPPUNIT_ASSERT_EQUAL( (millis_t) 2635200000ll,
                        getMillisecondsSinceUnixEpoch(d2635200000));

   // 1970, Feb 1 @ 12:00:00.000 == 2721600000
   const DateTime d2721600000 = {0, 0, 0, 12, 1, 2, 1970};
   CPPUNIT_ASSERT_EQUAL( (millis_t) 2721600000ll,
                        getMillisecondsSinceUnixEpoch(d2721600000));

   // 1970, Feb 28 @ 12:00:00.000 == 5054400000
   const DateTime d5054400000 = {0, 0, 0, 12, 28, 2, 1970};
   CPPUNIT_ASSERT_EQUAL( (millis_t) 5054400000ll,
                        getMillisecondsSinceUnixEpoch(d5054400000));

   // 1970, Mar 1 @ 12:00:00.000 == 5140800000
   const DateTime d5140800000 = {0, 0, 0, 12, 1, 3, 1970};
   CPPUNIT_ASSERT_EQUAL( (millis_t) 5140800000ll,
                        getMillisecondsSinceUnixEpoch(d5140800000));

   // 1970, Apr 1 @ 12:00:00.000 == 7819200000
   const DateTime d7819200000 = {0, 0, 0, 12, 1, 4, 1970};
   CPPUNIT_ASSERT_EQUAL( (millis_t) 7819200000ll,
                        getMillisecondsSinceUnixEpoch(d7819200000));

   // 1971, Jan 1 @ 12:00:00.000 == 31579200000
   const DateTime d31579200000 = {0, 0, 0, 12, 1, 1, 1971};
   CPPUNIT_ASSERT_EQUAL( (millis_t) 31579200000ll,
                        getMillisecondsSinceUnixEpoch(d31579200000));

   // 1984, Mar 17 @ 10:08:00.000 == 448366080000.  My birth timestamp.
   const DateTime d448366080000 = {0, 0, 8, 10, 17, 3, 1984};
   CPPUNIT_ASSERT_EQUAL( (millis_t) 448366080000ll,
                        getMillisecondsSinceUnixEpoch(d448366080000));

   // 2013, January 1 @ 00:00:00.000 == 1356998400000 milliseconds since epoch.
   const DateTime jan_1_2013 = {0, 0, 0, 0, 1, 1, 2013};
   CPPUNIT_ASSERT_EQUAL( (millis_t) 1356998400000ll,
                        getMillisecondsSinceUnixEpoch(jan_1_2013));

}

void DateTimeTest::testGetDeltaInMillis() {
  const DateTime epoch = {0, 0, 0, 0, 1, 1, 1970};
  const DateTime d1000000 = {0, 40, 16, 0, 1, 1, 1970};
  CPPUNIT_ASSERT_EQUAL( (millis_t) 0ll, getTimeDeltaInMillis(epoch, epoch));
  CPPUNIT_ASSERT_EQUAL( (millis_t) 1000000ll, getTimeDeltaInMillis(d1000000, epoch));
  CPPUNIT_ASSERT_EQUAL( (millis_t) -1000000ll, getTimeDeltaInMillis(epoch, d1000000));
}

void DateTimeTest::testMillisToMinutes() {
  CPPUNIT_ASSERT_EQUAL(0.1f, millisToMinutes(6000));
  CPPUNIT_ASSERT_EQUAL(1.0f, millisToMinutes(60000));
  CPPUNIT_ASSERT_EQUAL(10.0001f, millisToMinutes(600006));
  CPPUNIT_ASSERT_EQUAL(10.000016667f, millisToMinutes(600001));
  CPPUNIT_ASSERT_EQUAL(100.000016667f, millisToMinutes(6000001));
}

void DateTimeTest::testMillisToSeconds() {
  CPPUNIT_ASSERT_EQUAL(0.001f, millisToSeconds(1));
  CPPUNIT_ASSERT_EQUAL(0.01f, millisToSeconds(10));
  CPPUNIT_ASSERT_EQUAL(0.1f, millisToSeconds(100));
  CPPUNIT_ASSERT_EQUAL(1.0f, millisToSeconds(1000));
  CPPUNIT_ASSERT_EQUAL(10.0f, millisToSeconds(10000));
  CPPUNIT_ASSERT_EQUAL(10.001f, millisToSeconds(10001));
  CPPUNIT_ASSERT_EQUAL(100.001f, millisToSeconds(100001));

  // Test something really large.  Doubles or longs would be nice :).
  CPPUNIT_ASSERT_EQUAL(31557600.001f, millisToSeconds(31557600001));
}

void DateTimeTest::testTinyMillisToMinutes() {
  CPPUNIT_ASSERT_EQUAL(0.1f, tinyMillisToMinutes(6000));
  CPPUNIT_ASSERT_EQUAL(1.0f, tinyMillisToMinutes(60000));
  CPPUNIT_ASSERT_EQUAL(10.0001f, tinyMillisToMinutes(600006));
  CPPUNIT_ASSERT_EQUAL(10.000016667f, tinyMillisToMinutes(600001));
  CPPUNIT_ASSERT_EQUAL(100.000016667f, tinyMillisToMinutes(6000001));
}

void DateTimeTest::testTinyMillisToSeconds() {
  CPPUNIT_ASSERT_EQUAL(0.001f, tinyMillisToSeconds(1));
  CPPUNIT_ASSERT_EQUAL(0.01f, tinyMillisToSeconds(10));
  CPPUNIT_ASSERT_EQUAL(0.1f, tinyMillisToSeconds(100));
  CPPUNIT_ASSERT_EQUAL(1.0f, tinyMillisToSeconds(1000));
  CPPUNIT_ASSERT_EQUAL(10.0f, tinyMillisToSeconds(10000));
  CPPUNIT_ASSERT_EQUAL(10.001f, tinyMillisToSeconds(10001));
  CPPUNIT_ASSERT_EQUAL(100.001f, tinyMillisToSeconds(100001));
}

void DateTimeTest::testUptime() {
  CPPUNIT_ASSERT_EQUAL(0, (int) xTaskGetTickCount());
  CPPUNIT_ASSERT_EQUAL(0 * MS_PER_TICK, (int) getUptime());

  increment_tick();
  CPPUNIT_ASSERT_EQUAL(1, (int) xTaskGetTickCount());
  CPPUNIT_ASSERT_EQUAL(1 * MS_PER_TICK, (int) getUptime());

  increment_tick();
  CPPUNIT_ASSERT_EQUAL(2, (int) xTaskGetTickCount());
  CPPUNIT_ASSERT_EQUAL(2 * MS_PER_TICK, (int) getUptime());
}

void DateTimeTest::testMillisSinceEpoch() {
  CPPUNIT_ASSERT_EQUAL(0, (int) xTaskGetTickCount());
  CPPUNIT_ASSERT_EQUAL( (millis_t) 0ll, getMillisSinceEpoch());

  increment_tick();
  CPPUNIT_ASSERT_EQUAL(1, (int) xTaskGetTickCount());
  CPPUNIT_ASSERT_EQUAL( (millis_t) 0ll, getMillisSinceEpoch());

  increment_tick();
  const DateTime d448366080000 = {0, 0, 8, 10, 17, 3, 1984};

  GpsSample sample;
  sample.quality = GPS_QUALITY_3D;
  sample.time = getMillisecondsSinceUnixEpoch(d448366080000);
  GPS_sample_update(&sample);

  CPPUNIT_ASSERT_EQUAL(2, (int) xTaskGetTickCount());
  CPPUNIT_ASSERT_EQUAL( (millis_t) 448366080000ll + 0 * MS_PER_TICK, getMillisSinceEpoch());

  increment_tick();
  CPPUNIT_ASSERT_EQUAL(3, (int) xTaskGetTickCount());
  CPPUNIT_ASSERT_EQUAL( (millis_t) 448366080000ll + 1 * MS_PER_TICK, getMillisSinceEpoch());
}

void DateTimeTest::testDateTimeFromEpochMillis(){
    DateTime dt;

    getDateTimeFromEpochMillis(&dt, -1000);
    CPPUNIT_ASSERT_EQUAL(0, (int)dt.millisecond);
    CPPUNIT_ASSERT_EQUAL(59, (int)dt.second);
    CPPUNIT_ASSERT_EQUAL(59, (int)dt.minute);
    CPPUNIT_ASSERT_EQUAL(23, (int)dt.hour);
    CPPUNIT_ASSERT_EQUAL(31, (int)dt.day);
    CPPUNIT_ASSERT_EQUAL(12, (int)dt.month);
    CPPUNIT_ASSERT_EQUAL(1969, (int)dt.year);

    getDateTimeFromEpochMillis(&dt, 0);
    CPPUNIT_ASSERT_EQUAL(0, (int)dt.millisecond);
    CPPUNIT_ASSERT_EQUAL(0, (int)dt.second);
    CPPUNIT_ASSERT_EQUAL(0, (int)dt.minute);
    CPPUNIT_ASSERT_EQUAL(0, (int)dt.hour);
    CPPUNIT_ASSERT_EQUAL(1, (int)dt.day);
    CPPUNIT_ASSERT_EQUAL(1, (int)dt.month);
    CPPUNIT_ASSERT_EQUAL(1970, (int)dt.year);

    getDateTimeFromEpochMillis(&dt, 1000);
    CPPUNIT_ASSERT_EQUAL(0, (int)dt.millisecond);
    CPPUNIT_ASSERT_EQUAL(1, (int)dt.second);
    CPPUNIT_ASSERT_EQUAL(0, (int)dt.minute);
    CPPUNIT_ASSERT_EQUAL(0, (int)dt.hour);
    CPPUNIT_ASSERT_EQUAL(1, (int)dt.day);
    CPPUNIT_ASSERT_EQUAL(1, (int)dt.month);
    CPPUNIT_ASSERT_EQUAL(1970, (int)dt.year);

    getDateTimeFromEpochMillis(&dt, 946684799000); //party time
    CPPUNIT_ASSERT_EQUAL(0, (int)dt.millisecond);
    CPPUNIT_ASSERT_EQUAL(59, (int)dt.second);
    CPPUNIT_ASSERT_EQUAL(59, (int)dt.minute);
    CPPUNIT_ASSERT_EQUAL(23, (int)dt.hour);
    CPPUNIT_ASSERT_EQUAL(31, (int)dt.day);
    CPPUNIT_ASSERT_EQUAL(12, (int)dt.month);
    CPPUNIT_ASSERT_EQUAL(1999, (int)dt.year);

    getDateTimeFromEpochMillis(&dt, 946684799000 + 999);
    CPPUNIT_ASSERT_EQUAL(999, (int)dt.millisecond);
    CPPUNIT_ASSERT_EQUAL(59, (int)dt.second);
    CPPUNIT_ASSERT_EQUAL(59, (int)dt.minute);
    CPPUNIT_ASSERT_EQUAL(23, (int)dt.hour);
    CPPUNIT_ASSERT_EQUAL(31, (int)dt.day);
    CPPUNIT_ASSERT_EQUAL(12, (int)dt.month);
    CPPUNIT_ASSERT_EQUAL(1999, (int)dt.year);

    getDateTimeFromEpochMillis(&dt, 946684799000 + 1000);
    CPPUNIT_ASSERT_EQUAL(0, (int)dt.millisecond);
    CPPUNIT_ASSERT_EQUAL(0, (int)dt.second);
    CPPUNIT_ASSERT_EQUAL(0, (int)dt.minute);
    CPPUNIT_ASSERT_EQUAL(0, (int)dt.hour);
    CPPUNIT_ASSERT_EQUAL(1, (int)dt.day);
    CPPUNIT_ASSERT_EQUAL(1, (int)dt.month);
    CPPUNIT_ASSERT_EQUAL(2000, (int)dt.year);

    getDateTimeFromEpochMillis(&dt, 1426905472000);
    CPPUNIT_ASSERT_EQUAL(0, (int)dt.millisecond);
    CPPUNIT_ASSERT_EQUAL(52, (int)dt.second);
    CPPUNIT_ASSERT_EQUAL(37, (int)dt.minute);
    CPPUNIT_ASSERT_EQUAL(2, (int)dt.hour);
    CPPUNIT_ASSERT_EQUAL(21, (int)dt.day);
    CPPUNIT_ASSERT_EQUAL(3, (int)dt.month);
    CPPUNIT_ASSERT_EQUAL(2015, (int)dt.year);
}
