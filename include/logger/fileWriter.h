#ifndef FILEWRITER_H_
#define FILEWRITER_H_
#include "loggerConfig.h"
#include "FreeRTOS.h"
#include "ff.h"
#include "sampleRecord.h"


void startFileWriterTask( int priority );
void fileWriterTask(void *params);
portBASE_TYPE queue_logfile_record(LoggerMessage * sr);

#endif /* FILEWRITER_H_ */
