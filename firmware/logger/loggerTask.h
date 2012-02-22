#ifndef LOGGERTASK_H_
#define LOGGERTASK_H_
#include "loggerConfig.h"
#include "FreeRTOS.h"
#include "ff.h"

#include <limits.h>

#define NIL_SAMPLE_VALUE INT_MIN
#define NIL_SAMPLE(X) X.intValue = NIL_SAMPLE_VALUE
#define IS_NIL_SAMPLE(X) (X.intValue == NIL_SAMPLE_VALUE)

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

//Not sure if I like where this is going, playing around.
struct SampleRecord * createNewSampleRecord(){
	struct SampleRecord *sr = (struct SampleRecord *)malloc(sizeof(struct SampleRecord));

	for (int i=0; i < CONFIG_ADC_CHANNELS;i++){
		//set channel config pointer from respective logger config channel
		NIL_SAMPLE(sr->ADCSamples[i]);
	}
	for (int i=0; i < CONFIG_PWM_CHANNELS;i++){
		NIL_SAMPLE(sr->PWMSamples[i]);
	}
	for (int i=0; i < CONFIG_GPIO_CHANNELS;i++){
		NIL_SAMPLE(sr->GPIOSamples[i]);
	}
	for (int i=0; i < CONFIG_TIMER_CHANNELS;i++){
		NIL_SAMPLE(sr->TimerSamples[i]);
	}
	for (int i=0; i < CONFIG_ACCEL_CHANNELS;i++){
		NIL_SAMPLE(sr->AccelSamples[i]);
	}
	NIL_SAMPLE(sr->GPS_LatitueSample);
	NIL_SAMPLE(sr->GPS_LongitudeSample);
	NIL_SAMPLE(sr->GPS_VelocitySample);
	NIL_SAMPLE(sr->GPS_TimeSample);
	return sr;
}

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
void writeGPSChannelHeaders(FIL *f, struct GPSConfig *config);


void writeADC(portTickType currentTicks, struct LoggerConfig *config);
void writeGPIOs(portTickType currentTicks, struct LoggerConfig *config);
void writeTimerChannels(portTickType currentTicks, struct LoggerConfig *config);
void writePWMChannels(portTickType currentTicks, struct LoggerConfig *config);
void writeAccelerometer(portTickType currentTicks, struct LoggerConfig *config);
void writeGPSChannels(portTickType currentTicks, struct GPSConfig *config);

#endif /*LOGGERTASK_H_*/
