#ifndef TIMER_H_
#define TIMER_H_
#include <stdint.h>
#include "loggerConfig.h"

int timer_init(LoggerConfig *loggerConfig);
uint32_t timer_get_raw(size_t channel);
uint32_t timer_get_usec(size_t channel);
uint32_t timer_get_ms(size_t channel);
uint32_t timer_get_rpm(size_t channel);
uint32_t timer_get_hz(size_t channel);
uint32_t timer_get_count(size_t channel);
void timer_reset_count(size_t channel);

unsigned int timerPeriodToUs(unsigned int ticks, unsigned int scaling);
unsigned int timerPeriodToMs(unsigned int ticks, unsigned int scaling);
unsigned int timerPeriodToHz(unsigned int ticks, unsigned int scaling);
unsigned int timerPeriodToRpm(unsigned int ticks, unsigned int scaling);

#endif /* TIMER_H_ */
