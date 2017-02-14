/*
 * mem_mang.h
 *
 *  Created on: Nov 2, 2013
 *      Author: brent
 */

#ifndef MEM_MANG_H_
#define MEM_MANG_H_
#include "heap.h"

#define portMalloc pvPortMalloc
#define portFree vPortFree
#define portRealloc pvPortRealloc
#define portGetFreeHeapSize xPortGetFreeHeapSize

#endif /* MEM_MANG_H_ */
