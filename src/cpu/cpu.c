#include "cpu.h"
#include "cpu_device.h"

int cpu_init(void){
	return cpu_device_init();
}

void cpu_reset(void){
	cpu_device_reset();
}
