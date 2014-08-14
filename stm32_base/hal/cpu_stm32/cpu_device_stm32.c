#include "cpu_device.h"
#include <stm32f4xx_misc.h>
#include <stdint.h>

extern uint32_t _flash_start;

int cpu_device_init(void){
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, _flash_start);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	return 1;
}

void cpu_device_reset(){

}
