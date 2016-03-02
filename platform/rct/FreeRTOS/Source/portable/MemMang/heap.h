#ifndef HEAP_H_
#define HEAP_H_
#include <stdlib.h>

void *pvPortMalloc( size_t xWantedSize );
void vPortFree( void *pv );
void * pvPortRealloc( void *pv, size_t xWantedSize );
size_t xPortGetFreeHeapSize( void );

#endif
