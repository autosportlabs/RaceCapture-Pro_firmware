#include "GPIO.h"
#include "GPIO_device.h"

int GPIO_init(LoggerConfig *loggerConfig){
	return GPIO_device_init(loggerConfig);
}

unsigned int GPIO_get(unsigned int port){
	return GPIO_device_get(port);
}

void GPIO_set(unsigned int port, unsigned int state){
	GPIO_device_set(port, state);
}

int GPIO_is_button_pressed(void){
	return GPIO_device_is_button_pressed();
}

