#ifndef CELLTELEMETRY_H_
#define CELLTELEMETRY_H_
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "loggerConfig.h"
#include "sampleRecord.h"
#include "stdint.h"


void cellTelemetryTask(void *params);

#endif /* CELLTELEMETRY_H_ */
