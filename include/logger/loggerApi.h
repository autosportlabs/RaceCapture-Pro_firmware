/**
 * AutoSport Labs - Race Capture Pro Firmware
 *
 * Copyright (C) 2014 AutoSport Labs
 *
 * This file is part of the Race Capture Pro firmware suite
 *
 * This is free software: you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details. You should have received a copy of the GNU
 * General Public License along with this code. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef LOGGERAPI_H_
#define LOGGERAPI_H_

#include "capabilities.h"
#include "serial.h"
#include "jsmn.h"
#include "api.h"
#include "sampleRecord.h"

/* TODO - make this more modular; maybe define size with preprocessor and then populate as necessary
#define LOGGER_API \
{"s", api_sampleData}, \
{"hb", api_heart_beat}, \
{"getVer", api_getVersion}, \
{"getStatus", api_getStatus}, \
{"getMeta", api_getMeta}, \
{"log", api_log}, \
{"getCapabilities", api_getCapabilities}, \
{"flashCfg", api_flashConfig}, \
{"getGpsCfg", api_getGpsConfig}, \
{"setGpsCfg", api_setGpsConfig}, \
{"setConnCfg", api_setConnectivityConfig}, \
{"getConnCfg", api_getConnectivityConfig}, \
{"getImuCfg", api_getImuConfig}, \
{"setImuCfg", api_setImuConfig}, \
{"calImu", api_calibrateImu}, \
#if ANALOG_CHANNELS > 0 \
{"setAnalogCfg", api_setAnalogConfig}, \
{"getAnalogCfg", api_getAnalogConfig}, \
#endif \
#if PWM_CHANNELS > 0 \
{"getPwmCfg", api_getPwmConfig}, \
{"setPwmCfg", api_setPwmConfig}, \
#endif \
#if GPIO_CHANNELS > 0 \
{"getGpioCfg", api_getGpioConfig}, \
{"setGpioCfg", api_setGpioConfig}, \
#endif \
#if TIMER_CHANNELS > 0 \
{"getTimerCfg", api_getTimerConfig}, \
{"setTimerCfg", api_setTimerConfig}, \
#endif \
{"setLapCfg", api_setLapConfig}, \
{"getLapCfg", api_getLapConfig}, \
{"getTrackCfg", api_getTrackConfig}, \
{"setTrackCfg", api_setTrackConfig}, \
{"getLogfile", api_getLogfile}, \
{"setLogfileLevel", api_setLogfileLevel}, \
{"getCanCfg", api_getCanConfig}, \
{"setCanCfg", api_setCanConfig}, \
{"getObd2Cfg", api_getObd2Config}, \
{"setObd2Cfg", api_setObd2Config}, \
#if LUA_SUPPORT == 1
{"getScriptCfg", api_getScript}, \
{"setScriptCfg", api_setScript}, \
{"runScript", api_runScript}, \
#endif
{"addTrackDb", api_addTrackDb}, \
{"getTrackDb", api_getTrackDb}, \
{"sysReset", api_systemReset}, \
{"facReset", api_factoryReset}

*/
#define LOGGER_API \
{"s", api_sampleData}, \
{"hb", api_heart_beat}, \
{"getVer", api_getVersion}, \
{"getStatus", api_getStatus}, \
{"getMeta", api_getMeta}, \
{"log", api_log}, \
{"getCapabilities", api_getCapabilities}, \
{"flashCfg", api_flashConfig}, \
{"getGpsCfg", api_getGpsConfig}, \
{"setGpsCfg", api_setGpsConfig}, \
{"setConnCfg", api_setConnectivityConfig}, \
{"getConnCfg", api_getConnectivityConfig}, \
{"getImuCfg", api_getImuConfig}, \
{"setImuCfg", api_setImuConfig}, \
{"calImu", api_calibrateImu}, \
{"setAnalogCfg", api_setAnalogConfig}, \
{"getAnalogCfg", api_getAnalogConfig}, \
{"setLapCfg", api_setLapConfig}, \
{"getLapCfg", api_getLapConfig}, \
{"getTrackCfg", api_getTrackConfig}, \
{"setTrackCfg", api_setTrackConfig}, \
{"getLogfile", api_getLogfile}, \
{"setLogfileLevel", api_setLogfileLevel}, \
{"getCanCfg", api_getCanConfig}, \
{"setCanCfg", api_setCanConfig}, \
{"getObd2Cfg", api_getObd2Config}, \
{"setObd2Cfg", api_setObd2Config}, \
{"addTrackDb", api_addTrackDb}, \
{"getTrackDb", api_getTrackDb}, \
{"sysReset", api_systemReset}, \
{"facReset", api_factoryReset}




