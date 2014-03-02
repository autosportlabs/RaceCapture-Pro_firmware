/*
 * virtual_channel_lock.c
 *
 *  Created on: Mar 2, 2014
 *      Author: brent
 */
#include "virtual_channel_lock.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"

static xSemaphoreHandle virtualChannelLock;

int init_virtual_channel_lock(){
	vSemaphoreCreateBinary(virtualChannelLock);
	return 1;
}
void lock_virtual_channel(){
	xSemaphoreTake(virtualChannelLock, portMAX_DELAY);
}

void unlock_virtual_channel(){
	xSemaphoreGive(virtualChannelLock);
}

