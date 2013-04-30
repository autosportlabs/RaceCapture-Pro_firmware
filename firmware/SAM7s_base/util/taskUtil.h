/*
 * taskUtil.h
 *
 *  Created on: Feb 18, 2012
 *      Author: brent
 */

#ifndef TASKUTIL_H_
#define TASKUTIL_H_

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"

int isTimeoutMs(portTickType start,unsigned int timeout);
void delayMs(unsigned int delay);

#endif /* TASKUTIL_H_ */
