#include "cpu.h"
#include "board.h"
#include "cpu_device.h"

int cpu_init(void){
	return cpu_device_init();
}

