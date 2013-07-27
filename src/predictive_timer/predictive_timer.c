#include "predictive_timer.h"


static LapBuffer _buffer1;
static LapBuffer _buffer2;

static LapBuffer * _currentLap;
static LapBuffer * _lastLap;

#define SEARCH_INDEX_OUT_OF_RANGE -1
static int cachedLastLapIndex;


static void init_lap_buffer(LapBuffer *buffer){
	buffer->currentDistAccumulator = 0;
	buffer->currentSpeedAccumulator = 0;
	buffer->currentTimeAccumulator = 0;
	buffer->sampleCount = 0;
	buffer->sampleInterval = 1;
	buffer->currentInterval = 0;
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
}

#define AVERAGE_TWO(n1, n2) (n1 + n2) / 2;

static void compact_lap_buffer(LapBuffer *buffer){
	size_t sampleCount = buffer->sampleCount;
	size_t newCount = buffer->sampleCount / 2;
	size_t targetIndex = 0;
	for (size_t i = 0; i < sampleCount; i+=2,targetIndex++){
		LocationSample *s1 = &buffer->samples[i];
		LocationSample *s2 = &buffer->samples[i+1];
		LocationSample *target = &buffer->samples[targetIndex];
		target->distance = s2->distance;
		target->time = s2->time;
		target->speed = AVERAGE_TWO(s1->speed, s2->speed);
	}
	buffer->sampleInterval++;
	buffer->sampleCount = newCount;
}

void add_predictive_sample(float speed, float distance, float time){

	_currentLap->currentSpeedAccumulator += speed;
	_currentLap->currentDistAccumulator += distance;
	_currentLap->currentTimeAccumulator += time;
	_currentLap->currentInterval++;

	size_t currentSampleCount = _currentLap->sampleCount;
	if (_currentLap->currentInterval >= _currentLap->sampleInterval){
		if (currentSampleCount >= MAX_LOCATION_SAMPLES){
			compact_lap_buffer(_currentLap);
		}
		else{
			LocationSample *sample = &_currentLap->samples[currentSampleCount];
			sample->speed = _currentLap->currentSpeedAccumulator / (float)_currentLap->sampleInterval;
			sample->distance = _currentLap->currentDistAccumulator;
			sample->time = _currentLap->currentTimeAccumulator;
			if (currentSampleCount > 0 ){
				sample->distance += _currentLap->samples[currentSampleCount - 1].distance;
				sample->time += _currentLap->samples[currentSampleCount - 1].time;
			}
			_currentLap->currentSpeedAccumulator = 0;
			_currentLap->currentDistAccumulator = 0;
			_currentLap->currentTimeAccumulator = 0;
			_currentLap->currentInterval = 0;
			_currentLap->sampleCount++;
		}
	}
}


static size_t getLastLapIndex(int startingIndex){
	float currentDistance = _currentLap->currentDistAccumulator + _currentLap->samples[_currentLap->sampleCount - 1].distance;

	size_t lastLapSampleCount = _lastLap->sampleCount;

	//start from the startingIndex to save time if possible.
	int searchIndex = currentDistance < _lastLap->samples[startingIndex].distance ? 0 : startingIndex;
	for (; searchIndex < lastLapSampleCount; searchIndex++){
		float dist1 = _lastLap->samples[searchIndex].distance;
		float dist2 = (searchIndex < lastLapSampleCount - 1 ? _lastLap->samples[searchIndex + 1].distance : dist1);
		if (currentDistance < dist1) break;
		if (currentDistance >= dist1 && currentDistance < dist2) break;
	}
	if (searchIndex >= lastLapSampleCount) searchIndex = SEARCH_INDEX_OUT_OF_RANGE;
	return searchIndex;
}

float get_predicted_time(float currentSpeed){

	//Time so far on current lap
	float predictedTime = _currentLap->currentTimeAccumulator + _currentLap->samples[_currentLap->sampleCount - 1].time;

	//find the same point in last lap buffer based on distance
	int lastLapIndex = getLastLapIndex(cachedLastLapIndex);

	if (lastLapIndex != SEARCH_INDEX_OUT_OF_RANGE){		//calc estimated remaining time on last lap
		//get the total time for the last lap
		float lastLapTotalTime = _lastLap->samples[_lastLap->sampleCount - 1].time + _lastLap->currentTimeAccumulator;

		//get the time and speed from the last lap at the same location
		LocationSample *lastLapLocationSample = &_lastLap->samples[lastLapIndex];
		float timeAtSameDistanceOnLastLap = lastLapLocationSample->time; //need to linear interpolate
		float speedAtSameDistanceOnLastLap = lastLapLocationSample->speed; //need to linear interpolate

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



