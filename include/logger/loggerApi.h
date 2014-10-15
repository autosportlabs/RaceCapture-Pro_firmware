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
{"setConnCfg", api_setConnectivityConfig}, \
{"getConnCfg", api_getConnectivityConfig}, \
{"getPwmCfg", api_getPwmConfig}, \
{"setPwmCfg", api_setPwmConfig}, \
{"getGpioCfg", api_getGpioConfig}, \
{"setGpioCfg", api_setGpioConfig}, \
{"getTimerCfg", api_getTimerConfig}, \
{"setTimerCfg", api_setTimerConfig}, \
{"setLapCfg", api_setLapConfig}, \
{"getLapCfg", api_getLapConfig}, \
{"getTrackCfg", api_getTrackConfig}, \
{"setTrackCfg", api_setTrackConfig}, \
{"calImu", api_calibrateImu}, \
{"getLogfile", api_getLogfile}, \
{"setLogfileLevel", api_setLogfileLevel}, \
{"getCanCfg", api_getCanConfig}, \
{"setCanCfg", api_setCanConfig}, \
{"getObd2Cfg", api_getObd2Config}, \
{"setObd2Cfg", api_setObd2Config}, \
{"getScriptCfg", api_getScript}, \
{"setScriptCfg", api_setScript}, \
{"runScript", api_runScript}, \
{"addTrackDb", api_addTrackDb}, \
{"getTrackDb", api_getTrackDb}, \
{"addChannel", api_addChannel}, \
{"getChannels", api_getChannels}, \
{"getVer", api_getVersion}, \
{"getCapabilities", api_getCapabilities}, \
{"sysReset", api_systemReset}


//commands
int api_getVersion(Serial *serial, const jsmntok_t *json);
int api_getCapabilities(Serial *serial, const jsmntok_t *json);
int api_systemReset(Serial *serial, const jsmntok_t *json);
int api_sampleData(Serial *serial, const jsmntok_t *json);
int api_log(Serial *serial, const jsmntok_t *json);
int api_getMeta(Serial *serial, const jsmntok_t *json);
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
int api_setLogfileLevel(Serial *serial, const jsmntok_t *json);
int api_getLogfile(Serial *serial, const jsmntok_t *json);
int api_getTrackDb(Serial *serial, const jsmntok_t *json);
int api_addTrackDb(Serial *serial, const jsmntok_t *json);
int api_getChannels(Serial *serial, const jsmntok_t *json);
int api_addChannel(Serial *serial, const jsmntok_t *json);
int api_getObd2Config(Serial *serial, const jsmntok_t *json);
int api_setObd2Config(Serial *serial, const jsmntok_t *json);
int api_getCanConfig(Serial *serial, const jsmntok_t *json);
int api_setCanConfig(Serial *serial, const jsmntok_t *json);
int api_getScript(Serial *serial, const jsmntok_t *json);
int api_setScript(Serial *serial, const jsmntok_t *json);
int api_runScript(Serial *serial, const jsmntok_t *json);

//messages
void api_sendLogStart(Serial *serial);
void api_sendLogEnd(Serial *serial);
void api_sendSampleRecord(Serial *serial, ChannelSample *sr, size_t channelCount, unsigned int tick, int sendMeta);

//Utility functions
void unescapeTextField(char *data);

#endif /* LOGGERAPI_H_ */
