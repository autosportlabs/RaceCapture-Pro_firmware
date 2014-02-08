#include "watchdog.h"
#include "watchdog_device.h"

inline void watchdog_reset(){
	watchdog_device_reset();
}

void watchdog_init(int timeoutMs){

}

