/*
 * THIS FILE IS DEPRECATED & SCHEDULED FOR DELETION
 *
 * Keeping this around now for any useful code it may have.  DO NOT ADD TO THIS FILE.
 * YOU HAVE BEEN WARNED!!!
 */

#include "predictive_timer.h"
#include "linear_interpolate.h"
//#include <stdio.h>

static LapBuffer _buffer1;
static LapBuffer _buffer2;

static LapBuffer * _currentLap;
static LapBuffer * _lastLap;

#define SEARCH_INDEX_OUT_OF_RANGE -1
#define AVERAGE_TWO(n1, n2) (n1 + n2) / 2;

static int cachedLastLapIndex;


static void init_lap_buffer(LapBuffer *buffer){

	LocationSample *sample = &buffer->samples[0];
	sample->distance = 0;
	sample->speed = 0;
	sample->time = 0;
	buffer->sampleIndex = 0;
	buffer->sampleInterval = 1;
	buffer->currentInterval = 0;
	buffer->currentSpeedAccumulator = 0;
}

void init_predictive_timer(){
	init_lap_buffer(&_buffer1);
	init_lap_buffer(&_buffer2);
	_currentLap = &_buffer1;
	_lastLap = &_buffer2;
	cachedLastLapIndex = 0;
}

void end_lap(){
	if (_currentLap == &_buffer1){
		_currentLap = &_buffer2;
		_lastLap = &_buffer1;
	}
	else{
		_currentLap = &_buffer1;
		_lastLap = &_buffer2;
	}
	init_lap_buffer(_currentLap);
	cachedLastLapIndex = 0;
}


static float getCurrentDistance(){
	return _currentLap->samples[_currentLap->sampleIndex].distance;
}

static float getCurrentTime(){
	return _currentLap->samples[_currentLap->sampleIndex].time;
}

static float getLastLapTotalTime(){
	return _lastLap->samples[_lastLap->sampleIndex].time;
}

static LocationSample * getCurrentLocationSample(){
	return &_currentLap->samples[_currentLap->sampleIndex];
}

static size_t getLastLapIndex(int startingIndex){
	float currentDistance = getCurrentDistance();

	size_t lastLapSampleIndex = _lastLap->sampleIndex;

	//start from the startingIndex to save time if possible.
	int searchIndex = currentDistance < _lastLap->samples[startingIndex].distance ? 0 : startingIndex;
	for (; searchIndex <= lastLapSampleIndex; searchIndex++){
		float dist1 = _lastLap->samples[searchIndex].distance;
		float dist2 = (searchIndex < lastLapSampleIndex ? _lastLap->samples[searchIndex + 1].distance : dist1);
		if (currentDistance < dist1) break;
		if (currentDistance >= dist1 && currentDistance < dist2) break;
	}
	if (searchIndex > lastLapSampleIndex) searchIndex = SEARCH_INDEX_OUT_OF_RANGE;
	return searchIndex;
}

static void outputLapBuffer(LapBuffer *buffer){
	for (size_t i = 0; i < buffer->sampleIndex; i++){
		//LocationSample *target = &buffer->samples[i];
		//printf("%d distance/time/speed %f %f %f\r\n", i, target->distance, target->time, target->speed );
	}
}

static void compact_lap_buffer(LapBuffer *buffer){

	outputLapBuffer(buffer);

	size_t targetIndex = 0;
	for (size_t i = 0; i < MAX_LOCATION_SAMPLES; i+=2,targetIndex++){
		LocationSample *s1 = &buffer->samples[i];
		LocationSample *s2 = &buffer->samples[i+1];
		LocationSample *target = &buffer->samples[targetIndex];
		target->distance = s2->distance;
		target->time = s2->time;
		target->speed = AVERAGE_TWO(s1->speed, s2->speed);
	//	printf("%d %d distance/time/speed %f %f %f\r\n", i, targetIndex, target->distance, target->time, target->speed );
	}
	buffer->sampleInterval++;
	buffer->sampleIndex = MAX_LOCATION_SAMPLES / 2;

	outputLapBuffer(buffer);
}


void add_predictive_sample(float speed, float distance, float time){

	if (_currentLap->currentInterval >= _currentLap->sampleInterval){
		_currentLap->sampleIndex++;
		if (_currentLap->sampleIndex >= MAX_LOCATION_SAMPLES){
			compact_lap_buffer(_currentLap);
		}
		_currentLap->currentSpeedAccumulator = 0;
		_currentLap->currentInterval = 0;
		LocationSample * sample = getCurrentLocationSample();
		sample->speed = 0;
		if (_currentLap->sampleIndex > 0 ){
			sample->distance = _currentLap->samples[_currentLap->sampleIndex - 1].distance;
			sample->time = _currentLap->samples[_currentLap->sampleIndex - 1].time;
		}
		else{
			sample->distance = 0;
			sample->time = 0;
		}
	}

	_currentLap->currentInterval++;
	LocationSample * sample = getCurrentLocationSample();

	//on the fly averaging of speed
	_currentLap->currentSpeedAccumulator += speed;
	sample->speed = _currentLap->currentSpeedAccumulator / _currentLap->currentInterval;
	sample->distance += distance;
	sample->time += time;
//	printf("dist/spd/time %f %f %f -- distance/speed/time %f %f %f\n", distance, speed, time, sample->distance, sample->speed, sample->time);
}

static void outputLapInfo(LapBuffer *b){
	for (int i = 0; i <= b->sampleIndex; i++){
	//	printf("%d distance/time/speed %f %f %f\n", i, b->samples[i].distance, b->samples[i].time, b->samples[i].speed);
	}
	//printf("currentSpeedAcc / currentInterval: %d %f", b->currentSpeedAccumulator, b->currentInterval);
}
float get_predicted_time(float currentSpeed){

	outputLapInfo(_lastLap);

	//Time so far on current lap
	float predictedTime = getCurrentTime();

	//find the same point in last lap buffer based on distance
	int lastLapIndex = getLastLapIndex(cachedLastLapIndex);

	if (lastLapIndex != SEARCH_INDEX_OUT_OF_RANGE){		//calc estimated remaining time on last lap
		//get the total time for the last lap
		float lastLapTotalTime =  getLastLapTotalTime();

		//get the time and speed from the last lap at the same location
		LocationSample *lastLapLocationSample = &_lastLap->samples[lastLapIndex];
		LocationSample *lastLapLocationSampleNext = &_lastLap->samples[lastLapIndex + 1];

		float currentDistance = getCurrentDistance();

		float timeAtSameDistanceOnLastLap = LinearInterpolate(currentDistance, lastLapLocationSample->distance, lastLapLocationSample->time, lastLapLocationSampleNext->distance, lastLapLocationSampleNext->time);
		float speedAtSameDistanceOnLastLap = LinearInterpolate(currentDistance, lastLapLocationSample->distance, lastLapLocationSample->speed, lastLapLocationSampleNext->distance, lastLapLocationSampleNext->speed);

		float estimatedRemainingTime = (lastLapTotalTime - timeAtSameDistanceOnLastLap) * (speedAtSameDistanceOnLastLap / currentSpeed );
		predictedTime += estimatedRemainingTime;
	}
	cachedLastLapIndex = lastLapIndex;
	return predictedTime;
}

LapBuffer * get_last_lap_buffer(){
	return _lastLap;
}

LapBuffer * get_current_lap_buffer(){
	return _currentLap;
}



