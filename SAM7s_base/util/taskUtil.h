/*
 * taskUtil.h
 *
 *  Created on: Feb 18, 2012
 *      Author: brent
 */

#ifndef TASKUTIL_H_
#define TASKUTIL_H_

#include <stdint.h>
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"

int isTimeoutMs(portTickType start, unsigned int timeout);
inline void delayMs(unsigned int delay);
void delayTicks(size_t ticks);
inline size_t msToTicks(size_t ms);
size_t ticksToMs(size_t ticks);

#endif /* TASKUTIL_H_ */
