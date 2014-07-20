/**
 * Racecapture
 */

#ifndef __DATE_TIME_H__
#define __DATE_TIME_H__

#include <stdbool.h>
#include <stdint.h>

#define MILLIS_PER_SECOND (1000)
#define SECONDS_PER_MINUTE (60)
#define SECONDS_PER_HOUR (SECONDS_PER_MINUTE * 60)
#define SECONDS_PER_DAY (SECONDS_PER_HOUR * 24)

typedef struct _DateTime {
int16_t millisecond;
int8_t second;
int8_t minute;
int8_t hour;
int8_t day;
int8_t month;
int8_t partialYear;
} DateTime;

unsigned int convertToFullYear(int partialYear);
unsigned long getMillisecondsSinceUnixEpoch(DateTime dt);
long getTimeDeltaInMillis(DateTime a, DateTime b);

/**
 * Tells the caller if the given DateTime struct contains valid date Time info.
 */
bool isValid(const DateTime *dt);

#endif //__DATE_TIME_H__
