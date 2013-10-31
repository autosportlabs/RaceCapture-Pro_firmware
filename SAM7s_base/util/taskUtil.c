/*
 * taskUtil.c
 *
 *  Created on: Feb 18, 2012
 *      Author: brent
 */
#include "taskUtil.h"

inline int isTimeoutMs(portTickType start,unsigned int timeout){
	return ((xTaskGetTickCount() - start) * portTICK_RATE_MS >= timeout);
}

inline void delayMs(unsigned int delay){
	vTaskDelay(delay / portTICK_RATE_MS);
}



