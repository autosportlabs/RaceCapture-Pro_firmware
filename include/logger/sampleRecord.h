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

#define LOGGER_MSG_SAMPLE    0
#define LOGGER_MSG_START_LOG 1
#define LOGGER_MSG_END_LOG   2

// STIEG: Convert Ints to Int32 and Int64 types instead.
enum SampleData {
   SampleData_Int_Noarg,
   SampleData_Int,
   SampleData_LongLong_Noarg,
   SampleData_LongLong,
   SampleData_Float_Noarg,
   SampleData_Float,
   SampleData_Double_Noarg,
   SampleData_Double,
};

typedef struct _ChannelSample {
   // STIEG: Can we use const here?
   ChannelConfig *cfg;
   size_t channelIndex;
   bool populated;

   enum SampleData sampleData;
   union {
      int (*get_int_sample)(int);
      long long (*get_longlong_sample)(int);
      float (*get_float_sample)(int);
      double (*get_double_sample)(int);
      int (*get_int_sample_noarg)();
      long long (*get_longlong_sample_noarg)();
      float (*get_float_sample_noarg)();
      double (*get_double_sample_noarg)();
   };

   union {
      int valueInt;
      long long valueLongLong;
      float valueFloat;
      double valueDouble;
   };
} ChannelSample;

typedef struct _LoggerMessage
{
	int messageType;
	size_t sampleCount;
   // STIEG: Make this empty array pointer?
	ChannelSample *channelSamples;
} LoggerMessage;

ChannelSample* create_channel_sample_buffer(LoggerConfig *loggerConfig, size_t channelCount);

#endif /* SAMPLERECORD_H_ */
