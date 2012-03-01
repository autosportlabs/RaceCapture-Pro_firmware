#ifndef LOGGERTASK_H_
#define LOGGERTASK_H_
#include "loggerConfig.h"
#include "FreeRTOS.h"
#include "ff.h"




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
