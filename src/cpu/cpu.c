#include "cpu.h"
#include "cpu_device.h"

int cpu_init(void){
	return cpu_device_init();
}

void cpu_reset(int bootloader){
	cpu_device_reset(bootloader);
}

const char * cpu_get_serialnumber(void){
	return cpu_device_get_serialnumber();
}
