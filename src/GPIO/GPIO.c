#include "GPIO.h"
#include "GPIO_device.h"

int GPIO_init(LoggerConfig *loggerConfig){
	return GPIO_device_init(loggerConfig);
}

int GPIO_get(int port){
	return (int) GPIO_device_get((unsigned int) port);
}

void GPIO_set(int port, unsigned int state){
	GPIO_device_set((unsigned int) port, state);
}

int GPIO_is_button_pressed(void){
	return GPIO_device_is_button_pressed();
}
