/**
 * Racecapture
 */

#include <stdbool.h>

#include "capabilities.h"
#include "dateTime.h"
#include "FreeRTOS.h"
#include "task.h"

bool isLeapYear(const int year) {
   /*
    * Credit to http://www.dispersiondesign.com/articles/time/determining_leap_years
    */
   return (year % 4) || ((year % 100 == 0) && (year % 400)) ? false : true;
}

/**
 * Returns the days in the month.  1 = Jan.
 */
unsigned int getDaysInMonth(const int month, bool leapYear) {
   /*
    * Credit to http://www.dispersiondesign.com/articles/time/number_of_days_in_a_month
    * with a minor modification
    */

   return 31 - ((month == 2) ? (leapYear ? 2 : 3) : ((month - 1) % 7 % 2));
}

unsigned int getDaysInYear(const bool leapYear) {
   return leapYear ? 366 : 365;
}

unsigned int getDayCountUpToMonthWithYear(int month, const int year) {
   const bool ly = isLeapYear(year);
   unsigned int days = 0;

   while (--month > 0)
      days += getDaysInMonth(month, ly);

   return days;
}

unsigned int getDaysCountUpToMonthWithPartialYear(const int month,
                                                  const int year) {
   return getDayCountUpToMonthWithYear(month, year);
}

unsigned int getDayCountUpToYearSinceYear(int year, const int beforeYear) {
   unsigned int days = 0;

   while (--year >= beforeYear)
      days += getDaysInYear(isLeapYear(year));

   return days;
}

unsigned int getDayCountUpToYearSinceUnixEpoch(const int year) {
   return getDayCountUpToYearSinceYear(year, 1970);
}

millis_t getMillisecondsSinceUnixEpoch(DateTime dt) {
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

millis_t getTimeDeltaInMillis(DateTime a, DateTime b) {
   if (!isValidDateTime(a) || !isValidDateTime(b))
      return 0;

   // HACK: I'm sure there is a better way to do this.  This way just works for me.
   return getMillisecondsSinceUnixEpoch(a) - getMillisecondsSinceUnixEpoch(b);
}

tiny_millis_t getTimeDeltaInTinyMillis(DateTime a, DateTime b) {
   return (tiny_millis_t) getTimeDeltaInMillis(a, b);
}
static bool inRange(const int val, const int min, const int max) {
   return min <= val && val <= max;
}

bool isValidDateTime(const DateTime dt) {
   return inRange(dt.millisecond, 0, 999) &&
      inRange(dt.second, 0, 59) &&
      inRange(dt.minute, 0, 59) &&
      inRange(dt.hour, 0, 23) &&
      inRange(dt.day, 1, 31) &&
      inRange(dt.month, 1, 12) &&
      inRange(dt.year, 1900, 2099);
}

float millisToMinutes(const millis_t millis) {
   return ((float) (millis / MILLIS_PER_MINUTE)) +
      (((float) (millis % MILLIS_PER_MINUTE)) / MILLIS_PER_MINUTE);
}

float millisToSeconds(const millis_t millis) {
   return ((float) (millis / MILLIS_PER_SECOND)) +
      (((float) (millis % MILLIS_PER_SECOND)) / MILLIS_PER_SECOND);
}

float tinyMillisToMinutes(const tiny_millis_t millis) {
   return ((float) (millis / MILLIS_PER_MINUTE)) +
      (((float) (millis % MILLIS_PER_MINUTE)) / MILLIS_PER_MINUTE);
}

float tinyMillisToSeconds(const tiny_millis_t millis) {
   return ((float) (millis / MILLIS_PER_SECOND)) +
      (((float) (millis % MILLIS_PER_SECOND)) / MILLIS_PER_SECOND);
}

tiny_millis_t getUptime() {
   return (tiny_millis_t) (MS_PER_TICK * xTaskGetTickCount());
}

int getUptimeAsInt() {
   return (int) getUptime();
}
