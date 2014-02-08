#include "PWM.h"
#include "PWM_device.h"

static void init_pwm_channel(size_t channelId, PWMConfig *pc){
	PWM_device_channel_init(channelId, pc->startupPeriod, pc->startupDutyCycle);
}

int PWM_init(LoggerConfig *loggerConfig){
	PWM_device_init();
	PWM_device_configure_clock(loggerConfig->PWMClockFrequency);
	for (size_t i = 0; i < CONFIG_PWM_CHANNELS; i++)
	{
		init_pwm_channel(i,&(loggerConfig->PWMConfigs[i]));
	}
	PWM_device_channel_start_all();
	return 1;
}

void PWM_set_duty_cycle(unsigned int channel, unsigned short duty){
	PWM_device_set_duty_cycle(channel, duty);
}

unsigned short PWM_get_duty_cycle(unsigned short channel){
	return PWM_device_get_duty_cycle(channel);
}

void PWM_channel_set_period(unsigned int channel, unsigned short period){
	PWM_device_channel_set_period(channel, period);
}

unsigned short PWM_channel_get_period(unsigned int channel){
	return PWM_channel_get_period(channel);
}

void PWM_channel_start(unsigned int channel){
	PWM_device_channel_start(channel);
}

void PWM_channel_stop(unsigned int channel){
	PWM_device_channel_stop(channel);
}

void PWM_channel_start_all(void){
	PWM_device_channel_start_all();
}

void PWM_channel_stop_all(void){
	PWM_device_channel_stop_all();
}
