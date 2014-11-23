#include "timer.h"
#include "timer_device.h"
#include "filter.h"

static Filter g_timer_filter[CONFIG_TIMER_CHANNELS];

int timer_init(LoggerConfig *loggerConfig){
	for (size_t i = 0; i < CONFIG_TIMER_CHANNELS; i++){
		TimerConfig *tc = &loggerConfig->TimerConfigs[i];
		timer_device_init(i, tc->timerSpeed, tc->mode);
		init_filter(&g_timer_filter[i], tc->filterAlpha);
	}
	return 1;
}

uint32_t timer_get_raw(size_t channel){
	return timer_device_get_period(channel);
}

uint32_t timer_get_hz(size_t channel){
	uint32_t usec = timer_get_usec(channel);
	return usec > 0 ? 1000000 / usec : 0;
}

uint32_t timer_get_ms(size_t channel){
	return timer_get_usec(channel) / 1000;
}

uint32_t timer_get_rpm(size_t channel){
	return timer_get_hz(channel) * 60;
}

uint32_t timer_get_usec(size_t channel){
	Filter *filter = &g_timer_filter[channel];
	unsigned int period = timer_device_get_usec(channel);
	update_filter(filter, period);
	return filter->current_value;
}

uint32_t timer_get_count(size_t channel){
	return timer_device_get_count(channel);
}

void timer_reset_count(size_t channel){
	timer_device_reset_count(channel);
}
