#ifndef TIMER_H_
#define TIMER_H_
#include "loggerConfig.h"

int timer_init(LoggerConfig *loggerConfig);
unsigned int timer_get_period(unsigned int channel);
unsigned int timer_get_count(unsigned int channel);
void timer_reset_count(unsigned int channel);

unsigned int timerPeriodToUs(unsigned int ticks, unsigned int scaling);
unsigned int timerPeriodToMs(unsigned int ticks, unsigned int scaling);
unsigned int timerPeriodToHz(unsigned int ticks, unsigned int scaling);
unsigned int timerPeriodToRpm(unsigned int ticks, unsigned int scaling);

#endif /* TIMER_H_ */
