/*
 * sampleRecord.h
 *
 *  Created on: Feb 29, 2012
 *      Author: brent
 */

#ifndef SAMPLERECORD_H_
#define SAMPLERECORD_H_

#include <stdbool.h>
#include <stddef.h>

#include "dateTime.h"
#include "loggerConfig.h"

enum LoggerMessageType {
   LoggerMessageType_Sample,
   LoggerMessageType_Start,
   LoggerMessageType_Stop,
};

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
	enum LoggerMessageType type;
	size_t sampleCount;
    size_t ticks;
	ChannelSample *channelSamples;
} LoggerMessage;

ChannelSample* create_channel_sample_buffer(LoggerConfig *loggerConfig, size_t channelCount);

/**
 * Checks to ensure that the LoggerMessage object is not older than 10 milliseconds.  If it is then
 * we may read an old buffer and send bad data.
 * @param lm The LoggerMessage object.
 * @return True if its not older than 10ms, false otherwise.
 */
int isValidLoggerMessageAge(LoggerMessage *lm);

#endif /* SAMPLERECORD_H_ */
