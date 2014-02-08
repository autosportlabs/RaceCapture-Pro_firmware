#include "PWM.h"
#include "PWM_device.h"

static void init_pwm_channel(size_t channelId, PWMConfig *pc){
	PWM_device_channel_init(channelId, pc->startupPeriod, pc->startupDutyCycle);
}
int PWM_init(void){
	PWM_device_init();

	for (size_t i = 0; i < CONFIG_PWM_CHANNELS; i++)
	{

		init_pwm_channel(i,&(loggerConfig->PWMConfigs[i]));

	}
}


