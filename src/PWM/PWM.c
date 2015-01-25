#include "PWM.h"
#include "PWM_device.h"

static void init_pwm_channel(size_t channelId, PWMConfig *pc){
	PWM_device_channel_init(channelId, pc->startupPeriod, pc->startupDutyCycle);
}

int PWM_init(LoggerConfig *loggerConfig){
	PWM_device_init();
	PWM_set_clock_frequency(loggerConfig->PWMClockFrequency);

	for (size_t i = 0; i < CONFIG_PWM_CHANNELS; i++)
	{
		PWMConfig *pwmConfig = &(loggerConfig->PWMConfigs[i]);
		init_pwm_channel(i,pwmConfig);
	}
	PWM_device_channel_start_all();

	for (size_t i = 0; i < CONFIG_PWM_CHANNELS; i++){
		PWMConfig *pwmConfig = &(loggerConfig->PWMConfigs[i]);
		PWM_device_set_duty_cycle(i, pwmConfig->startupDutyCycle);
	}
	PWM_update_config(loggerConfig);
	return 1;
}

void PWM_set_clock_frequency(uint16_t clockFrequency){
	PWM_device_set_clock_frequency(clockFrequency);
}

int PWM_update_config(LoggerConfig *loggerConfig){
	for (size_t i = 0; i < CONFIG_PWM_CHANNELS; i++){
		PWMConfig *pwmConfig = &(loggerConfig->PWMConfigs[i]);
		PWM_channel_enable_analog(i, pwmConfig->outputMode == MODE_PWM_ANALOG);
	}
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

void PWM_channel_enable_analog(size_t channel, uint8_t enabled){
	PWM_device_channel_enable_analog(channel, enabled);
}

