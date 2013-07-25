#include "predictive_timer.h"


static LapBuffer currentLapBuffer;
static LapBuffer lastLapBuffer;

void init_timer(){

}

void end_lap(){

}

static compact_lap_buffer(LapBuffer *buffer){
	size_t sampleCount = buffer->sampleCount;
	size_t newCount = buffer->sampleCount / 2;
	for (size_t i = 0; i < sampleCount; i+=2){

	}

}
//	buffer->sampleInterval++;


void add_sample(float latitude, float longitude){

}

float getPredictedTime(){
	return 0;

}




