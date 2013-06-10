/*
 * loggerApi.h
 *
 *  Created on: Jun 10, 2013
 *      Author: brent
 */

#ifndef LOGGERAPI_H_
#define LOGGERAPI_H_

#include "serial.h"
#include "jsmn.h"
#include "api.h"

#define LOGGER_API \
{"sample", ,api_sample_data}

void api_sample_data(Serial *serial, const jsmntok_t *json);
void api_get_cell_config(Serial *serial, const jsmntok_t *json);

#endif /* LOGGERAPI_H_ */