//commands
int api_flashConfig(Serial *serial, const jsmntok_t *json);
int api_getVersion(Serial *serial, const jsmntok_t *json);
int api_getCapabilities(Serial *serial, const jsmntok_t *json);
int api_getStatus(Serial *serial, const jsmntok_t *json);
int api_systemReset(Serial *serial, const jsmntok_t *json);
int api_factoryReset(Serial *serial, const jsmntok_t *json);
int api_sampleData(Serial *serial, const jsmntok_t *json);
int api_heart_beat(Serial *serial, const jsmntok_t *json);
int api_log(Serial *serial, const jsmntok_t *json);
int api_getMeta(Serial *serial, const jsmntok_t *json);
int api_getConnectivityConfig(Serial *serial, const jsmntok_t *json);
int api_setConnectivityConfig(Serial *serial, const jsmntok_t *json);
int api_getGpsConfig(Serial *serial, const jsmntok_t *json);
int api_setGpsConfig(Serial *serial, const jsmntok_t *json);
int api_setLapConfig(Serial *serial, const jsmntok_t *json);
int api_getLapConfig(Serial *serial, const jsmntok_t *json);
int api_getTrackConfig(Serial *serial, const jsmntok_t *json);
int api_setTrackConfig(Serial *serial, const jsmntok_t *json);
int api_setLogfileLevel(Serial *serial, const jsmntok_t *json);
int api_getLogfile(Serial *serial, const jsmntok_t *json);
int api_getTrackDb(Serial *serial, const jsmntok_t *json);
int api_addTrackDb(Serial *serial, const jsmntok_t *json);
int api_getObd2Config(Serial *serial, const jsmntok_t *json);
int api_setObd2Config(Serial *serial, const jsmntok_t *json);
int api_getCanConfig(Serial *serial, const jsmntok_t *json);
int api_setCanConfig(Serial *serial, const jsmntok_t *json);

/* Sensor channels */
#if ANALOG_CHANNELS > 0
int api_getAnalogConfig(Serial *serial, const jsmntok_t *json);
int api_setAnalogConfig(Serial *serial, const jsmntok_t *json);
#endif
#if IMU_CHANNELS > 0
int api_getImuConfig(Serial *serial, const jsmntok_t *json);
int api_setImuConfig(Serial *serial, const jsmntok_t *json);
int api_calibrateImu(Serial *serial, const jsmntok_t *json);
#endif
#if PWM_CHANNELS > 0
int api_getPwmConfig(Serial *serial, const jsmntok_t *json);
int api_setPwmConfig(Serial *serial, const jsmntok_t *json);
#endif
#if GPIO_CHANNELS > 0
int api_getGpioConfig(Serial *serial, const jsmntok_t *json);
int api_setGpioConfig(Serial *serial, const jsmntok_t *json);
#endif
#if TIMER_CHANNELS > 0
int api_getTimerConfig(Serial *serial, const jsmntok_t *json);
int api_setTimerConfig(Serial *serial, const jsmntok_t *json);
#endif

#if LUA_SUPPORT == 1
int api_getScript(Serial *serial, const jsmntok_t *json);
int api_setScript(Serial *serial, const jsmntok_t *json);
int api_runScript(Serial *serial, const jsmntok_t *json);
#endif

//messages
void api_sendLogStart(Serial *serial);
void api_sendLogEnd(Serial *serial);
void api_send_sample_record(Serial *serial, struct sample *sample,
                            unsigned int tick, int sendMeta);

//Utility functions
void unescapeTextField(char *data);

#endif /* LOGGERAPI_H_ */
