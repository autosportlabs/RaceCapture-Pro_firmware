#include "watchdog_device.h"
#include "LED.h"
#include <stm32f4xx_iwdg.h>


inline void watchdog_device_reset(){
}

void watchdog_device_init(int timeoutMs){
}

int watchdog_device_is_watchdog_reset(){
	return 0;
}

int watchdog_device_is_poweron_reset(){
	return 1;
}
