#include "LED.h"
#include "LED_device.h"

int LED_init(void){
	return LED_device_init();
}

void LED_enable(unsigned int Led){
	LED_device_enable(Led);
}

void LED_disable(unsigned int Led){
	LED_device_disable(Led);
}

void LED_toggle(unsigned int Led){
	LED_device_toggle(Led);
}
