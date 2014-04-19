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

bool isLeapPartialYear(unsigned int partialYear) {
   return isLeapYear(convertToFullYear(partialYear));
}

bool isLeapYear(unsigned int year) {
   /*
    * Credit to http://www.dispersiondesign.com/articles/time/determining_leap_years
    */
   return (year % 4) || ((year % 100 == 0) && (year % 400));
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
   unsigned long millis = 0;

   // Get everything as seconds first
   millis += dt.second;
   millis += dt.minute * SECONDS_PER_MINUTE;
   millis += dt.hour * SECONDS_PER_HOUR;
   millis += dt.day * SECONDS_PER_DAY;

   const unsigned int year = convertToFullYear(dt.partialYear);
   millis += getDayCountUpToMonthWithYear(dt.month, year) * SECONDS_PER_DAY;
   millis += getDayCountUpToYearSinceUnixEpoch(year) * SECONDS_PER_DAY;

   // Then convert to millis.
   millis *= MILLIS_PER_SECOND;

   // And then add in the remaining millis and return it.
   return millis += dt.millisecond;
}

long getTimeDeltaInMillis(DateTime a, DateTime b) {
   // HACK: I'm sure there is a better way to do this.  This way just works for me.
   return getMillisecondsSinceUnixEpoch(a) - getMillisecondsSinceUnixEpoch(b);
}

