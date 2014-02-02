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

#define DISCRETE_GPS_CHANNELS 10
#define NIL_SAMPLE INT_MIN + 1
#define SAMPLE_RECORD_CHANNELS (DISCRETE_GPS_CHANNELS + CONFIG_ADC_CHANNELS + CONFIG_PWM_CHANNELS + CONFIG_GPIO_CHANNELS + CONFIG_TIMER_CHANNELS + CONFIG_ACCEL_CHANNELS)

#define LOGGER_MSG_SAMPLE    0
#define LOGGER_MSG_START_LOG 1
#define LOGGER_MSG_END_LOG   2


typedef struct _ChannelSample
	{
	ChannelConfig * channelConfig;
	int precision;
	size_t channelIndex;
	float (*get_sample)(int);
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
		ChannelSample GPS_LatitueSample;
		ChannelSample GPS_LongitudeSample;
		ChannelSample GPS_SpeedSample;
		ChannelSample GPS_TimeSample;
		ChannelSample GPS_SatellitesSample;
		ChannelSample Track_LapCountSample;
		ChannelSample Track_LapTimeSample;
		ChannelSample Track_SplitTimeSample;
		ChannelSample Track_DistanceSample;
		ChannelSample Track_PredTimeSample;
		};
	};
} SampleRecord;

typedef struct _LoggerMessage
{
	int messageType;
	SampleRecord *sampleRecord;
} LoggerMessage;

void clearSampleRecordBuffer(SampleRecord **srBuff, int size);

void clearSampleRecord(SampleRecord* sr);

SampleRecord ** createSampleRecordBuffer(LoggerConfig *loggerConfig, int size);

void initSampleRecord(LoggerConfig *loggerConfig,SampleRecord *sr);

void initSampleRecordBuffer(LoggerConfig *loggerConfig, SampleRecord ** srBuff,int size);

void freeSampleRecordBuffer(SampleRecord ** sampleRecordBuffer);

#endif /* SAMPLERECORD_H_ */
