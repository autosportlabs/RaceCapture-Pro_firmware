#ifndef BTTELEMETRY_H_
#define BTTELEMETRY_H_
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "loggerConfig.h"
#include "sampleRecord.h"
#include "stdint.h"


void btTelemetryTask(void *params);

#endif /* CELLTELEMETRY_H_ */
