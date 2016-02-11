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

#ifndef _FILEWRITER_TESTING_H_
#define _FILEWRITER_TESTING_H_

#include "fileWriter.h"
#include "cpp_guard.h"
#include "sampleRecord.h"

CPP_GUARD_BEGIN

int flush_logfile(struct logging_status *ls);
int logging_stop(struct logging_status *ls);
int logging_start(struct logging_status *ls);
int logging_sample(struct logging_status *ls, LoggerMessage *msg);

CPP_GUARD_END

#endif /* _FILEWRITER.TESTING_H_ */
