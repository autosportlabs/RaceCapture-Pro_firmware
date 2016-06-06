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
#include "dateTime.h"
#include "FreeRTOS.h"
#include "task.h"

#include <stdbool.h>

bool isLeapYear(const int year)
{
    /*
     * Credit to http://www.dispersiondesign.com/articles/time/determining_leap_years
     */
    return (year % 4) || ((year % 100 == 0) && (year % 400)) ? false : true;
}

/**
 * Returns the days in the month.  1 = Jan.
 */
unsigned int getDaysInMonth(const int month, bool leapYear)
{
    /*
     * Credit to http://www.dispersiondesign.com/articles/time/number_of_days_in_a_month
     * with a minor modification
     */

    return 31 - ((month == 2) ? (leapYear ? 2 : 3) : ((month - 1) % 7 % 2));
}

unsigned int getDaysInYear(const bool leapYear)
{
    return leapYear ? 366 : 365;
}

unsigned int getDayCountUpToMonthWithYear(int month, const int year)
{
    const bool ly = isLeapYear(year);
    unsigned int days = 0;

    while (--month > 0)
        days += getDaysInMonth(month, ly);

    return days;
}

unsigned int getDaysCountUpToMonthWithPartialYear(const int month,
        const int year)
{
    return getDayCountUpToMonthWithYear(month, year);
}

unsigned int getDayCountUpToYearSinceYear(int year, const int beforeYear)
{
    unsigned int days = 0;

    while (--year >= beforeYear)
        days += getDaysInYear(isLeapYear(year));

    return days;
}

unsigned int getDayCountUpToYearSinceUnixEpoch(const int year)
{
    return getDayCountUpToYearSinceYear(year, 1970);
}

millis_t getMillisecondsSinceUnixEpoch(DateTime dt)
{
    if (!isValidDateTime(dt))
        return 0ll;

    millis_t seconds = 0;

    // Get everything as seconds first.  No changes since 0 based.
    seconds += dt.second;
    seconds += dt.minute * SECONDS_PER_MINUTE;
    seconds += dt.hour * SECONDS_PER_HOUR;

    // Subtract 1 from day since they start at 1 instead of 0.
    seconds += (dt.day - 1) * SECONDS_PER_DAY;

    seconds += getDayCountUpToMonthWithYear(dt.month, dt.year) * SECONDS_PER_DAY;
    seconds += getDayCountUpToYearSinceUnixEpoch(dt.year) * SECONDS_PER_DAY;

    // And then convert seconds to millis and add in the remaining millis
    return seconds * MILLIS_PER_SECOND + dt.millisecond;
}

millis_t getTimeDeltaInMillis(DateTime a, DateTime b)
{
    if (!isValidDateTime(a) || !isValidDateTime(b))
        return 0;

    // HACK: I'm sure there is a better way to do this.  This way just works for me.
    return getMillisecondsSinceUnixEpoch(a) - getMillisecondsSinceUnixEpoch(b);
}

tiny_millis_t getTimeDeltaInTinyMillis(DateTime a, DateTime b)
{
    return (tiny_millis_t) getTimeDeltaInMillis(a, b);
}
static bool inRange(const int val, const int min, const int max)
{
    return min <= val && val <= max;
}

bool isValidDateTime(const DateTime dt)
{
    return inRange(dt.millisecond, 0, 999) &&
           inRange(dt.second, 0, 59) &&
           inRange(dt.minute, 0, 59) &&
           inRange(dt.hour, 0, 23) &&
           inRange(dt.day, 1, 31) &&
           inRange(dt.month, 1, 12) &&
           inRange(dt.year, 1900, 2099);
}

float millisToMinutes(const millis_t millis)
{
    return ((float) (millis / MILLIS_PER_MINUTE)) +
           (((float) (millis % MILLIS_PER_MINUTE)) / MILLIS_PER_MINUTE);
}

float millisToSeconds(const millis_t millis)
{
    return ((float) (millis / MILLIS_PER_SECOND)) +
           (((float) (millis % MILLIS_PER_SECOND)) / MILLIS_PER_SECOND);
}

float tinyMillisToMinutes(const tiny_millis_t millis)
{
    return ((float) (millis / MILLIS_PER_MINUTE)) +
           (((float) (millis % MILLIS_PER_MINUTE)) / MILLIS_PER_MINUTE);
}

float tinyMillisToSeconds(const tiny_millis_t millis)
{
    return ((float) (millis / MILLIS_PER_SECOND)) +
           (((float) (millis % MILLIS_PER_SECOND)) / MILLIS_PER_SECOND);
}

