#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

extern xSemaphoreHandle onTick;

static unsigned int tickCount = 0;

void vApplicationTickHook(void){
	tickCount++;

	if (tickCount == 100){
		xSemaphoreGiveFromISR( onTick, pdFALSE );
		tickCount = 0;
	}
}

