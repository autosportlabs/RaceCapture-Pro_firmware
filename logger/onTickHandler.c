#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

extern xSemaphoreHandle onTick;

static unsigned int tickCount = 0;

void vApplicationTickHook(void){
	portCHAR xTaskWoken = pdFALSE;
	tickCount++;

	if (tickCount == 4){
		xSemaphoreGiveFromISR( onTick, xTaskWoken );
		tickCount = 0;
	}
}

