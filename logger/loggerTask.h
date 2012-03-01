#ifndef LOGGERTASK_H_
#define LOGGERTASK_H_
#include "loggerConfig.h"
#include "FreeRTOS.h"
#include "ff.h"

#include <limits.h>

#define NIL_SAMPLE_VALUE INT_MIN
#define NIL_SAMPLE(X) X.intValue = NIL_SAMPLE_VALUE
#define IS_NIL_SAMPLE(X) (X.intValue == NIL_SAMPLE_VALUE)
#define SAMPLE_RECORD_CHANNELS (4 + CONFIG_ADC_CHANNELS + CONFIG_PWM_CHANNELS + CONFIG_GPIO_CHANNELS + CONFIG_TIMER_CHANNELS + CONFIG_ACCEL_CHANNELS)

struct ChannelSample
	{
	void * channelConfig;
	union
	{
		int intValue;
		float floatValue;
	};
};

struct SampleRecord
{
	union
	{
		struct ChannelSample Samples[SAMPLE_RECORD_CHANNELS];
	struct{
		struct ChannelSample ADCSamples[CONFIG_ADC_CHANNELS];
		struct ChannelSample PWMSamples[CONFIG_PWM_CHANNELS];
		struct ChannelSample GPIOSamples[CONFIG_GPIO_CHANNELS];
		struct ChannelSample TimerSamples[CONFIG_TIMER_CHANNELS];
		struct ChannelSample AccelSamples[CONFIG_ACCEL_CHANNELS];
		struct ChannelSample GPS_LatitueSample;
		struct ChannelSample GPS_LongitudeSample;
		struct ChannelSample GPS_VelocitySample;
		struct ChannelSample GPS_TimeSample;
		};
	};
};



void createLoggerTask();
void loggerTask(void *params);

portTickType getHighestIdleSampleRate(struct LoggerConfig *config);
portTickType getHighestSampleRate(struct LoggerConfig *config);


void lineAppendString(char *s);
void lineAppendInt(int num);
void lineAppendFloat(float num, int precision);
void lineAppendDouble(double num, int precision);

void fileWriteString(FIL *f, char *s);
void fileWriteInt(FIL *f, int num);
void fileWriteFloat(FIL *f, float num, int precision);
void fileWriteDouble(FIL *f, double num, int precision);

void writeHeaders(FIL *f, struct LoggerConfig *config);
void writeADCHeaders(FIL *f, struct LoggerConfig *config);
void writeGPIOHeaders(FIL *f, struct LoggerConfig *config);
void writeTimerChannelHeaders(FIL *f, struct LoggerConfig *config);
void writePWMChannelHeaders(FIL *f, struct LoggerConfig *config);
void writeAccelChannelHeaders(FIL *f, struct LoggerConfig *config);
void writeGPSChannelHeaders(FIL *f, GPSConfig *config);


void writeADC(portTickType currentTicks, struct LoggerConfig *config);
void writeGPIOs(portTickType currentTicks, struct LoggerConfig *config);
void writeTimerChannels(portTickType currentTicks, struct LoggerConfig *config);
void writePWMChannels(portTickType currentTicks, struct LoggerConfig *config);
void writeAccelerometer(portTickType currentTicks, struct LoggerConfig *config);
void writeGPSChannels(portTickType currentTicks, GPSConfig *config);

#endif /*LOGGERTASK_H_*/
