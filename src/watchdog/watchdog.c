#include "watchdog.h"
#include "watchdog_device.h"

void watchdog_reset(){
	watchdog_device_reset();
}

void watchdog_init(int timeoutMs){
	watchdog_device_init(timeoutMs);
}

int watchdog_is_watchdog_reset(){
	return watchdog_device_is_watchdog_reset();
}

int watchdog_is_poweron_reset(){
	return watchdog_device_is_poweron_reset();
}
