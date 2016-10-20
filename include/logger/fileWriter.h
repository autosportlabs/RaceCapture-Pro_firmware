/*
 * Race Capture Firmware
 *
 * Copyright (C) 2016 Autosport Labs
 *
 * This file is part of the Race Capture firmware suite
 *
 * This is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details. You should
 * have received a copy of the GNU General Public License along with
 * this code. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef FILEWRITER_H_
#define FILEWRITER_H_

#include "FreeRTOS.h"
#include "cpp_guard.h"
#include "ff.h"
#include "loggerConfig.h"
#include "sampleRecord.h"

CPP_GUARD_BEGIN

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
	portTickType last_sample_tick;
        char name[FILENAME_LEN];
};


void startFileWriterTask( int priority );
portBASE_TYPE queue_logfile_record(const LoggerMessage *msg);

CPP_GUARD_END

#endif /* FILEWRITER_H_ */
