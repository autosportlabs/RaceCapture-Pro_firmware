#ifndef LOGGERTASK_H_
#define LOGGERTASK_H_
#include "efs.h"


void createLoggerTask();
void loggerTask(void *params);
void writeLogFileValue(char * buf, int value, EmbeddedFile *f);

#endif /*LOGGERTASK_H_*/
