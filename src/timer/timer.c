#include "timer.h"
#include "timer_device.h"

int timer_init(LoggerConfig *loggerConfig){
	for (size_t i = 0; i < CONFIG_TIMER_CHANNELS; i++){
		TimerConfig *tc = &loggerConfig->TimerConfigs[i];
		timer_device_init(i, tc->timerDivider, tc->mode);
	}
	return 1;
}

unsigned int timer_get_period(unsigned int channel){
	return timer_device_get_period(channel);
}

unsigned int timer_get_count(unsigned int channel){
	return timer_device_get_count(channel);
}

void timer_reset_count(unsigned int channel){
	return timer_device_reset_count(channel);
}

