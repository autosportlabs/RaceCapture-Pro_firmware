#include "LED.h"

static int g_leds[3] = {0,0,0};

int LED_device_init(void){

}

void LED_device_enable(unsigned int Led){
	g_leds[Led] = 1;
}

void LED_device_disable(unsigned int Led){
	g_leds[Led] = 0;
}

void LED_device_toggle(unsigned int Led){
	g_leds[Led] = g_leds[Led] == 1 ? 0 : 1;
}
