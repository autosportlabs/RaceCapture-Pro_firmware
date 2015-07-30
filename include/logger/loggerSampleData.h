/*
 * loggerSampleData.h
 *
 *  Created on: Jan 31, 2014
 *      Author: brentp
 */

#ifndef LOGGERSAMPLEDATA_H_
#define LOGGERSAMPLEDATA_H_

#include <stddef.h>

#include "loggerConfig.h"
#include "sampleRecord.h"

/**
 * Populates a struct sample object with channel data.  Note this does not
 * handle the timestamping.  That is done by creation and association of
 * a LoggerMessage object.
 */
int populate_sample_buffer(struct sample *s, size_t logTick);

void init_channel_sample_buffer(LoggerConfig *loggerConfig,
                                struct sample *s);

float get_mapped_value(float value, ScalingMap *scalingMap);

#endif /* LOGGERSAMPLEDATA_H_ */
