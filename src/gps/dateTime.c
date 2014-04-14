/**
 * Racecapture
 */

#include "dateTime.h"

bool isLeapYear(const unsigned int year) {
   /*
    * Credit to http://www.dispersiondesign.com/articles/time/determining_leap_years
    */
   return (year % 4) || ((year % 100 === 0) && (year % 400));
}

/**
 * Returns the days in the month.  1 = Jan.
 */
int getDaysInMonth(const unsigned int month, bool isLeapYear) {
   /*
    * Credit to http://www.dispersiondesign.com/articles/time/number_of_days_in_a_month
    * with a minor modification
    */

   return 31 - ((month == 2) ? (isLeapYear ? 2 : 3) : ((month - 1) % 7 % 2));
}

DateTime getTimeDifference(DateTime a, DateTime b) {
   DateTime result;

   result.milliseconds = a.milliseconds - b.milliseconds;
   result.seconds = a.seconds - b.seconds;
   result.minutes = a.minutes - b.minutes;
   result.hour = a.hour - b.hour;
   result.day = a.day - b.day;
   result.month = a.month - b.month;
   result.year = a.year - b.year;

   return result;
}

int getDayCountUpToMonth(int month, int year) {
   const bool ly = isLeapYear(year);
   int days = 0;

   while(--month > 0)
      days += getDaysInMonth(month, ly);

   return days;
}

float getSecondsSinceYearStart(DateTime dt) {
   float seconds = 0.0;

   seconds += ((float) dt.milliseconds) / 1000;
   seconds += dt.seconds;
   seconds += ((float) dt.minutes) * SECONDS_PER_MINUTE;
   seconds += ((float) dt.hour) * SECONDS_PER_HOUR;
   seconds += ((float) dt.day) * SECONDS_PER_DAY;
   seconds += ((float) getDayCountUpToMonth(dt.month, dt.year)) * SECONDS_PER_DAY;

   return seconds;
}
