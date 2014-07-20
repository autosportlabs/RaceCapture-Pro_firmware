/**
 * Racecapture
 */

#include "dateTime.h"
#include <stdbool.h>

/**
 * Converts years that end in XX to full XXXX years.  So 70 -> 1970.  01 -> 2001
 * @param partialYear The partial year as stored in DateTime
 * @return The full year.
 */
unsigned int convertToFullYear(int partialYear) {
   return  partialYear + (partialYear >= 70 ? 1900 : 2000);
}

bool isLeapYear(unsigned int year) {
   /*
    * Credit to http://www.dispersiondesign.com/articles/time/determining_leap_years
    */
   return (year % 4) || ((year % 100 == 0) && (year % 400));
}

bool isLeapPartialYear(unsigned int partialYear) {
   return isLeapYear(convertToFullYear(partialYear));
}

/**
 * Returns the days in the month.  1 = Jan.
 */
unsigned int getDaysInMonth(const unsigned int month, bool leapYear) {
   /*
    * Credit to http://www.dispersiondesign.com/articles/time/number_of_days_in_a_month
    * with a minor modification
    */

   return 31 - ((month == 2) ? (leapYear ? 2 : 3) : ((month - 1) % 7 % 2));
}

unsigned int getDaysInYear(const bool leapYear) {
   return leapYear ? 366 : 365;
}

unsigned int getDayCountUpToMonthWithYear(unsigned int month, unsigned int year) {
   const bool ly = isLeapYear(year);
   int days = 0;

   while (--month > 0)
      days += getDaysInMonth(month, ly);

   return days;
}

unsigned int getDaysCountUpToMonthWithPartialYear(unsigned int month,
      unsigned int partialYear) {
   return getDayCountUpToMonthWithYear(month, convertToFullYear(partialYear));
}

unsigned int getDayCountUpToYearSinceYear(unsigned int year, const unsigned int beforeYear) {
   unsigned int days = 0;

   while (--year >= beforeYear)
      days += getDaysInYear(isLeapYear(year));

   return days;
}

unsigned int getDayCountUpToYearSinceUnixEpoch(const unsigned int year) {
   return getDayCountUpToYearSinceYear(year, 1970);
}

unsigned int getDayCountUpToPartialYearSinceUnixEpoch(const unsigned int partialYear) {
   return getDayCountUpToYearSinceUnixEpoch(convertToFullYear(partialYear));
}

unsigned long getMillisecondsSinceUnixEpoch(DateTime dt) {
   unsigned long seconds = 0;

   // Get everything as seconds first
   seconds += dt.second;
   seconds += dt.minute * SECONDS_PER_MINUTE;
   seconds += dt.hour * SECONDS_PER_HOUR;
   seconds += dt.day * SECONDS_PER_DAY;

   const unsigned int year = convertToFullYear(dt.partialYear);
   seconds += getDayCountUpToMonthWithYear(dt.month, year) * SECONDS_PER_DAY;
   seconds += getDayCountUpToYearSinceUnixEpoch(year) * SECONDS_PER_DAY;

   // And then convert seconds to millis and add in the remaining millis
   return seconds * MILLIS_PER_SECOND + dt.millisecond;
}

long getTimeDeltaInMillis(DateTime a, DateTime b) {
   // HACK: I'm sure there is a better way to do this.  This way just works for me.
   return getMillisecondsSinceUnixEpoch(a) - getMillisecondsSinceUnixEpoch(b);
}

bool isValid(const DateTime *dt) {
	return dt->month != 0;
}
