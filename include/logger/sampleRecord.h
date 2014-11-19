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

/*
 * Important note here.  Use of Int and LongLong work on 32 bit ARM
 * architectures only in terms of their desired consistency of supplying
 * 32 and 64 bit values respectively.  We should change them to int32_t
 * and int64_t if we want to gaurantee this going forward.
 */
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
	ChannelSample *channelSamples;
} LoggerMessage;

ChannelSample* create_channel_sample_buffer(LoggerConfig *loggerConfig, size_t channelCount);

#endif /* SAMPLERECORD_H_ */
