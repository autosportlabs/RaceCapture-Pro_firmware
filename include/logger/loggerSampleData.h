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

#ifndef LOGGERSAMPLEDATA_H_
#define LOGGERSAMPLEDATA_H_

#include "cpp_guard.h"
#include "loggerConfig.h"
#include "sampleRecord.h"

#include <stddef.h>

CPP_GUARD_BEGIN

/**
 * Populates a struct sample object with channel data.  Note this does not
 * handle the timestamping.  That is done by creation and association of
 * a LoggerMessage object.
 */
int populate_sample_buffer(struct sample *s, size_t logTick);

void init_channel_sample_buffer(LoggerConfig *loggerConfig,
                                struct sample *s);

float get_mapped_value(float value, ScalingMap *scalingMap);

typedef void logger_sample_cb_t(const struct sample* sample,
                                const int ticks,
				void* data);

void logger_sample_process_callbacks(const int sample_tick,
                                     const struct sample* sample);

int logger_sample_create_callback(logger_sample_cb_t* cb, const int rate,
				  void* data);

bool logger_sample_destroy_callback(const int handle);

CPP_GUARD_END

#endif /* LOGGERSAMPLEDATA_H_ */
