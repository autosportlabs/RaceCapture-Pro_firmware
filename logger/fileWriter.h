#ifndef FILEWRITER_H_
#define FILEWRITER_H_
#include "loggerConfig.h"
#include "FreeRTOS.h"
#include "ff.h"
#include "sampleRecord.h"


void createFileWriterTask();
void fileWriterTask(void *params);
portBASE_TYPE queueLogfileRecord(LoggerMessage * sr);

#endif /* FILEWRITER_H_ */
