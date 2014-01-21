#include "taskUtil.h"

inline int isTimeoutMs(portTickType start,unsigned int timeout){
	return ((xTaskGetTickCount() - start) * portTICK_RATE_MS >= timeout);
}

inline void delayMs(unsigned int delay){
	vTaskDelay(msToTicks(delay));
}

inline void delayTicks(size_t ticks){
	vTaskDelay(ticks);
}

inline size_t msToTicks(size_t ms){
	return ms / portTICK_RATE_MS;
}

inline size_t ticksToMs(size_t ticks){
	return ticks * portTICK_RATE_MS;
}




