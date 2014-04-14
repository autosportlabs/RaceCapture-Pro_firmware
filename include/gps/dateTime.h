/**
 * Racecapture
 */

#ifndef __DATE_TIME_H__
#define __DATE_TIME_H__

#define SECONDS_PER_MINUTE (60)
#define SECONDS_PER_HOUR (SECONDS_PER_MINUTE * 60)
#define SECONDS_PER_DAY (SECONDS_PER_HOUR * 24)

typedef struct _DateTime {
int8_t deciseconds;
int8_t seconds;
int8_t minutes;
int8_t hour;
int8_t day;
int8_t month;
int16_t year;
} DateTime;

#endif __DATE_TIME_H__
