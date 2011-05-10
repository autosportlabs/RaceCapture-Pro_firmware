#ifndef LOGGERTASK_H_
#define LOGGERTASK_H_
#include "efs.h"
#include "loggerConfig.h"
#include "FreeRTOS.h"

void createLoggerTask();
void loggerTask(void *params);

portTickType getHighestIdleSampleRate(struct LoggerConfig *config);
portTickType getHighestSampleRate(struct LoggerConfig *config);


void lineAppendString(char *s);
void lineAppendInt(int num);
void lineAppendFloat(float num, int precision);
void lineAppendDouble(double num, int precision);

void fileWriteString(EmbeddedFile *f, char *s);
void fileWriteInt(EmbeddedFile *f, int num);
void fileWriteFloat(EmbeddedFile *f, float num, int precision);
void fileWriteDouble(EmbeddedFile *f, double num, int precision);

void writeHeaders(EmbeddedFile *f, struct LoggerConfig *config);
void writeADCHeaders(EmbeddedFile *f, struct LoggerConfig *config);
void writeGPIOHeaders(EmbeddedFile *f, struct LoggerConfig *config);
void writeTimerChannelHeaders(EmbeddedFile *f, struct LoggerConfig *config);
void writePWMChannelHeaders(EmbeddedFile *f, struct LoggerConfig *config);
void writeAccelChannelHeaders(EmbeddedFile *f, struct LoggerConfig *config);
void writeGPSChannelHeaders(EmbeddedFile *f, struct GPSConfig *config);


void writeADC(portTickType currentTicks, struct LoggerConfig *config);
void writeGPIOs(portTickType currentTicks, struct LoggerConfig *config);
void writeTimerChannels(portTickType currentTicks, struct LoggerConfig *config);
void writePWMChannels(portTickType currentTicks, struct LoggerConfig *config);
void writeAccelerometer(portTickType currentTicks, struct LoggerConfig *config);
void writeGPSChannels(portTickType currentTicks, struct GPSConfig *config);

#endif /*LOGGERTASK_H_*/
