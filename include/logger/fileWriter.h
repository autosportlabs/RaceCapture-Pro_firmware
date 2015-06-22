#ifndef FILEWRITER_H_
#define FILEWRITER_H_
#include "loggerConfig.h"
#include "FreeRTOS.h"
#include "ff.h"
#include "sampleRecord.h"

#define FILENAME_LEN 13

enum writing_status {
    WRITING_INACTIVE = 0,
    WRITING_ACTIVE
};

struct logging_status
{
        bool logging;
        enum writing_status writing_status;
        portTickType flush_tick;
        portTickType write_tick;
        char name[FILENAME_LEN];
};


void startFileWriterTask( int priority );
void fileWriterTask(void *params);
portBASE_TYPE queue_logfile_record(LoggerMessage * sr);

#endif /* FILEWRITER_H_ */
