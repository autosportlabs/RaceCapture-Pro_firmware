#include "watchdog_device.h"
#include "LED.h"

inline void watchdog_device_reset(){
}

void watchdog_device_init(int timeoutMs){
}

static int get_reset_status(){
	return 0;
}

int watchdog_device_is_watchdog_reset(){
	return 0;
}

int watchdog_device_is_poweron_reset(){
	return 1;
}
