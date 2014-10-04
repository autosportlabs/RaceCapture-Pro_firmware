/**
 * Racecapture
 */

#ifndef __DATE_TIME_H__
#define __DATE_TIME_H__

#include <stdbool.h>
#include <stdint.h>

#define MILLIS_PER_MINUTE (60000)
#define MILLIS_PER_SECOND (1000)
#define SECONDS_PER_MINUTE (60)
#define SECONDS_PER_HOUR (3600)
#define SECONDS_PER_DAY (86400)

// Time type used for expressing time in millis since epoch
typedef int64_t millis_t;

// Time type used to save space on chip.
typedef int32_t tiny_millis_t;

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
millis_t getMillisecondsSinceUnixEpoch(DateTime dt);
millis_t getTimeDeltaInMillis(DateTime a, DateTime b);
tiny_millis_t getTimeDeltaInTinyMillis(DateTime a, DateTime b);
bool isValidDateTime(const DateTime dt);

/**
 * Converts Millisecond values to legacy minutes float.  Used as
 * a compatibility layer until we move to milliseconds only and
 * remove the use of floats as they are inaccurate.
 */
float millisToMinutes(const millis_t millis);

/**
 * Converts Millisecond values to legacy seconds float.  Used as
 * a compatibility layer until we move to milliseconds only and
 * remove the use of floats as they are inaccurate.
 */
float millisToSeconds(const millis_t millis);

/**
 * Converts Millisecond values to legacy minutes float.  Used as
 * a compatibility layer until we move to milliseconds only and
 * remove the use of floats as they are inaccurate.
 */
float tinyMillisToMinutes(const tiny_millis_t millis);

/**
 * Converts Millisecond values to legacy seconds float.  Used as
 * a compatibility layer until we move to milliseconds only and
 * remove the use of floats as they are inaccurate.
 */
float tinyMillisToSeconds(const tiny_millis_t millis);

#endif //__DATE_TIME_H__
