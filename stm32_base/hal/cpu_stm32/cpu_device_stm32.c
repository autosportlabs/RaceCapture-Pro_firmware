#include "cpu_device.h"
#include <stm32f4xx_misc.h>


int cpu_device_init(void){
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	return 1;
}

void cpu_device_reset(){

}
