#include "predictive_timer.h"


static LapBuffer currentLapBuffer;
static LapBuffer lastLapBuffer;

void init_timer(){

}

void end_lap(){

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

	buffer->sampleCount = newCount;
}

void add_sample(float latitude, float longitude, float speed, float distance){

	if (currentLapBuffer.sampleCount >= MAX_LOCATION_SAMPLES){
		compact_lap_buffer(&currentLapBuffer);
	}
	LocationSample *sample = &currentLapBuffer.samples[currentLapBuffer.sampleCount++];
	sample->distance = distance;
	sample->latitude = latitude;
	sample->longitude = longitude;
	sample->speed = speed;
}

float getPredictedTime(){
	return 0;
}




