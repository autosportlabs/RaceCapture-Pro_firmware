#ifndef LOGGERTASK_H_
#define LOGGERTASK_H_
#include "loggerConfig.h"
#include "FreeRTOS.h"
#include "ff.h"




void createLoggerTask();
void loggerTask(void *params);

portTickType getHighestIdleSampleRate(LoggerConfig *config);


void lineAppendString(char *s);
void lineAppendInt(int num);
void lineAppendFloat(float num, int precision);
void lineAppendDouble(double num, int precision);

void fileWriteString(FIL *f, char *s);
void fileWriteInt(FIL *f, int num);
void fileWriteFloat(FIL *f, float num, int precision);
void fileWriteDouble(FIL *f, double num, int precision);

void writeHeaders(FIL *f, LoggerConfig *config);
void writeADCHeaders(FIL *f, LoggerConfig *config);
void writeGPIOHeaders(FIL *f, LoggerConfig *config);
void writeTimerChannelHeaders(FIL *f, LoggerConfig *config);
void writePWMChannelHeaders(FIL *f, LoggerConfig *config);
void writeAccelChannelHeaders(FIL *f, LoggerConfig *config);
void writeGPSChannelHeaders(FIL *f, GPSConfig *config);


void writeADC(portTickType currentTicks, LoggerConfig *config);
void writeGPIOs(portTickType currentTicks, LoggerConfig *config);
void writeTimerChannels(portTickType currentTicks, LoggerConfig *config);
void writePWMChannels(portTickType currentTicks, LoggerConfig *config);
void writeAccelerometer(portTickType currentTicks, LoggerConfig *config);
void writeGPSChannels(portTickType currentTicks, GPSConfig *config);

#endif /*LOGGERTASK_H_*/
