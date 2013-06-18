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
{"sample", api_sampleData}, \
{"getCellCfg", api_getCellConfig}, \
{"setAnalogCfg", api_setAnalogConfig}, \
{"setAccelCfg", api_setAccelConfig}, \
{"setCellCfg", api_setCellConfig}, \
{"setPwmCfg", api_setPwmConfig}


void api_sampleData(Serial *serial, const jsmntok_t *json);
void api_getCellConfig(Serial *serial, const jsmntok_t *json);
void api_setCellConfig(Serial *serial, const jsmntok_t *json);
void api_setAnalogConfig(Serial *serial, const jsmntok_t *json);
void api_setAccelConfig(Serial *serial, const jsmntok_t *json);
void api_setPwmConfig(Serial *serial, const jsmntok_t *json);
#endif /* LOGGERAPI_H_ */
