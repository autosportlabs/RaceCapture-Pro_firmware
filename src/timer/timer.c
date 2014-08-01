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
	return timer_device_reset_count(channel);
}

