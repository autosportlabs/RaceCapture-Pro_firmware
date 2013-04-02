#ifndef CONSOLECONNECTIVITY_H_
#define CONSOLECONNECTIVITY_H_
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "loggerConfig.h"
#include "sampleRecord.h"
#include "stdint.h"


void consoleConnectivityTask(void *params);

#endif /* CONSOLECONNECTIVITY_H_ */
