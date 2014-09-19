/**
 * Racecapture
 */

#ifndef __DATE_TIME_H__
#define __DATE_TIME_H__

#include <stdbool.h>
#include <stdint.h>

#define MILLIS_PER_SECOND (1000)
#define SECONDS_PER_MINUTE (60)
#define SECONDS_PER_HOUR (3600)
#define SECONDS_PER_DAY (86400)

typedef struct _DateTime {
   int16_t millisecond;
   int8_t second;
   int8_t minute;
   int8_t hour;
   int8_t day;
   int8_t month;
   int8_t partialYear;
} DateTime;

bool isLeapYear(const unsigned int year);
unsigned int getDaysInMonth(const unsigned int month, bool leapYear);
unsigned long long getMillisecondsSinceUnixEpoch(DateTime dt);
long long getTimeDeltaInMillis(DateTime a, DateTime b);
bool isValidDateTime(const DateTime dt);
#endif //__DATE_TIME_H__
