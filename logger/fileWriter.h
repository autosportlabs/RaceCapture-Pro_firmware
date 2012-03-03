#ifndef FILEWRITER_H_
#define FILEWRITER_H_
#include "loggerConfig.h"
#include "FreeRTOS.h"
#include "ff.h"


void createFileWriterTask();
void fileWriterTask(void *params);


#endif /* FILEWRITER_H_ */
