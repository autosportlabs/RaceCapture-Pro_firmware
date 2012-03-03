/*
 * sampleRecord.c
 *
 *  Created on: Feb 29, 2012
 *      Author: brent
 */
#include "sampleRecord.h"
#include "FreeRTOS.h"

SampleRecord * createNewSampleRecord(){
	SampleRecord *sr = (SampleRecord *)pvPortMalloc(sizeof(SampleRecord));
	for (int i=0; i < SAMPLE_RECORD_CHANNELS;i++){
		NIL_SAMPLE(sr->Samples[i]);
	}
	return sr;
}


