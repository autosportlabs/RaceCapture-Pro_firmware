/*
 * predictive_timer.h
 *
 *  Created on: Jul 24, 2013
 *      Author: brent
 */

#ifndef PREDICTIVE_TIMER_H_
#define PREDICTIVE_TIMER_H_

#include <stddef.h>

#define MAX_LOCATION_SAMPLES 10

typedef struct _LocationSample{
	float speed;
	float distance;
} LocationSample;

typedef struct _LapBuffer{
	LocationSample samples[MAX_LOCATION_SAMPLES];
	size_t sampleCount;
	size_t sampleInterval;
	size_t currentInterval;
	float currentDistAccumulator;
	float currentSpeedAccumulator;
} LapBuffer;

void init_timer();
void end_lap();
void add_sample(float speed, float distance);
LapBuffer * get_last_lap_buffer();
LapBuffer * get_current_lap_buffer();

float getPredictedTime();

#endif /* PREDICTIVE_TIMER_H_ */
