#ifndef LOGGERTASK_H_
#define LOGGERTASK_H_
#include "efs.h"


void createLoggerTask();
void loggerTask(void *params);
void loggerWriter(void *params);

void fileWriteString(char *s);
void fileWriteInt(int num);

void writeADC(unsigned int a0,unsigned int a1,unsigned int a2,unsigned int a3,unsigned int a4,unsigned int a5,unsigned int a6,unsigned int a7);

void writeAccelerometer();


void writeGPSPosition();
void writeGPSVelocity();

#endif /*LOGGERTASK_H_*/
