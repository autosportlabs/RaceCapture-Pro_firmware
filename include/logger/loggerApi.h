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

#ifndef LOGGERAPI_H_
#define LOGGERAPI_H_

#include "api.h"
#include "capabilities.h"
#include "cpp_guard.h"
#include "jsmn.h"
#include "sampleRecord.h"
#include "serial.h"

CPP_GUARD_BEGIN

#define API_METHOD(_NAME, _FUNC) {(_NAME), (_FUNC)},

#define BASE_API_METHODS                                        \
        API_METHOD("addTrackDb", api_addTrackDb)                \
        API_METHOD("facReset", api_factoryReset)                \
        API_METHOD("getCanCfg", api_getCanConfig)               \
        API_METHOD("getLapCfg", api_getLapConfig)               \
        API_METHOD("getLogfile", api_getLogfile)                \
        API_METHOD("getObd2Cfg", api_getObd2Config)             \
        API_METHOD("getTrackCfg", api_getTrackConfig)           \
        API_METHOD("getTrackDb", api_getTrackDb)                \
        API_METHOD("setCanCfg", api_setCanConfig)               \
        API_METHOD("setLapCfg", api_setLapConfig)               \
        API_METHOD("setLogfileLevel", api_setLogfileLevel)      \
        API_METHOD("setObd2Cfg", api_setObd2Config)             \
        API_METHOD("setTrackCfg", api_setTrackConfig)           \
        API_METHOD("sysReset", api_systemReset)                 \
        API_METHOD("calImu", api_calibrateImu)                  \
        API_METHOD("flashCfg", api_flashConfig)                 \
        API_METHOD("getCapabilities", api_getCapabilities)      \
        API_METHOD("getConnCfg", api_getConnectivityConfig)     \
        API_METHOD("getGpsCfg", api_getGpsConfig)               \
        API_METHOD("getImuCfg", api_getImuConfig)               \
        API_METHOD("getMeta", api_getMeta)                      \
        API_METHOD("getStatus", api_getStatus)                  \
        API_METHOD("getVer", api_getVersion)                    \
        API_METHOD("hb", api_heart_beat)                        \
        API_METHOD("log", api_log)                              \
        API_METHOD("s", api_sampleData)                         \
        API_METHOD("setConnCfg", api_setConnectivityConfig)     \
        API_METHOD("setGpsCfg", api_setGpsConfig)               \
        API_METHOD("setImuCfg", api_setImuConfig)

#if ANALOG_CHANNELS > 0
#define ANALOG_API_METHODS                              \
        API_METHOD("getAnalogCfg", api_getAnalogConfig) \
        API_METHOD("setAnalogCfg", api_setAnalogConfig)
#else
#define ANALOG_API_METHODS
#endif

#if PWM_CHANNELS > 0
#define PWM_API_METHODS                                 \
        API_METHOD("getPwmCfg", api_getPwmConfig)       \
        API_METHOD("setPwmCfg", api_setPwmConfig)
#else
#define PWM_API_METHODS
#endif

#if GPIO_CHANNELS > 0
#define GPIO_API_METHODS                                \
        API_METHOD("getGpioCfg", api_getGpioConfig)     \
        API_METHOD("setGpioCfg", api_setGpioConfig)
#else
#define GPIO_API_METHODS
#endif

#if TIMER_CHANNELS > 0
#define TIMER_API_METHODS                               \
        API_METHOD("getTimerCfg", api_getTimerConfig)   \
        API_METHOD("setTimerCfg", api_setTimerConfig)
#else
#define TIMER_API_METHODS
#endif

#if defined(LUA_SUPPORT)
#define LUA_API_METHODS                                 \
        API_METHOD("getScriptCfg", api_getScript)       \
        API_METHOD("runScript", api_runScript)          \
        API_METHOD("setScriptCfg", api_setScript)
#else
#define LUA_API_METHODS
#endif

#define API_METHODS                             \
        BASE_API_METHODS                        \
        ANALOG_API_METHODS                      \
        PWM_API_METHODS                         \
        GPIO_API_METHODS                        \
        TIMER_API_METHODS                       \
        LUA_API_METHODS


/* commands */
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
int api_getAnalogConfig(Serial *serial, const jsmntok_t *json);
int api_setAnalogConfig(Serial *serial, const jsmntok_t *json);
int api_getImuConfig(Serial *serial, const jsmntok_t *json);
int api_setImuConfig(Serial *serial, const jsmntok_t *json);
int api_calibrateImu(Serial *serial, const jsmntok_t *json);
int api_getPwmConfig(Serial *serial, const jsmntok_t *json);
int api_setPwmConfig(Serial *serial, const jsmntok_t *json);
int api_getGpioConfig(Serial *serial, const jsmntok_t *json);
int api_setGpioConfig(Serial *serial, const jsmntok_t *json);
int api_getTimerConfig(Serial *serial, const jsmntok_t *json);
int api_setTimerConfig(Serial *serial, const jsmntok_t *json);

int api_getScript(Serial *serial, const jsmntok_t *json);
int api_setScript(Serial *serial, const jsmntok_t *json);
int api_runScript(Serial *serial, const jsmntok_t *json);

//messages
void api_sendLogStart(Serial *serial);
void api_sendLogEnd(Serial *serial);
void api_send_sample_record(Serial *serial, struct sample *sample,
                            unsigned int tick, int sendMeta);
//Utility functions
void unescapeTextField(char *data);

CPP_GUARD_END

#endif /* LOGGERAPI_H_ */
