/*
 * taskUtil.h
 *
 *  Created on: Feb 18, 2012
 *      Author: brent
 */

#ifndef TASKUTIL_H_
#define TASKUTIL_H_

#include <stddef.h>

unsigned int getCurrentTicks();
int isTimeoutMs(unsigned int startTicks, unsigned int timeoutMs);
void delayMs(unsigned int delay);
void delayTicks(size_t ticks);
size_t msToTicks(size_t ms);
size_t ticksToMs(size_t ticks);

#endif /* TASKUTIL_H_ */
