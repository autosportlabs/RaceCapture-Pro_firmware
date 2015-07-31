#ifndef FILEWRITER_H_
#define FILEWRITER_H_

#include "loggerConfig.h"
#include "FreeRTOS.h"
#include "ff.h"
#include "sampleRecord.h"

#define FILENAME_LEN 13
#define FLUSH_INTERVAL_MS 1000

enum writing_status {
    WRITING_INACTIVE = 0,
    WRITING_ACTIVE
};

struct logging_status
{
        bool logging;
        unsigned int rows_written;
        enum writing_status writing_status;
        portTickType flush_tick;
        char name[FILENAME_LEN];
};


void startFileWriterTask( int priority );
portBASE_TYPE queue_logfile_record(const LoggerMessage *msg);

#endif /* FILEWRITER_H_ */