tiny_millis_t getUptime()
{
    return (tiny_millis_t) (MS_PER_TICK * xTaskGetTickCount());
}

int getUptimeAsInt()
{
    return (int) getUptime();
}

/**
 * Credit to:
 *  http://stackoverflow.com/questions/11188621/how-can-i-convert-seconds-since-the-epoch-to-hours-minutes-seconds-in-java
 */
void getDateTimeFromEpochMillis(DateTime *dateTime, millis_t millis)
{

    uint64_t secondsSinceEpoch = millis / 1000;

    uint64_t sec;
    uint32_t quadricentennials, centennials, quadrennials, annuals/*1-ennial?*/;
    uint32_t year, leap;
    uint32_t yday, hour, min;
    uint32_t month, mday;
    static const uint32_t daysSinceJan1st[2][13]= {
        {0,31,59,90,120,151,181,212,243,273,304,334,365}, // 365 days, non-leap
        {0,31,60,91,121,152,182,213,244,274,305,335,366}  // 366 days, leap
    };
    /*
      400 years:

      1st hundred, starting immediately after a leap year that's a multiple of 400:
      n n n l  \
      n n n l   } 24 times
      ...      /
      n n n l /
      n n n n

      2nd hundred:
      n n n l  \
      n n n l   } 24 times
      ...      /
      n n n l /
      n n n n

      3rd hundred:
      n n n l  \
      n n n l   } 24 times
      ...      /
      n n n l /
      n n n n

      4th hundred:
      n n n l  \
      n n n l   } 24 times
      ...      /
      n n n l /
      n n n L <- 97'th leap year every 400 years
    */

    // Re-bias from 1970 to 1601:
    // 1970 - 1601 = 369 = 3*100 + 17*4 + 1 years (incl. 89 leap days) =
    // (3*100*(365+24/100) + 17*4*(365+1/4) + 1*365)*24*3600 seconds
    sec = secondsSinceEpoch + 11644473600;

    // Remove multiples of 400 years (incl. 97 leap days)
    quadricentennials = (uint32_t)(sec / 12622780800ULL); // 400*365.2425*24*3600
    sec %= 12622780800ULL;

    // Remove multiples of 100 years (incl. 24 leap days), can't be more than 3
    // (because multiples of 4*100=400 years (incl. leap days) have been removed)
    centennials = (uint32_t)(sec / 3155673600ULL); // 100*(365+24/100)*24*3600
    if (centennials > 3) {
        centennials = 3;
    }
    sec -= centennials * 3155673600ULL;

    // Remove multiples of 4 years (incl. 1 leap day), can't be more than 24
    // (because multiples of 25*4=100 years (incl. leap days) have been removed)
    quadrennials = (uint32_t)(sec / 126230400); // 4*(365+1/4)*24*3600
    if (quadrennials > 24) {
        quadrennials = 24;
    }
    sec -= quadrennials * 126230400ULL;

    // Remove multiples of years (incl. 0 leap days), can't be more than 3
    // (because multiples of 4 years (incl. leap days) have been removed)
    annuals = (uint32_t)(sec / 31536000); // 365*24*3600
    if (annuals > 3) {
        annuals = 3;
    }
    sec -= annuals * 31536000ULL;

    // Calculate the year and find out if it's leap
    year = 1601 + quadricentennials * 400 + centennials * 100 + quadrennials * 4 + annuals;

    leap = isLeapYear(year);

    // Calculate the day of the year and the time
    yday = sec / 86400;
    sec %= 86400;
    hour = sec / 3600;
    sec %= 3600;
    min = sec / 60;
    sec %= 60;

    // Calculate the month
    for (mday = month = 1; month < 13; month++) {
        if (yday < daysSinceJan1st[leap][month]) {
            mday += yday - daysSinceJan1st[leap][month - 1];
            break;
        }
    }

    dateTime->millisecond = millis % 1000;
    dateTime->second = sec;
    dateTime->minute = min;
    dateTime->hour = hour;
    dateTime->month = month;
    dateTime->day = mday;
    dateTime->year = year;
}

/**
 * Gets the uptime from now as given by the first argument.
 * @param to_add The amount of time to add to the current time.  Value is
 * in milliseconds.
 */
tiny_millis_t date_time_uptime_now_plus(const tiny_millis_t to_add)
{
        return getUptime() + to_add;
}

/**
 * Tells the caller whether or not the provided time is in the past.
 * @param time The time to check
 * @return True if the time is in the past, false otherwise.
 */
bool date_time_is_past(const tiny_millis_t time)
{
        return getUptime() > time;
}
