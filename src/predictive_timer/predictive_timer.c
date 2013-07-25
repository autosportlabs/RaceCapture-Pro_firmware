#include "predictive_timer.h"


static LapBuffer currentLapBuffer;
static LapBuffer lastLapBuffer;

static void init_lap_buffer(LapBuffer *buffer){
	buffer->currentDistAccumulator = 0;
	buffer->currentSpeedAccumulator = 0;
	buffer->sampleCount = 0;
	buffer->sampleInterval = 1;
	buffer->currentInterval = 0;
}

void init_timer(){
	init_lap_buffer(&currentLapBuffer);
}

void end_lap(){
	//move current buffer to last lap buffer
}

#define AVERAGE_TWO(n1, n2) (n1 + n2) / 2;

static void compact_lap_buffer(LapBuffer *buffer){
	size_t sampleCount = buffer->sampleCount;
	size_t newCount = buffer->sampleCount / 2;
	for (size_t i = 0; i < sampleCount; i+=2){
		LocationSample *s1 = &buffer->samples[i];
		LocationSample *s2 = &buffer->samples[i+2];

		s1->distance = s1->distance + s2->distance;
		s1->speed = AVERAGE_TWO(s1->speed, s2->speed);
	}
	buffer->sampleInterval++;
	buffer->sampleCount = newCount;
}

void add_sample(float speed, float distance){

	currentLapBuffer.currentDistAccumulator += distance;
	currentLapBuffer.currentSpeedAccumulator += speed;
	currentLapBuffer.currentInterval++;

	if (currentLapBuffer.currentInterval >= currentLapBuffer.sampleInterval){
		if (currentLapBuffer.sampleCount >= MAX_LOCATION_SAMPLES){
			compact_lap_buffer(&currentLapBuffer);
		}
		LocationSample *sample = &currentLapBuffer.samples[currentLapBuffer.sampleCount++];
		sample->distance = currentLapBuffer.currentDistAccumulator;
		sample->speed = currentLapBuffer.currentSpeedAccumulator / (float)currentLapBuffer.sampleInterval;
		currentLapBuffer.currentDistAccumulator = 0;
		currentLapBuffer.currentSpeedAccumulator = 0;
		currentLapBuffer.currentInterval = 0;
	}
}

float getPredictedTime(){
	return 0;
}

LapBuffer * get_last_lap_buffer(){
	return &lastLapBuffer;
}

LapBuffer * get_current_lap_buffer(){
	return &currentLapBuffer;
}



