/*
 * sampleRecord.h
 *
 *  Created on: Feb 29, 2012
 *      Author: brent
 */

#ifndef SAMPLERECORD_H_
#define SAMPLERECORD_H_

#include <limits.h>
#include "loggerConfig.h"

#define DISCRETE_GPS_CHANNELS 8
#define NIL_SAMPLE INT_MIN
#define SAMPLE_RECORD_CHANNELS (DISCRETE_GPS_CHANNELS + CONFIG_ADC_CHANNELS + CONFIG_PWM_CHANNELS + CONFIG_GPIO_CHANNELS + CONFIG_TIMER_CHANNELS + CONFIG_ACCEL_CHANNELS)


typedef struct _ChannelSample
	{
	ChannelConfig * channelConfig;
	union
	{
		int intValue;
		float floatValue;
	};
} ChannelSample;

typedef struct _SampleRecord
{
	union
	{
		ChannelSample Samples[SAMPLE_RECORD_CHANNELS];
	struct{
		ChannelSample ADCSamples[CONFIG_ADC_CHANNELS];
		ChannelSample PWMSamples[CONFIG_PWM_CHANNELS];
		ChannelSample GPIOSamples[CONFIG_GPIO_CHANNELS];
		ChannelSample TimerSamples[CONFIG_TIMER_CHANNELS];
		ChannelSample AccelSamples[CONFIG_ACCEL_CHANNELS];
		ChannelSample GPS_LapCountSample;
		ChannelSample GPS_LapTimeSample;
		ChannelSample GPS_LatitueSample;
		ChannelSample GPS_LongitudeSample;
		ChannelSample GPS_VelocitySample;
		ChannelSample GPS_TimeSample;
		ChannelSample GPS_QualitySample;
		ChannelSample GPS_SatellitesSample;
		};
	};
} SampleRecord;

void clearSampleRecordBuffer(SampleRecord **srBuff, int size);

void clearSampleRecord(SampleRecord* sr);

SampleRecord ** createSampleRecordBuffer(LoggerConfig *loggerConfig, int size);

void initSampleRecord(LoggerConfig *loggerConfig,SampleRecord *sr);

void initSampleRecordBuffer(LoggerConfig *loggerConfig, SampleRecord ** srBuff,int size);

void freeSampleRecordBuffer(SampleRecord ** sampleRecordBuffer);

#endif /* SAMPLERECORD_H_ */
