#include "taskUtil.h"
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"

inline unsigned int getCurrentTicks(){
	return xTaskGetTickCount();
}

inline int isTimeoutMs(unsigned int startTicks, unsigned int timeoutMs){
	return ((xTaskGetTickCount() - startTicks) * portTICK_RATE_MS >= timeoutMs);
}

inline void delayMs(unsigned int delay){
	vTaskDelay(msToTicks(delay));
}

inline void delayTicks(size_t ticks){
	vTaskDelay(ticks);
}

size_t msToTicks(size_t ms){
	return ms / portTICK_RATE_MS;
}

inline size_t ticksToMs(size_t ticks){
	return ticks * portTICK_RATE_MS;
}




