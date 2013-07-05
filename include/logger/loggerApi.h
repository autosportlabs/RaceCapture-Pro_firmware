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
{"flashCfg", api_flashConfig}, \
{"setAnalogCfg", api_setAnalogConfig}, \
{"getAnalogCfg", api_getAnalogConfig}, \
{"getGpsCfg", api_getGpsConfig}, \
{"setGpsCfg", api_setGpsConfig}, \
{"getAccelCfg", api_getAccelConfig}, \
{"setAccelCfg", api_setAccelConfig}, \
{"getBtCfg", api_getBluetoothConfig}, \
{"setBtCfg", api_setBluetoothConfig}, \
{"setConnCfg", api_setConnectivityConfig}, \
{"getConnCfg", api_getConnectivityConfig}, \
{"getPwmCfg", api_getPwmConfig}, \
{"setPwmCfg", api_setPwmConfig}, \
{"getGpioCfg", api_getGpioConfig}, \
{"setGpioCfg", api_setGpioConfig}, \
{"getTimerCfg", api_getTimerConfig}, \
{"setTimerCfg", api_setTimerConfig}, \
{"getCellCfg", api_getCellConfig}, \
{"setCellCfg", api_setCellConfig}, \
{"calAccel", api_calibrateAccel}


void writeSampleRecord(Serial *serial, SampleRecord *sr, int sendMeta);

void unescapeTextField(char *data);

int api_sampleData(Serial *serial, const jsmntok_t *json);
int api_enableLogging(Serial *serial, const jsmntok_t *json);
int api_getCellConfig(Serial *serial, const jsmntok_t *json);
int api_setCellConfig(Serial *serial, const jsmntok_t *json);
int api_getBluetoothConfig(Serial *serial, const jsmntok_t *json);
int api_setBluetoothConfig(Serial *serial, const jsmntok_t *json);
int api_getConnectivityConfig(Serial *serial, const jsmntok_t *json);
int api_setConnectivityConfig(Serial *serial, const jsmntok_t *json);
int api_getAnalogConfig(Serial *serial, const jsmntok_t *json);
int api_setAnalogConfig(Serial *serial, const jsmntok_t *json);
int api_getGpsConfig(Serial *serial, const jsmntok_t *json);
int api_setGpsConfig(Serial *serial, const jsmntok_t *json);
int api_getAccelConfig(Serial *serial, const jsmntok_t *json);
int api_setAccelConfig(Serial *serial, const jsmntok_t *json);
int api_getPwmConfig(Serial *serial, const jsmntok_t *json);
int api_setPwmConfig(Serial *serial, const jsmntok_t *json);
int api_getGpioConfig(Serial *serial, const jsmntok_t *json);
int api_setGpioConfig(Serial *serial, const jsmntok_t *json);
int api_getTimerConfig(Serial *serial, const jsmntok_t *json);
int api_setTimerConfig(Serial *serial, const jsmntok_t *json);
int api_calibrateAccel(Serial *serial, const jsmntok_t *json);
int api_flashConfig(Serial *serial, const jsmntok_t *json);

#endif /* LOGGERAPI_H_ */
