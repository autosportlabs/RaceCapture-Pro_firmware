#include "PWM_device.h"


#define MAX_DUTY_CYCLE 100


void PWM_device_configure_clock(unsigned short clockFrequency){
}

int PWM_device_init(){
	return 1;
}

void PWM_device_channel_init(unsigned int channel, unsigned short period, unsigned short dutyCycle){

}


void PWM_device_channel_start_all(){
}

void PWM_device_channel_stop_all(){
}

void PWM_device_channel_start(unsigned int channel){
}

void PWM_device_channel_stop(unsigned int channel){
}

void PWM_device_channel_set_period(unsigned int channel, unsigned short period)
{
}

unsigned short PWM_device_channel_get_period(unsigned int channel){
	return 0;
}

void PWM_device_set_duty_cycle(unsigned int channel, unsigned short duty){
}

unsigned short PWM_device_get_duty_cycle(unsigned int channel){
	return 0;
}


