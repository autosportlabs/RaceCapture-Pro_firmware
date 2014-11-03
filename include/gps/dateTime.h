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
   int16_t year;
} DateTime;

bool isLeapYear(const int year);
unsigned int getDaysInMonth(const int month, bool leapYear);
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

/**
 * Hacky method that gets us a rough idea of the amount of time the
 * system has been up.
 * <br>
 * IMPORTANT: This is currently based on the ticks and is
 * thus only as accurate as the tick rate.  Its subject to sku and
 * honestly should not be counted on being very accurate.  However since
 * this is primarily needed by the log file (which takes sampes no faster than
 * our tick rate), this will be sufficent for now.  Will fix this in
 * MK3 with a real time clock on chip.
 * @return The uptime of the sytem in milliseconds.
 */
tiny_millis_t getUptime();

int getUptimeAsInt();

#endif //__DATE_TIME_H__
