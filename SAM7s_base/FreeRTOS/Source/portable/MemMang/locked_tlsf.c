#include "tlsf.h"
#include "FreeRTOS.h"
#include "task.h"

//the following is defined in the linker script
extern unsigned int _CONFIG_HEAP_SIZE;
#define configTOTAL_HEAP_SIZE ((unsigned int)(&_CONFIG_HEAP_SIZE))
extern unsigned char  _heap_address[];

void initMemoryPool(){
	init_memory_pool(configTOTAL_HEAP_SIZE, _heap_address);

	//void *address = _heap_address;
	//init_memory_pool(50000, address);
}

void * pvPortMalloc( size_t xWantedSize ){
	vTaskSuspendAll();
	void * res = tlsf_malloc(xWantedSize);
	xTaskResumeAll();
	return res;
}

void vPortFree( void *pv ){
	vTaskSuspendAll();
	tlsf_free(pv);
	xTaskResumeAll();
}

void * pvPortRealloc( void *pv, size_t xWantedSize ){
	vTaskSuspendAll();
	void * res = tlsf_realloc(pv, xWantedSize);
	xTaskResumeAll();
	return res;
}
