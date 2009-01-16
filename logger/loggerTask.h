#ifndef LOGGERTASK_H_
#define LOGGERTASK_H_
#include "efs.h"


void createLoggerTask();
void loggerTask(void *params);
void loggerWriter(void *params);
void fileWriteString(char *s);

#endif /*LOGGERTASK_H_*/
