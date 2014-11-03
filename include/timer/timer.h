#ifndef TIMER_H_
#define TIMER_H_
#include "loggerConfig.h"

// STIEG: Change these to functions to save space.
#define TIMER_PERIOD_TO_USEC(TICKS, SCALING) (unsigned int)((TICKS * 100000) / (SCALING / 10))
#define TIMER_PERIOD_TO_MS(TICKS, SCALING) (unsigned int)(TICKS * 1000) / SCALING
#define TIMER_PERIOD_TO_HZ(TICKS, SCALING) 1000000 / TIMER_PERIOD_TO_USEC(TICKS, SCALING)
#define TIMER_PERIOD_TO_RPM(TICKS, SCALING) 60000000 / TIMER_PERIOD_TO_USEC(TICKS, SCALING)

int timer_init(LoggerConfig *loggerConfig);
unsigned int timer_get_period(unsigned int channel);
unsigned int timer_get_count(unsigned int channel);
void timer_reset_count(unsigned int channel);

#endif /* TIMER_H_ */
