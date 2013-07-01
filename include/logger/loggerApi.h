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
#include "sampleRecord.h"

#ifndef BOARD_MCK
#define BOARD_MCK 48054840
#endif

#define LOGGER_API \
{"s", api_sampleData}, \
{"log", api_enableLogging}, \
{"getCellCfg", api_getCellConfig}, \
{"setAnalogCfg", api_setAnalogConfig}, \
{"setAccelCfg", api_setAccelConfig}, \
{"setCellCfg", api_setCellConfig}, \
{"setBtCfg", api_setBluetoothConfig}, \
{"setPwmCfg", api_setPwmConfig}, \
{"setGpioCfg", api_setGpioConfig}, \
{"setTimerCfg", api_setTimerConfig}, \
{"calAccel", api_calibrateAccel}, \
{"flashCfg", api_flashConfig}


void writeSampleRecord(Serial *serial, SampleRecord *sr, int sendMeta);


int api_sampleData(Serial *serial, const jsmntok_t *json);
int api_enableLogging(Serial *serial, const jsmntok_t *json);
int api_getCellConfig(Serial *serial, const jsmntok_t *json);
int api_setCellConfig(Serial *serial, const jsmntok_t *json);
int api_setBluetoothConfig(Serial *serial, const jsmntok_t *json);
int api_setAnalogConfig(Serial *serial, const jsmntok_t *json);
int api_setAccelConfig(Serial *serial, const jsmntok_t *json);
int api_setPwmConfig(Serial *serial, const jsmntok_t *json);
int api_setGpioConfig(Serial *serial, const jsmntok_t *json);
int api_setTimerConfig(Serial *serial, const jsmntok_t *json);
int api_calibrateAccel(Serial *serial, const jsmntok_t *json);
int api_flashConfig(Serial *serial, const jsmntok_t *json);

#endif /* LOGGERAPI_H_ */
