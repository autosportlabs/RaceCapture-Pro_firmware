/*
 * raceCaptureConfig.cpp
 *
 *  Created on: Apr 28, 2012
 *      Author: brent
 */
#include "raceCapture/raceCaptureConfig.h"

SampleRates ChannelConfig::sampleRates;

ChannelConfig::ChannelConfig(){
}

SampleRates ChannelConfig::GetSampleRates(){

//	const wxString SAMPLE_RATES[] = {"Disabled","1","5","10","20","30","50","100"};
	//TODO make threadsafe or change design
	if (ChannelConfig::sampleRates.Count() == 0){
		ChannelConfig::sampleRates.Add(sample_disabled);
		ChannelConfig::sampleRates.Add(sample_1Hz);
		ChannelConfig::sampleRates.Add(sample_5Hz);
		ChannelConfig::sampleRates.Add(sample_10Hz);
		ChannelConfig::sampleRates.Add(sample_20Hz);
		ChannelConfig::sampleRates.Add(sample_30Hz);
		ChannelConfig::sampleRates.Add(sample_50Hz);
		ChannelConfig::sampleRates.Add(sample_100Hz);
	}
	return sampleRates;
}




