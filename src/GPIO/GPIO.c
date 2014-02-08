#include "GPIO.h"
#include "GPIO_device.h"

int GPIO_init(LoggerConfig *loggerConfig){
	GPIO_device_init_SD_card_IO();
	GPIO_device_init_pushbutton();
	for (size_t i = 0; i < CONFIG_GPIO_CHANNELS; i++){
		GPIO_device_init_port(i, loggerConfig->GPIOConfigs[i].mode);
	}
	GPIO_device_init_events();
	return 1;
}

unsigned int GPIO_get(unsigned int port){
	return GPIO_device_get(port);
}

void GPIO_set(unsigned int port, unsigned int state){
	GPIO_device_set(port, state);
}

int GPIO_is_SD_card_present(void){
	return GPIO_device_is_SD_card_present();
}

int GPIO_is_SD_card_writable(void){
	return GPIO_device_is_SD_card_writable();
}

int GPIO_is_button_pressed(void){
	return GPIO_device_is_button_pressed();
}

