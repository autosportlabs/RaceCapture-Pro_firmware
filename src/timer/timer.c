#include "timer.h"
#include "timer_device.h"
#include "filter.h"
static Filter g_timer_filter[CONFIG_TIMER_CHANNELS];

int timer_init(LoggerConfig *loggerConfig){
	for (size_t i = 0; i < CONFIG_TIMER_CHANNELS; i++){
		TimerConfig *tc = &loggerConfig->TimerConfigs[i];
		timer_device_init(i, tc->timerDivider, tc->mode);
		init_filter(&g_timer_filter[i], tc->filterAlpha);
	}
	return 1;
}

unsigned int timer_get_period(unsigned int channel){
	Filter *filter = &g_timer_filter[channel];
	unsigned int period = timer_device_get_period(channel);
	update_filter(filter, period);
	return filter->current_value;
}

unsigned int timer_get_count(unsigned int channel){
	return timer_device_get_count(channel);
}

void timer_reset_count(unsigned int channel){
	timer_device_reset_count(channel);
}

unsigned int timerPeriodToUs(unsigned int ticks, unsigned int scaling) {
   return (ticks * 100000) / (scaling / 10);
}

unsigned int timerPeriodToMs(unsigned int ticks, unsigned int scaling) {
   return (ticks * 1000) / scaling;
}

unsigned int timerPeriodToHz(unsigned int ticks, unsigned int scaling) {
   return 1000000 / timerPeriodToUs(ticks, scaling);
}

unsigned int timerPeriodToRpm(unsigned int ticks, unsigned int scaling) {
   return 60000000 / timerPeriodToUs(ticks, scaling);
}
