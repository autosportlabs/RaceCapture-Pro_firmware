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
{"log", api_log}, \
{"getMeta", api_getMeta}, \
{"flashCfg", api_flashConfig}, \
{"setAnalogCfg", api_setAnalogConfig}, \
{"getAnalogCfg", api_getAnalogConfig}, \
{"getGpsCfg", api_getGpsConfig}, \
{"setGpsCfg", api_setGpsConfig}, \
{"getImuCfg", api_getImuConfig}, \
{"setImuCfg", api_setImuConfig}, \
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
{"setLapCfg", api_setLapConfig}, \
{"getLapCfg", api_getLapConfig}, \
{"getTrackCfg", api_getTrackConfig}, \
{"setTrackCfg", api_setTrackConfig}, \
{"calImu", api_calibrateImu}, \
{"getLogfile", api_getLogfile}, \
{"getTracks", api_getTracks}, \
{"getChannels", api_getChannels}


//commands
int api_sampleData(Serial *serial, const jsmntok_t *json);
int api_log(Serial *serial, const jsmntok_t *json);
int api_getMeta(Serial *serial, const jsmntok_t *json);
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
int api_setLapConfig(Serial *serial, const jsmntok_t *json);
int api_getLapConfig(Serial *serial, const jsmntok_t *json);
int api_getTrackConfig(Serial *serial, const jsmntok_t *json);
int api_setTrackConfig(Serial *serial, const jsmntok_t *json);
int api_getImuConfig(Serial *serial, const jsmntok_t *json);
int api_setImuConfig(Serial *serial, const jsmntok_t *json);
int api_getPwmConfig(Serial *serial, const jsmntok_t *json);
int api_setPwmConfig(Serial *serial, const jsmntok_t *json);
int api_getGpioConfig(Serial *serial, const jsmntok_t *json);
int api_setGpioConfig(Serial *serial, const jsmntok_t *json);
int api_getTimerConfig(Serial *serial, const jsmntok_t *json);
int api_setTimerConfig(Serial *serial, const jsmntok_t *json);
int api_calibrateImu(Serial *serial, const jsmntok_t *json);
int api_flashConfig(Serial *serial, const jsmntok_t *json);
int api_getLogfile(Serial *serial, const jsmntok_t *json);
int api_getTracks(Serial *serial, const jsmntok_t *json);
int api_getChannels(Serial *serial, const jsmntok_t *json);


//messages
void api_sendLogStart(Serial *serial);
void api_sendLogEnd(Serial *serial);
void api_sendSampleRecord(Serial *serial, ChannelSample *sr, size_t channelCount, unsigned int tick, int sendMeta);

//Utility functions
void unescapeTextField(char *data);

#endif /* LOGGERAPI_H_ */
