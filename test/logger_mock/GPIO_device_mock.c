#include "GPIO_device.h"

#define GPIO_CHANNELS 3

static int g_isCardPresent = 0;
static int g_isCardWritable = 0;
static int g_isButtonPressed = 0;
static int g_gpio[GPIO_CHANNELS] = {0,0,0};


void GPIO_device_init_port(unsigned int port, unsigned int mode){

}
void GPIO_device_init_SD_card_IO(void){

}

void GPIO_device_init_pushbutton(void){

}

void GPIO_device_init_events(void){

}

unsigned int GPIO_device_get(unsigned int port){
	return g_gpio[port];
}

void GPIO_device_set(unsigned int port, unsigned int state){
	g_gpio[port] = state;
}

int GPIO_device_is_SD_card_present(void){
	return g_isCardPresent;
}

int GPIO_device_is_SD_card_writable(void){
	return g_isCardWritable;
}

int GPIO_device_is_button_pressed(void){
	return g_isButtonPressed;
}

void mock_setIsCardPresent(int present){
	g_isCardPresent = present;
}

void mock_setIsCardWritable(int writable){
	g_isCardWritable = writable;
}

void mock_setIsButtonPressed(int pressed){
	g_isButtonPressed = pressed;
}

