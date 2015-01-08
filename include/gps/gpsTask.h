/*
 * gpsTask.h
 *
 *  Created on: Jun 22, 2013
 *      Author: brent
 */
#ifndef GPSTASK_H_
#define GPSTASK_H_

#include "stdbool.h"

void GPSTask( void *pvParameters );

void startGPSTask(int priority);
void setGpsDataLogging(bool);

#endif /* GPSTASK_H_ */
