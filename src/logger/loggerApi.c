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

#include "ADC.h"
#include "FreeRTOS.h"
#include "GPIO.h"
#include "PWM.h"
#include "bluetooth.h"
#include "capabilities.h"
#include "cellular.h"
#include "cellular_api_status_keys.h"
#include "channel_config.h"
#include "constants.h"
#include "cpu.h"
#include "dateTime.h"
#include "esp8266_drv.h"
#include "flags.h"
#include "geopoint.h"
#include "gps.h"
#include "imu.h"
#include "imu_device.h"
#include "jsmn.h"
#include "lap_stats.h"
#include "launch_control.h"
#include "logger.h"
#include "loggerApi.h"
#include "loggerConfig.h"
#include "loggerData.h"
#include "loggerHardware.h"
#include "loggerNotifications.h"
#include "loggerSampleData.h"
#include "loggerTaskEx.h"
#include "luaScript.h"
#include "luaTask.h"
#include "macros.h"
#include "mem_mang.h"
#include "printk.h"
#include "sampleRecord.h"
#include "serial.h"
#include "str_util.h"
#include "task.h"
#include "taskUtil.h"
#include "timer.h"
#include "tracks.h"
#include "units.h"
#include "wifi.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

/* Max number of PIDs that can be specified in the setOBD2Cfg message */
#define MAX_OBD2_MESSAGE_PIDS 10

typedef void (*getConfigs_func)(size_t channeId, void ** baseCfg, ChannelConfig ** channelCfg);
typedef const jsmntok_t * (*setExtField_func)(const jsmntok_t *json, const char *name, const char *value, void *cfg);
typedef int (*reInitConfig_func)(LoggerConfig *config);

static int setUnsignedCharValueIfExists(const jsmntok_t *root, const char * fieldName, unsigned char *target, unsigned char (*filter)(unsigned char))
{
    const jsmntok_t *valueNode = jsmn_find_get_node_value_prim(root, fieldName);
    if (valueNode) {
        unsigned char value = atoi(valueNode->data);
        if (filter != NULL)
            value = filter(value);
        * target = value;
    }
    return (valueNode != NULL);
}

int api_systemReset(struct Serial *serial, const jsmntok_t *json)
{
    int loader = 0;
    int reset_delay_ms = 0;
    jsmn_exists_set_val_int(json, "loader", &loader);
    jsmn_exists_set_val_int(json, "delay", &reset_delay_ms);

    if (reset_delay_ms > 0) {
        vTaskDelay(reset_delay_ms / portTICK_RATE_MS);
    }
    cpu_reset(loader);
    return API_SUCCESS_NO_RETURN;
}

int api_factoryReset(struct Serial *serial, const jsmntok_t *json)
{
        flash_default_logger_config();
        flash_default_tracks();

#if LUA_SUPPORT
        flash_default_script();
#endif

        cpu_reset(0);
        return API_SUCCESS_NO_RETURN;
}

static void rc_version_info(struct Serial *serial, const int more,
                            const char *major_name,
                            const char *minor_name,
                            const char *bf_name)
{
        const enum release_type rt = version_get_release_type();
        const char* rt_key = version_release_type_api_key(rt);

        json_int(serial, major_name, MAJOR_REV, 1);
        json_int(serial, minor_name, MINOR_REV, 1);
        json_int(serial, bf_name, BUGFIX_REV, 1);
        json_string(serial, "serial", cpu_get_serialnumber(), 1);
        json_string(serial, "git_info", version_full(), 1);
        json_string(serial, "release_type", rt_key, more);
}

int api_getVersion(struct Serial *serial, const jsmntok_t *json)
{
        json_objStart(serial);
        json_objStartString(serial,"ver");
        json_string(serial, "name", DEVICE_NAME, 1);
        json_string(serial, "fname", FRIENDLY_DEVICE_NAME, 1);
        rc_version_info(serial, 0, "major", "minor", "bugfix");
        json_objEnd(serial, 0);
        json_objEnd(serial, 0);

        return API_SUCCESS_NO_RETURN;
}

int api_getCapabilities(struct Serial *serial, const jsmntok_t *json)
{
        json_objStart(serial);
        json_objStartString(serial,"capabilities");

        /* Send all of our feature flags over the wire */
        json_arrayStart(serial, "flags");
        const char** flags = flags_get_features();
        while (*flags) {
                const char** next = flags + 1;
                json_arrayElementString(serial, *flags, !!*next);
                flags = next;
        }
        json_arrayEnd(serial, 1);

        json_objStartString(serial,"channels");
        json_int(serial, "analog", ANALOG_CHANNELS, 1);
        json_int(serial, "imu", IMU_CHANNELS, 1);
#if GPIO_CHANNELS > 0
        json_int(serial, "gpio", GPIO_CHANNELS, 1);
#endif
#if TIMER_CHANNELS > 0
        json_int(serial, "timer", TIMER_CHANNELS, 1);
#endif
#if PWM_CHANNELS > 0
        json_int(serial, "pwm", PWM_CHANNELS, 1);
#endif
        json_int(serial, "can", CAN_CHANNELS, 0);
        json_objEnd(serial, 1);

        json_objStartString(serial,"sampleRates");
        json_int(serial, "sensor", MAX_SENSOR_SAMPLE_RATE, 1);
        json_int(serial, "gps", MAX_GPS_SAMPLE_RATE, 0);
        json_objEnd(serial, 1);

        json_objStartString(serial,"db");

#if LUA_SUPPORT
        json_int(serial, "script", SCRIPT_MEMORY_LENGTH, 1);
#endif

        json_int(serial, "tracks", MAX_TRACKS, 1);
        json_int(serial, "sectors", MAX_SECTORS, 0);
        json_objEnd(serial, 0);

        json_objEnd(serial, 0);
        json_objEnd(serial, 0);
        return API_SUCCESS_NO_RETURN;
}

static void get_imu_status(struct Serial *serial, const bool more)
{
        const bool imu_init =
                imu_device_init_status() == IMU_INIT_STATUS_SUCCESS;

        json_objStartString(serial, "imu");
        json_bool(serial, "init", imu_init, false);
        json_objEnd(serial, more);
}

static void get_wifi_status(struct Serial* serial, const bool more)
{
        const LoggerConfig *lc = getWorkingLoggerConfig();

        const struct wifi_ap_cfg* ap_cfg = esp8266_drv_get_ap_config();
        const struct wifi_client_cfg* clt_cfg = esp8266_drv_get_client_config();

        const bool ap_active = ap_cfg && ap_cfg->active;
        const bool client_active = clt_cfg && clt_cfg->active;
        const bool client_connected = esp8266_drv_client_connected();
	const bool device_active = lc->ConnectivityConfigs.wifi.active;
	const bool device_init = esp8266_drv_is_initialized();

        json_objStartString(serial, "wifi");
	json_bool(serial, "active", device_active, true);
	json_bool(serial, "initialized", device_init, true);

        json_objStartString(serial, "ap");
        json_bool(serial, "active", ap_active, false);
        json_objEnd(serial, true);

        json_objStartString(serial, "client");
        json_bool(serial, "active", client_active, true);
        json_bool(serial, "connected", client_connected, false);
        json_objEnd(serial, false);

        json_objEnd(serial, more);
}

static void get_cellular_status(struct Serial* serial, const bool more)
{
#if CELLULAR_SUPPORT
	const enum cellular_net_status ns = cellmodem_get_status();
	const char* ns_val = cellular_net_status_api_key(ns);

	json_objStartString(serial, "cell");
	json_int(serial, "init", (int) ns, 1);
	json_string(serial, "IMEI", cell_get_IMEI(), 1);
	json_int(serial, "sig_str", cell_get_signal_strength(), 1);
	json_string(serial, "number", cell_get_subscriber_number(), 1);
	json_string(serial, "state", ns_val, 0);
	json_objEnd(serial, 1);

	const telemetry_status_t ts = cellular_get_connection_status();
	const char *ts_val = cellular_telemetry_status_api_key(ts);

	json_objStartString(serial, "telemetry");
	json_int(serial, "status", (int) ts, 1);
	json_string(serial, "state", ts_val, 1);
	json_int(serial, "dur", cellular_active_time(), 0);
	json_objEnd(serial, more);
#endif
}

static void get_bt_status(struct Serial* serial, const bool more)
{
#if BLUETOOTH_SUPPORT
	json_objStartString(serial, "bt");
	json_int(serial, "init", (int)bt_get_status(), 0);
	json_objEnd(serial, 1);
#endif
}

static void get_logging_status(struct Serial* serial, const bool more)
{
#if SDCARD_SUPPORT
	json_objStartString(serial, "logging");
	json_int(serial, "status", (int)logging_get_status(), 1);
	json_int(serial, "dur", logging_active_time(), 0);
	json_objEnd(serial, 1);
#endif
}

int api_getStatus(struct Serial *serial, const jsmntok_t *json)
{
	json_objStart(serial);
	json_objStartString(serial, "status");

	json_objStartString(serial, "system");
	json_string(serial, "model", FRIENDLY_DEVICE_NAME, 1);
	rc_version_info(serial, 1, "ver_major", "ver_minor", "ver_bugfix");
	json_uint(serial, "uptime", getUptimeAsInt(), 0);
	json_objEnd(serial, 1);

	json_objStartString(serial, "GPS");
	json_int(serial, "init", (int)GPS_getStatus(), 1);
	json_int(serial, "qual", GPS_getQuality(), 1);
	json_float(serial, "lat", GPS_getLatitude(),
		   DEFAULT_GPS_POSITION_PRECISION, 1);
	json_float(serial, "lon", GPS_getLongitude(),
		   DEFAULT_GPS_POSITION_PRECISION, 1);
	json_int(serial, "sats", GPS_getSatellitesUsedForPosition(), 1);
	json_int(serial, "DOP", GPS_getDOP(), 0);
	json_objEnd(serial, 1);

	get_cellular_status(serial, true);
	get_bt_status(serial, true);
	get_logging_status(serial, true);

	json_objStartString(serial, "track");
	json_int(serial, "status", lapstats_get_track_status(), 1);
	json_bool(serial, "valid", lapstats_is_track_valid(), 1);
	json_int(serial, "trackId", lapstats_get_selected_track_id(), 1);
	json_int(serial, "inLap", (int)lapstats_lap_in_progress(), 1);
	json_int(serial, "armed", lc_is_armed(), 0);
	json_objEnd(serial, true);

	get_imu_status(serial, true);
	get_wifi_status(serial, false);

	json_objEnd(serial, 0);
	json_objEnd(serial, 0);

	return API_SUCCESS_NO_RETURN;
}

int api_sampleData(struct Serial *serial, const jsmntok_t *json)
{
    int sendMeta = 0;
    if (json->type == JSMN_OBJECT && json->size == 2) {
        const jsmntok_t * meta = json + 1;
        const jsmntok_t * value = json + 2;

        jsmn_trimData(meta);
        jsmn_trimData(value);

        if (STR_EQ("meta",meta->data)) {
            sendMeta = atoi(value->data);
        }
    }

    LoggerConfig *config = getWorkingLoggerConfig();
    size_t channelCount = get_enabled_channel_count(config);

    if (0 == channelCount)
        return API_ERROR_SEVERE;

    struct sample s;
    memset(&s, 0, sizeof(struct sample));
    const size_t size = init_sample_buffer(&s, channelCount);
    if (!size)
       return API_ERROR_SEVERE;

    populate_sample_buffer(&s, 0);
    api_send_sample_record(serial, &s, 0, sendMeta);

    free_sample_buffer(&s);
    return API_SUCCESS_NO_RETURN;
}

int api_heart_beat(struct Serial *serial, const jsmntok_t *json)
{
    json_objStart(serial);
    json_int(serial, "hb", getUptimeAsInt(), 0);
    json_objEnd(serial, 0);
    return API_SUCCESS_NO_RETURN;
}

void api_sendLogStart(struct Serial *serial)
{
    json_objStart(serial);
    json_int(serial, "logStart", 1, 0);
    json_objEnd(serial, 0);
}

void api_sendLogEnd(struct Serial *serial)
{
    json_objStart(serial);
    json_int(serial, "logEnd", 1, 0);
    json_objEnd(serial, 0);
}

int api_log(struct Serial *serial, const jsmntok_t *json)
{
    if (json->type == JSMN_PRIMITIVE && json->size == 0) {
        jsmn_trimData(json);
        int doLogging = atoi(json->data);
        //TODO when RCP simulator is fully working, enable this.
#ifndef RCP_TESTING

        if (doLogging) {
            startLogging();
        } else {
            stopLogging();
        }
#else
        (void) doLogging;
#endif

    }
    return API_SUCCESS;
}

static void json_channelConfig(struct Serial *serial, ChannelConfig *cfg, int more)
{
    json_string(serial, "nm", cfg->label, 1);
    json_string(serial, "ut", cfg->units, 1);
    json_float(serial, "min", cfg->min, cfg->precision, 1);
    json_float(serial, "max", cfg->max, cfg->precision, 1);
    json_int(serial, "prec", (int) cfg->precision, 1);
    json_int(serial, "sr", decodeSampleRate(cfg->sampleRate), more);
}

static void write_sample_meta(struct Serial *serial, const struct sample *sample,
                              int sampleRateLimit, int more)
{
        json_arrayStart(serial, "meta");
        ChannelSample *channel_sample = sample->channel_samples;

        for (size_t i = 0; i < sample->channel_count; ++i, ++channel_sample) {
                if (0 < i)
                        serial_write_c(serial, ',');

                serial_write_c(serial, '{');
                json_channelConfig(serial, channel_sample->cfg, 0);
                serial_write_c(serial, '}');
        }

        json_arrayEnd(serial, more);
}

int api_getMeta(struct Serial *serial, const jsmntok_t *json)
{
    json_objStart(serial);

    LoggerConfig * config = getWorkingLoggerConfig();
    const size_t channelCount = get_enabled_channel_count(config);

    if (0 == channelCount)
        return API_ERROR_SEVERE;

    struct sample s;
    memset(&s, 0, sizeof(struct sample));
    const size_t size = init_sample_buffer(&s, channelCount);
    if (!size)
       return API_ERROR_SEVERE;

    write_sample_meta(serial, &s, getConnectivitySampleRateLimit(), 0);

    free_sample_buffer(&s);
    json_objEnd(serial, 0);
    return API_SUCCESS_NO_RETURN;
}


#define MAX_BITMAPS 10

void api_send_sample_record(struct Serial *serial,
                            const struct sample *sample,
                            const unsigned int tick, const int sendMeta)
{
        json_objStart(serial);
        json_objStartString(serial, "s");
        json_uint(serial,"t", tick, 1);

        if (sendMeta)
                write_sample_meta(serial, sample,
                                  getConnectivitySampleRateLimit(), 1);

        size_t channelBitmaskIndex = 0;
        unsigned int channelBitmask[MAX_BITMAPS];
        memset(channelBitmask, 0, sizeof(channelBitmask));

        json_arrayStart(serial, "d");
        ChannelSample *cs = sample->channel_samples;

        size_t channelBitPosition = 0;
        for (size_t i = 0; i < sample->channel_count;
             i++, channelBitPosition++, cs++) {

                if (channelBitPosition > 31) {
                        channelBitmaskIndex++;
                        channelBitPosition=0;
                        if (channelBitmaskIndex > MAX_BITMAPS)
                                break;
                }

                if (cs->populated) {
                        channelBitmask[channelBitmaskIndex] |=
                                (1 << channelBitPosition);

                        const int precision = cs->cfg->precision;
                        switch(cs->sampleData) {
                        case SampleData_Float:
                        case SampleData_Float_Noarg:
                                put_float(serial, cs->valueFloat, precision);
                                break;
                        case SampleData_Int:
                        case SampleData_Int_Noarg:
                                put_int(serial, cs->valueInt);
                                break;
                        case SampleData_LongLong:
                        case SampleData_LongLong_Noarg:
                                put_ll(serial, cs->valueLongLong);
                                break;
                        case SampleData_Double:
                        case SampleData_Double_Noarg:
                                put_double(serial, cs->valueDouble, precision);
                                break;
                        default:
                                pr_warning_int_msg("[loggerApi] Unknown sample"
                                                   " data type: ",
                                                   cs->sampleData);
                                break;
                        }
                        serial_write_c(serial, ',');
                }
        }

        size_t channelBitmaskCount = channelBitmaskIndex + 1;
        for (size_t i = 0; i < channelBitmaskCount; i++) {
                put_uint(serial, channelBitmask[i]);
                if (i < channelBitmaskCount - 1)
                        serial_write_c(serial, ',');
        }

        json_arrayEnd(serial, 0);
        json_objEnd(serial, 0);
        json_objEnd(serial, 0);
}

static const jsmntok_t * setChannelConfig(struct Serial *serial, const jsmntok_t *cfg,
        ChannelConfig *channelCfg,
        setExtField_func setExtField,
        void *extCfg)
{

    if (cfg->type != JSMN_OBJECT || cfg->size % 2 != 0)
        return cfg;

    int size = cfg->size;
    cfg++;

    for (int i = 0; i < size; i += 2 ) {
        const jsmntok_t *nameTok = cfg;
        jsmn_trimData(nameTok);
        cfg++;

        const jsmntok_t *valueTok = cfg;
        cfg++;

        if (valueTok->type == JSMN_PRIMITIVE || valueTok->type == JSMN_STRING)
            jsmn_trimData(valueTok);

        char *name = nameTok->data;
        char *value = valueTok->data;

        if (STR_EQ("nm", name))
		jsmn_decode_string(channelCfg->label, value, DEFAULT_LABEL_LENGTH);
        else if (STR_EQ("ut", name))
		jsmn_decode_string(channelCfg->units, value, DEFAULT_UNITS_LENGTH);
        else if (STR_EQ("min", name))
		channelCfg->min = atof(value);
        else if (STR_EQ("max", name))
		channelCfg->max = atof(value);
        else if (STR_EQ("sr", name))
		channelCfg->sampleRate = encodeSampleRate(atoi(value));
        else if (STR_EQ("prec", name))
		channelCfg->precision = (unsigned char) atoi(value);
        else if (setExtField != NULL)
		cfg = setExtField(valueTok, name, value, extCfg);
    }

    return cfg;
}

static int setMultiChannelConfigGeneric(struct Serial *serial, const jsmntok_t * json,
                                        getConfigs_func getConfigsFunc,
                                        setExtField_func setExtFieldFunc,
                                        reInitConfig_func reInitConfigFunc)
{
    if (json->type == JSMN_OBJECT && json->size % 2 == 0) {
        for (int i = 1; i <= json->size; i += 2) {
            const jsmntok_t *idTok = json + i;
            const jsmntok_t *cfgTok = json + i + 1;
            jsmn_trimData(idTok);
            size_t id = atoi(idTok->data);
            void *baseCfg = NULL;
            ChannelConfig *channelCfg = NULL;
            getConfigsFunc(id, &baseCfg, &channelCfg);
            if (channelCfg && baseCfg) {
                setChannelConfig(serial, cfgTok, channelCfg, setExtFieldFunc, baseCfg);
            } else {
                return API_ERROR_PARAMETER;
            }
        }
    }
    configChanged();
    int initRes = reInitConfigFunc(getWorkingLoggerConfig());
    return (initRes ? API_SUCCESS : API_ERROR_SEVERE);
}

static const jsmntok_t * setScalingMapRaw(ADCConfig *adcCfg, const jsmntok_t *mapArrayTok)
{
    if (mapArrayTok->type == JSMN_ARRAY) {
        int size = mapArrayTok->size;
        for (int i = 0; i < size; i++) {
            mapArrayTok++;
            if (mapArrayTok->type == JSMN_PRIMITIVE) {
                jsmn_trimData(mapArrayTok);
                if (i < ANALOG_SCALING_BINS) {
                    adcCfg->scalingMap.rawValues[i] = atof(mapArrayTok->data);
                }
            }
        }
    }
    return mapArrayTok + 1;
}

static const jsmntok_t * setScalingMapValues(ADCConfig *adcCfg, const jsmntok_t *mapArrayTok)
{
    if (mapArrayTok->type == JSMN_ARRAY) {
        int size = mapArrayTok->size;
        for (int i = 0; i < size; i++) {
            mapArrayTok++;
            if (mapArrayTok->type == JSMN_PRIMITIVE) {
                jsmn_trimData(mapArrayTok);
                if (i < ANALOG_SCALING_BINS) {
                    adcCfg->scalingMap.scaledValues[i] = atof(mapArrayTok->data);
                }
            }
        }
    }
    return mapArrayTok + 1;
}

static const jsmntok_t * setScalingRow(ADCConfig *adcCfg, const jsmntok_t *mapRow)
{
    if (mapRow->type == JSMN_STRING) {
        jsmn_trimData(mapRow);
        if (STR_EQ(mapRow->data, "raw"))
            mapRow = setScalingMapRaw(adcCfg, mapRow + 1);
        else if (STR_EQ("scal", mapRow->data))
            mapRow = setScalingMapValues(adcCfg, mapRow + 1);
    } else {
        mapRow++;
    }
    return mapRow;
}

static const jsmntok_t * setAnalogExtendedField(const jsmntok_t *valueTok, const char *name, const char *value, void *cfg)
{
    ADCConfig *adcCfg = (ADCConfig *)cfg;
    if (STR_EQ("scalMod", name))
        adcCfg->scalingMode = filterAnalogScalingMode(atoi(value));
    else if (STR_EQ("scaling", name))
        adcCfg->linearScaling = atof(value);
    else if (STR_EQ("offset", name))
        adcCfg->linearOffset = atof(value);
    else if (STR_EQ("alpha", name))
        adcCfg->filterAlpha = atof(value);
    else if (STR_EQ("cal", name))
        adcCfg->calibration = atof(value);
    else if (STR_EQ("map", name)) {
        if (valueTok->type == JSMN_OBJECT) {
            valueTok++;
            valueTok = setScalingRow(adcCfg, valueTok);
            valueTok = setScalingRow(adcCfg, valueTok);
            valueTok--;
        }
    }
    return valueTok + 1;
}

static void getAnalogConfigs(size_t channelId, void ** baseCfg, ChannelConfig ** channelCfg)
{
    if (channelId < ANALOG_CHANNELS) {
        ADCConfig *c =&(getWorkingLoggerConfig()->ADCConfigs[channelId]);
        *baseCfg = c;
        *channelCfg = &c->cfg;
    }
}

int api_setAnalogConfig(struct Serial *serial, const jsmntok_t * json)
{
    int res = setMultiChannelConfigGeneric(serial, json, getAnalogConfigs, setAnalogExtendedField, ADC_init);
    return res;
}

static void sendAnalogConfig(struct Serial *serial, size_t startIndex, size_t endIndex)
{

    json_objStart(serial);
    json_objStartString(serial, "analogCfg");
    for (size_t i = startIndex; i <= endIndex; i++) {

        ADCConfig *adcCfg = &(getWorkingLoggerConfig()->ADCConfigs[i]);
        json_objStartInt(serial, i);
        json_channelConfig(serial, &(adcCfg->cfg), 1);
        json_int(serial, "scalMod", adcCfg->scalingMode, 1);
        json_float(serial, "scaling", adcCfg->linearScaling, LINEAR_SCALING_PRECISION, 1);
        json_float(serial, "offset", adcCfg->linearOffset, LINEAR_SCALING_PRECISION, 1);
        json_float(serial, "alpha", adcCfg->filterAlpha, FILTER_ALPHA_PRECISION, 1);
        json_float(serial, "cal", adcCfg->calibration, LINEAR_SCALING_PRECISION, 1);

        json_objStartString(serial, "map");
        json_arrayStart(serial, "raw");

        for (size_t b = 0; b < ANALOG_SCALING_BINS; b++) {
            put_float(serial,  adcCfg->scalingMap.rawValues[b], SCALING_MAP_BIN_PRECISION);
            if (b < ANALOG_SCALING_BINS - 1)
                serial_write_c(serial, ',');
        }

        json_arrayEnd(serial, 1);
        json_arrayStart(serial, "scal");

        for (size_t b = 0; b < ANALOG_SCALING_BINS; b++) {
            put_float(serial, adcCfg->scalingMap.scaledValues[b], DEFAULT_ANALOG_SCALING_PRECISION);
            if (b < ANALOG_SCALING_BINS - 1)
                serial_write_c(serial, ',');
        }

        json_arrayEnd(serial, 0);
        json_objEnd(serial, 0); //map
        json_objEnd(serial, i != endIndex); //index
    }
    json_objEnd(serial, 0);
    json_objEnd(serial, 0);
}

int api_getAnalogConfig(struct Serial *serial, const jsmntok_t * json)
{
    size_t startIndex = 0;
    size_t endIndex = 0;
    if (json->type == JSMN_PRIMITIVE) {
        if (jsmn_isNull(json)) {
            startIndex = 0;
            endIndex = CONFIG_ADC_CHANNELS - 1;
        } else {
            jsmn_trimData(json);
            startIndex = endIndex = atoi(json->data);
        }
    }
    if (startIndex <= CONFIG_ADC_CHANNELS) {
        sendAnalogConfig(serial, startIndex, endIndex);
        return API_SUCCESS_NO_RETURN;
    } else {
        return API_ERROR_PARAMETER;
    }
}

static const jsmntok_t * setImuExtendedField(const jsmntok_t *valueTok, const char *name, const char *value, void *cfg)
{
    ImuConfig *imuCfg = (ImuConfig *)cfg;

    if (STR_EQ("mode",name))
        imuCfg->mode = filterImuMode(atoi(value));
    else if (STR_EQ("chan",name))
        imuCfg->physicalChannel = filterImuChannel(atoi(value));
    else if (STR_EQ("zeroVal",name))
        imuCfg->zeroValue = atoi(value);
    else if (STR_EQ("alpha", name))
        imuCfg->filterAlpha = atof(value);
    return valueTok + 1;
}

static void getImuConfigs(size_t channelId, void ** baseCfg, ChannelConfig ** channelCfg)
{
    if (channelId < IMU_CHANNELS) {
        ImuConfig *c = &(getWorkingLoggerConfig()->ImuConfigs[channelId]);
        *baseCfg = c;
        *channelCfg = &c->cfg;
    }
}

int api_setImuConfig(struct Serial *serial, const jsmntok_t *json)
{
    int res = setMultiChannelConfigGeneric(serial, json, getImuConfigs, setImuExtendedField, imu_soft_init);
    return res;
}

static void sendImuConfig(struct Serial *serial, size_t startIndex, size_t endIndex)
{
    json_objStart(serial);
    json_objStartString(serial, "imuCfg");
    for (size_t i = startIndex; i <= endIndex; i++) {
        ImuConfig *cfg = &(getWorkingLoggerConfig()->ImuConfigs[i]);
        json_objStartInt(serial, i);
        json_channelConfig(serial, &(cfg->cfg), 1);
        json_uint(serial, "mode", cfg->mode, 1);
        json_uint(serial, "chan", cfg->physicalChannel, 1);
        json_int(serial, "zeroVal", cfg->zeroValue, 1);
        json_float(serial, "alpha", cfg->filterAlpha, FILTER_ALPHA_PRECISION, 0 );
        json_objEnd(serial, i != endIndex); //index
    }
    json_objEnd(serial, 0);
    json_objEnd(serial, 0);
}

int api_getImuConfig(struct Serial *serial, const jsmntok_t *json)
{
    size_t startIndex = 0;
    size_t endIndex = 0;
    if (json->type == JSMN_PRIMITIVE) {
        if (jsmn_isNull(json)) {
            startIndex = 0;
            endIndex = CONFIG_IMU_CHANNELS - 1;
        } else {
            jsmn_trimData(json);
            startIndex = endIndex = atoi(json->data);
        }
    }
    if (startIndex <= CONFIG_IMU_CHANNELS) {
        sendImuConfig(serial, startIndex, endIndex);
        return API_SUCCESS_NO_RETURN;
    } else {
        return API_ERROR_PARAMETER;
    }
}

#ifdef FALSE
// DELETE ME after June 1, 2014 if not used.
static void setConfigGeneric(struct Serial *serial, const jsmntok_t * json, void *cfg, setExtField_func setExtField)
{
    int size = json->size;
    if (json->type == JSMN_OBJECT && json->size % 2 == 0) {
        json++;
        for (int i = 0; i < size; i += 2 ) {
            const jsmntok_t *nameTok = json;
            jsmn_trimData(nameTok);
            json++;
            const jsmntok_t *valueTok = json;
            json++;
            if (valueTok->type == JSMN_PRIMITIVE || valueTok->type == JSMN_STRING)
                jsmn_trimData(valueTok);

            const char *name = nameTok->data;
            const char *value = valueTok->data;

            setExtField(valueTok, name, value, cfg);
        }
    }

}
#endif

int api_getCellConfig(struct Serial *serial, const jsmntok_t *json)
{
    CellularConfig *cfg = &(getWorkingLoggerConfig()->ConnectivityConfigs.cellularConfig);
    json_objStart(serial);
    json_objStartString(serial, "cellCfg");
    json_string(serial, "apnHost", cfg->apnHost, 1);
    json_string(serial, "apnUser", cfg->apnUser, 1);
    json_string(serial, "apnPass", cfg->apnPass, 0);
    json_objEnd(serial, 0);
    json_objEnd(serial, 0);
    return API_SUCCESS_NO_RETURN;
}

int api_getLogfile(struct Serial *serial, const jsmntok_t *json)
{
    json_objStart(serial);
    json_valueStart(serial, "logfile");
    serial_write_c(serial, '"');
    read_log_to_serial(serial, 1);
    serial_write_c(serial, '"');
    json_objEnd(serial,0);
    return API_SUCCESS_NO_RETURN;
}

int api_setLogfileLevel(struct Serial *serial, const jsmntok_t *json)
{
    int level;
    if (jsmn_exists_set_val_int(json, "level", &level)) {
        set_log_level((enum log_level) level);
        return API_SUCCESS;
    } else {
        return API_ERROR_PARAMETER;
    }
}

static void setCellConfig(const jsmntok_t *root)
{
    const jsmntok_t *cellCfgNode = jsmn_find_node(root, "cellCfg");
    if (cellCfgNode) {
        CellularConfig *cellCfg = &(getWorkingLoggerConfig()->ConnectivityConfigs.cellularConfig);
        cellCfgNode++;
        setUnsignedCharValueIfExists(cellCfgNode, "cellEn", &cellCfg->cellEnabled, NULL);
        jsmn_exists_set_val_string(cellCfgNode, "apnHost", cellCfg->apnHost,
				   CELL_APN_HOST_LENGTH, true);
        jsmn_exists_set_val_string(cellCfgNode, "apnUser", cellCfg->apnUser,
				   CELL_APN_USER_LENGTH, true);
        jsmn_exists_set_val_string(cellCfgNode, "apnPass", cellCfg->apnPass,
				   CELL_APN_PASS_LENGTH, false);
    }
}

static void setBluetoothConfig(const jsmntok_t *root)
{
    const jsmntok_t *btCfgNode = jsmn_find_node(root, "btCfg");
    if (btCfgNode != NULL) {
        btCfgNode++;
        BluetoothConfig *btCfg = &(getWorkingLoggerConfig()->ConnectivityConfigs.bluetoothConfig);
        setUnsignedCharValueIfExists(btCfgNode, "btEn", &btCfg->btEnabled, NULL);
        jsmn_exists_set_val_string(btCfgNode, "name", btCfg->new_name,
				   BT_DEVICE_NAME_LENGTH, true);
        jsmn_exists_set_val_string(btCfgNode, "pass", btCfg->new_pin,
				   BT_PASSCODE_LENGTH, false);
    }
}

static void setTelemetryConfig(const jsmntok_t *root)
{
    const jsmntok_t *telemetryCfgNode = jsmn_find_node(root, "telCfg");
    if (telemetryCfgNode) {
        telemetryCfgNode++;
        TelemetryConfig *telemetryCfg = &(getWorkingLoggerConfig()->ConnectivityConfigs.telemetryConfig);
        jsmn_exists_set_val_string(telemetryCfgNode, "deviceId",
				   telemetryCfg->telemetryDeviceId,
				   DEVICE_ID_LENGTH, true);
        jsmn_exists_set_val_string(telemetryCfgNode, "host",
				   telemetryCfg->telemetryServerHost,
				   TELEMETRY_SERVER_HOST_LENGTH, true);
        setUnsignedCharValueIfExists(telemetryCfgNode, "bgStream",
				     &telemetryCfg->backgroundStreaming,
				     filterBgStreamingMode);
    }
}

int api_setConnectivityConfig(struct Serial *serial, const jsmntok_t *json)
{
    setBluetoothConfig(json);
    setCellConfig(json);
    setTelemetryConfig(json);
    configChanged();
    return API_SUCCESS;
}

int api_getConnectivityConfig(struct Serial *serial, const jsmntok_t *json)
{
    ConnectivityConfig *cfg = &(getWorkingLoggerConfig()->ConnectivityConfigs);
    json_objStart(serial);
    json_objStartString(serial, "connCfg");

    json_objStartString(serial, "btCfg");
    json_int(serial, "btEn", cfg->bluetoothConfig.btEnabled, 1);
    /* Remove Name and Pass in next major API version change.  Issue #720 */
    json_string(serial, "name", "", 1);
    json_string(serial, "pass", "", 0);
    json_objEnd(serial, 1);

    json_objStartString(serial, "cellCfg");
    json_int(serial, "cellEn", cfg->cellularConfig.cellEnabled, 1);
    json_string(serial, "apnHost", cfg->cellularConfig.apnHost, 1);
    json_string(serial, "apnUser", cfg->cellularConfig.apnUser, 1);
    json_string(serial, "apnPass", cfg->cellularConfig.apnPass, 0);
    json_objEnd(serial, 1);

    json_objStartString(serial, "telCfg");
    json_int(serial, "bgStream", cfg->telemetryConfig.backgroundStreaming, 1);
    json_string(serial, "deviceId", cfg->telemetryConfig.telemetryDeviceId, 1);
    json_string(serial, "host", cfg->telemetryConfig.telemetryServerHost, 0);
    json_objEnd(serial, 0);

    json_objEnd(serial, 0);
    json_objEnd(serial, 0);
    return API_SUCCESS_NO_RETURN;
}

#if PWM_CHANNELS > 0
static void sendPwmConfig(struct Serial *serial, size_t startIndex, size_t endIndex)
{

    json_objStart(serial);
    json_objStartString(serial, "pwmCfg");
    for (size_t i = startIndex; i <= endIndex; i++) {
        PWMConfig *cfg = &(getWorkingLoggerConfig()->PWMConfigs[i]);
        json_objStartInt(serial, i);
        json_channelConfig(serial, &(cfg->cfg), 1);
        json_uint(serial, "outMode", cfg->outputMode, 1);
        json_uint(serial, "logMode", cfg->loggingMode, 1);
        json_uint(serial, "stDutyCyc", cfg->startupDutyCycle, 1);
        json_uint(serial, "stPeriod", cfg->startupPeriod, 0);
        json_objEnd(serial, i != endIndex); //index
    }
    json_objEnd(serial, 0);
    json_objEnd(serial, 0);
}


int api_getPwmConfig(struct Serial *serial, const jsmntok_t *json)
{
    size_t startIndex = 0;
    size_t endIndex = 0;
    if (json->type == JSMN_PRIMITIVE) {
        if (jsmn_isNull(json)) {
            startIndex = 0;
            endIndex = CONFIG_PWM_CHANNELS - 1;
        } else {
            jsmn_trimData(json);
            startIndex = endIndex = atoi(json->data);
        }
    }
    if (startIndex <= CONFIG_PWM_CHANNELS) {
        sendPwmConfig(serial, startIndex, endIndex);
        return API_SUCCESS_NO_RETURN;
    } else {
        return API_ERROR_PARAMETER;
    }
}

static void getPwmConfigs(size_t channelId, void ** baseCfg, ChannelConfig ** channelCfg)
{
    if (channelId < PWM_CHANNELS) {
        PWMConfig *c =&(getWorkingLoggerConfig()->PWMConfigs[channelId]);
        *baseCfg = c;
        *channelCfg = &c->cfg;
    }
}

static const jsmntok_t * setPwmExtendedField(const jsmntok_t *valueTok, const char *name, const char *value, void *cfg)
{
    PWMConfig *pwmCfg = (PWMConfig *)cfg;

    if (STR_EQ("outMode", name))
        pwmCfg->outputMode = filterPwmOutputMode(atoi(value));
    if (STR_EQ("logMode", name))
        pwmCfg->loggingMode = filterPwmLoggingMode(atoi(value));
    if (STR_EQ("stDutyCyc", name))
        pwmCfg->startupDutyCycle = filterPwmDutyCycle(atoi(value));
    if (STR_EQ("stPeriod", name))
        pwmCfg->startupPeriod = filterPwmPeriod(atoi(value));
    return valueTok + 1;
}

int api_setPwmConfig(struct Serial *serial, const jsmntok_t *json)
{
    int res = setMultiChannelConfigGeneric(serial, json, getPwmConfigs, setPwmExtendedField, PWM_update_config);
    return res;
}
#endif

#if GPIO_CHANNELS > 0
static void getGpioConfigs(size_t channelId, void ** baseCfg, ChannelConfig ** channelCfg)
{
    if (channelId < GPIO_CHANNELS) {
        GPIOConfig *c =&(getWorkingLoggerConfig()->GPIOConfigs[channelId]);
        *baseCfg = c;
        *channelCfg = &c->cfg;
    }
}

static const jsmntok_t * setGpioExtendedField(const jsmntok_t *valueTok, const char *name, const char *value, void *cfg)
{
    GPIOConfig *gpioCfg = (GPIOConfig *)cfg;

    if (STR_EQ("mode", name))
        gpioCfg->mode = filterGpioMode(atoi(value));
    return valueTok + 1;
}

static void sendGpioConfig(struct Serial *serial, size_t startIndex, size_t endIndex)
{
    json_objStart(serial);
    json_objStartString(serial, "gpioCfg");
    for (size_t i = startIndex; i <= endIndex; i++) {
        GPIOConfig *cfg = &(getWorkingLoggerConfig()->GPIOConfigs[i]);
        json_objStartInt(serial, i);
        json_channelConfig(serial, &(cfg->cfg), 1);
        json_uint(serial, "mode", cfg->mode, 0);
        json_objEnd(serial, i != endIndex);
    }
    json_objEnd(serial, 0);
    json_objEnd(serial, 0);
}


int api_getGpioConfig(struct Serial *serial, const jsmntok_t *json)
{
    size_t startIndex = 0;
    size_t endIndex = 0;
    if (json->type == JSMN_PRIMITIVE) {
        if (jsmn_isNull(json)) {
            startIndex = 0;
            endIndex = CONFIG_GPIO_CHANNELS - 1;
        } else {
            jsmn_trimData(json);
            startIndex = endIndex = atoi(json->data);
        }
    }
    if (startIndex <= CONFIG_GPIO_CHANNELS) {
        sendGpioConfig(serial, startIndex, endIndex);
        return API_SUCCESS_NO_RETURN;
    } else {
        return API_ERROR_PARAMETER;
    }
}

int api_setGpioConfig(struct Serial *serial, const jsmntok_t *json)
{
    int res = setMultiChannelConfigGeneric(serial, json, getGpioConfigs, setGpioExtendedField, GPIO_init);
    return res;
}
#endif

#if TIMER_CHANNELS > 0
static void getTimerConfigs(size_t channelId, void ** baseCfg, ChannelConfig ** channelCfg)
{
    if (channelId < TIMER_CHANNELS) {
        TimerConfig *c =&(getWorkingLoggerConfig()->TimerConfigs[channelId]);
        *baseCfg = c;
        *channelCfg = &c->cfg;
    }
}

static const jsmntok_t * setTimerExtendedField(const jsmntok_t *valueTok,
                                               const char *name,
                                               const char *value,
                                               void *cfg)
{
    TimerConfig *timerCfg = (TimerConfig *)cfg;

    if (STR_EQ("mode", name))
        timerCfg->mode = filterTimerMode(atoi(value));
    if (STR_EQ("alpha", name))
        timerCfg->filterAlpha = atof(value);
    if (STR_EQ("ppr", name))
        timerCfg->pulsePerRevolution = atof(value);
    if (STR_EQ("speed", name))
        timerCfg->timerSpeed = filterTimerDivider(atoi(value));
    if (STR_EQ("filter_period", name))
            timerCfg->filter_period_us = atoi(value);
    if (STR_EQ("edge", name))
            timerCfg->edge = get_timer_edge_enum(value);

    return valueTok + 1;
}

static void sendTimerConfig(struct Serial *serial, size_t startIndex, size_t endIndex)
{
    json_objStart(serial);
    json_objStartString(serial, "timerCfg");
    for (size_t i = startIndex; i <= endIndex; i++) {
        TimerConfig *cfg = &(getWorkingLoggerConfig()->TimerConfigs[i]);
        json_objStartInt(serial, i);
        json_channelConfig(serial, &(cfg->cfg), 1);
         /* DEPRECATED.  Only here for compatibility */
        json_uint(serial, "st", 0, 1);
        json_uint(serial, "mode", cfg->mode, 1);
        json_float(serial, "alpha", cfg->filterAlpha, FILTER_ALPHA_PRECISION, 1);
        json_float(serial, "ppr", cfg->pulsePerRevolution, 6, 1);
        json_uint(serial, "speed", cfg->timerSpeed, 1);
        json_int(serial, "filter_period", cfg->filter_period_us, 1);
        json_string(serial, "edge", get_timer_edge_api_key(cfg->edge), 0);
        json_objEnd(serial, i != endIndex);
    }
    json_objEnd(serial, 0);
    json_objEnd(serial, 0);
}

int api_getTimerConfig(struct Serial *serial, const jsmntok_t *json)
{
    size_t startIndex = 0;
    size_t endIndex = 0;
    if (json->type == JSMN_PRIMITIVE) {
        if (jsmn_isNull(json)) {
            startIndex = 0;
            endIndex = CONFIG_TIMER_CHANNELS - 1;
        } else {
            jsmn_trimData(json);
            startIndex = endIndex = atoi(json->data);
        }
    }
    if (startIndex <= CONFIG_TIMER_CHANNELS) {
        sendTimerConfig(serial, startIndex, endIndex);
        return API_SUCCESS_NO_RETURN;
    } else {
        return API_ERROR_PARAMETER;
    }
}

int api_setTimerConfig(struct Serial *serial, const jsmntok_t *json)
{
    int res = setMultiChannelConfigGeneric(serial, json, getTimerConfigs, setTimerExtendedField, timer_init);
    return res;
}
#endif

static unsigned short getGpsConfigHighSampleRate(GPSConfig *cfg)
{
    unsigned short rate = SAMPLE_DISABLED;

    rate = getHigherSampleRate(rate, cfg->latitude.sampleRate);
    rate = getHigherSampleRate(rate, cfg->longitude.sampleRate);
    rate = getHigherSampleRate(rate, cfg->speed.sampleRate);
    rate = getHigherSampleRate(rate, cfg->distance.sampleRate);
    rate = getHigherSampleRate(rate, cfg->altitude.sampleRate);
    rate = getHigherSampleRate(rate, cfg->satellites.sampleRate);
    rate = getHigherSampleRate(rate, cfg->quality.sampleRate);
    rate = getHigherSampleRate(rate, cfg->DOP.sampleRate);
    return rate;
}

int api_getGpsConfig(struct Serial *serial, const jsmntok_t *json)
{

    GPSConfig *gpsCfg = &(getWorkingLoggerConfig()->GPSConfigs);

    json_objStart(serial);
    json_objStartString(serial, "gpsCfg");

    unsigned short highestRate = getGpsConfigHighSampleRate(gpsCfg);
    json_int(serial, "sr", decodeSampleRate(highestRate), 1);

    const int posEnabled = gpsCfg->latitude.sampleRate != SAMPLE_DISABLED &&
                           gpsCfg->longitude.sampleRate != SAMPLE_DISABLED;
    json_int(serial, "pos",  posEnabled, 1);
    json_int(serial, "speed", gpsCfg->speed.sampleRate != SAMPLE_DISABLED, 1);
    json_int(serial, "dist", gpsCfg->distance.sampleRate != SAMPLE_DISABLED, 1);
    json_int(serial, "alt", gpsCfg->altitude.sampleRate != SAMPLE_DISABLED, 1);
    json_int(serial, "sats", gpsCfg->satellites.sampleRate != SAMPLE_DISABLED, 1);
    json_int(serial, "qual", gpsCfg->quality.sampleRate != SAMPLE_DISABLED, 1);
    json_int(serial, "dop", gpsCfg->DOP.sampleRate != SAMPLE_DISABLED, 1);

    json_objStartString(serial, "units");
    json_string(serial, "alt", gpsCfg->altitude.units, 1);
    json_string(serial, "dist", gpsCfg->distance.units, 1);
    json_string(serial, "speed", gpsCfg->speed.units, 0);
    json_objEnd(serial, 0);

    json_objEnd(serial, 0);
    json_objEnd(serial, 0);
    return API_SUCCESS_NO_RETURN;
}

/**
 * Sets alternate unit configuration or default values based on what users
 * provide.  This will never fail, but will fall back to using default
 * values when an invalid unit is given.
 * @see https://github.com/autosportlabs/RaceCapture-Pro_firmware/issues/734
 */
static void gps_set_units(const jsmntok_t *json, GPSConfig *cfg)
{
	jsmn_exists_set_val_string(json, "alt", &cfg->altitude.units,
				   DEFAULT_UNITS_LENGTH, true);
	jsmn_exists_set_val_string(json, "dist", &cfg->distance.units,
				   DEFAULT_UNITS_LENGTH, true);
	jsmn_exists_set_val_string(json, "speed", &cfg->speed.units,
				   DEFAULT_UNITS_LENGTH, true);

	/* Altitude supports only Meters or Feet */
	if (UNIT_LENGTH_METERS != units_get_unit(cfg->altitude.units))
		strcpy(cfg->altitude.units,
		       units_get_label(UNIT_LENGTH_FEET));

	/* Distance supports only Kilometers or Miles */
	if (UNIT_LENGTH_KILOMETERS != units_get_unit(cfg->distance.units))
		strcpy(cfg->distance.units,
		       units_get_label(UNIT_LENGTH_MILES));

	/* Speed supports only Kilometers/Hr or Miles/Hr */
	if (UNIT_SPEED_KILOMETERS_HOUR != units_get_unit(cfg->speed.units))
		strcpy(cfg->speed.units,
		       units_get_label(UNIT_SPEED_MILES_HOUR));
}

static void gpsConfigTestAndSet(const jsmntok_t *json, ChannelConfig *cfg,
                                const char *str, const unsigned short sr)
{
	unsigned char test = 0;
	setUnsignedCharValueIfExists(json, str, &test, NULL);
	cfg->sampleRate = test == 0 ? SAMPLE_DISABLED : sr;
}

int api_setGpsConfig(struct Serial *serial, const jsmntok_t *json)
{
	GPSConfig *gpsCfg = &(getWorkingLoggerConfig()->GPSConfigs);

	unsigned short sr = SAMPLE_DISABLED;
	int tmp = 0;
	if (jsmn_exists_set_val_int(json, "sr", &tmp))
		sr = encodeSampleRate(tmp);

	gpsConfigTestAndSet(json, &(gpsCfg->latitude), "pos", sr);
	gpsConfigTestAndSet(json, &(gpsCfg->longitude), "pos", sr);
	gpsConfigTestAndSet(json, &(gpsCfg->speed), "speed", sr);
	gpsConfigTestAndSet(json, &(gpsCfg->distance), "dist", sr);
	gpsConfigTestAndSet(json, &(gpsCfg->altitude), "alt", sr);
	gpsConfigTestAndSet(json, &(gpsCfg->satellites), "sats", sr);
	gpsConfigTestAndSet(json, &(gpsCfg->quality), "qual", sr);
	gpsConfigTestAndSet(json, &(gpsCfg->DOP), "dop", sr);

	const jsmntok_t *units_tok = jsmn_find_node(json, "units");
	if (units_tok)
		gps_set_units(units_tok, gpsCfg);

	configChanged();
	return API_SUCCESS;
}

int api_getCanConfig(struct Serial *serial, const jsmntok_t *json)
{

    CANConfig *canCfg = &getWorkingLoggerConfig()->CanConfig;
    json_objStart(serial);
    json_objStartString(serial, "canCfg");
    json_int(serial, "en", canCfg->enabled, 1);
    json_arrayStart(serial, "baud");
    for (size_t i = 0; i < CONFIG_CAN_CHANNELS; i++) {
        json_arrayElementInt(serial, canCfg->baud[i], i < CONFIG_CAN_CHANNELS - 1);
    }
    json_arrayEnd(serial, 0);
    json_objEnd(serial, 0);
    json_objEnd(serial, 0);

    return API_SUCCESS_NO_RETURN;
}

int api_setCanConfig(struct Serial *serial, const jsmntok_t *json)
{

    CANConfig *canCfg = &getWorkingLoggerConfig()->CanConfig;
    setUnsignedCharValueIfExists( json, "en", &canCfg->enabled, NULL);

    const jsmntok_t *baudTok = jsmn_find_node(json, "baud");
    if (baudTok != NULL && (++baudTok)->type == JSMN_ARRAY) {
        size_t arrSize = json->size;
        if (arrSize > CONFIG_CAN_CHANNELS)
            arrSize = CONFIG_CAN_CHANNELS;
        size_t can_index = 0;
        for (baudTok++; can_index < arrSize; can_index++, baudTok++) {
            canCfg->baud[can_index] = atoi(baudTok->data);
        }
    }
    return API_SUCCESS;
}

int api_getObd2Config(struct Serial *serial, const jsmntok_t *json)
{
    json_objStart(serial);
    json_objStartString(serial, "obd2Cfg");

    OBD2Config *obd2Cfg = &(getWorkingLoggerConfig()->OBD2Configs);

    int enabledPids = obd2Cfg->enabledPids;
    json_int(serial,"en", obd2Cfg->enabled, 1);
    json_arrayStart(serial, "pids");

    for (int i = 0; i < enabledPids; i++) {
        PidConfig *pidCfg = &obd2Cfg->pids[i];
        json_objStart(serial);
        json_channelConfig(serial, &(pidCfg->cfg), 1);
        json_int(serial,"pid",pidCfg->pid, 0);
        json_objEnd(serial, i < enabledPids - 1);
    }

    json_arrayEnd(serial, 0);
    json_objEnd(serial,0);
    json_objEnd(serial, 0);
    return API_SUCCESS_NO_RETURN;
}

static const jsmntok_t * setPidExtendedField(const jsmntok_t *valueTok, const char *name,
        const char *value, void *cfg)
{
    PidConfig *pidCfg = (PidConfig *) cfg;

    if (STR_EQ("pid", name))
        pidCfg->pid = (unsigned short) atoi(value);

    return valueTok + 1;
}

int api_setObd2Config(struct Serial *serial, const jsmntok_t *json)
{
    OBD2Config *obd2Cfg = &(getWorkingLoggerConfig()->OBD2Configs);

    int pidIndex = 0;
    jsmn_exists_set_val_int(json, "index", &pidIndex);

    if (pidIndex >= OBD2_CHANNELS) {
        return API_ERROR_PARAMETER;
    }

    const jsmntok_t *pidsTok = jsmn_find_node(json, "pids");
    if (pidsTok != NULL && (++pidsTok)->type == JSMN_ARRAY) {
        int pidMax = pidsTok->size;
        if (pidMax > MAX_OBD2_MESSAGE_PIDS) {
            return API_ERROR_PARAMETER;
        }
        pidMax += pidIndex;
        if (pidMax > OBD2_CHANNELS) {
            return API_ERROR_PARAMETER;
        }

        for (pidsTok++; pidIndex < pidMax; pidIndex++) {
            PidConfig *pidCfg = obd2Cfg->pids + pidIndex;
            ChannelConfig *chCfg = &(pidCfg->cfg);
            pidsTok = setChannelConfig(serial, pidsTok, chCfg, setPidExtendedField, pidCfg);
        }
    }
    obd2Cfg->enabledPids = pidIndex;

    setUnsignedCharValueIfExists(json, "en", &obd2Cfg->enabled, NULL);

    configChanged();
    return API_SUCCESS;
}

/**
 * Sets the sampleRate in the LapConfig struct of all channels to the
 * rate of the highest channel within the LapConfig struct.
 */
void set_consistent_sample_rates(LapConfig *lc)
{
	ChannelConfig *lc_cfgs[] = {
		&lc->lapCountCfg,
		&lc->lapTimeCfg,
		&lc->sectorCfg,
		&lc->sectorTimeCfg,
		&lc->predTimeCfg,
		&lc->elapsed_time_cfg,
		&lc->current_lap_cfg,
		NULL,
	};

	/* Find the highest sample rate */
	int high_sr = 0;
	for (ChannelConfig **cc_ptr = lc_cfgs; *cc_ptr; ++cc_ptr)
		high_sr = getHigherSampleRate(high_sr, (*cc_ptr)->sampleRate);

	/* Now set them all to the highest rate. */
	for (ChannelConfig **cc_ptr = lc_cfgs; *cc_ptr; ++cc_ptr)
		(*cc_ptr)->sampleRate = high_sr;
}

int api_setLapConfig(struct Serial *serial, const jsmntok_t *json)
{
    LapConfig *lapCfg = &(getWorkingLoggerConfig()->LapConfigs);

    const jsmntok_t *lapCount = jsmn_find_node(json, "lapCount");
    if (lapCount != NULL)
        setChannelConfig(serial, lapCount + 1, &lapCfg->lapCountCfg, NULL, NULL);

    const jsmntok_t *lapTime = jsmn_find_node(json, "lapTime");
    if (lapTime != NULL)
        setChannelConfig(serial, lapTime + 1, &lapCfg->lapTimeCfg, NULL, NULL);

    const jsmntok_t *predTime = jsmn_find_node(json, "predTime");
    if (predTime != NULL)
        setChannelConfig(serial, predTime + 1, &lapCfg->predTimeCfg, NULL, NULL);

    const jsmntok_t *sector = jsmn_find_node(json, "sector");
    if (sector != NULL)
        setChannelConfig(serial, sector + 1, &lapCfg->sectorCfg, NULL, NULL);

    const jsmntok_t *sectorTime = jsmn_find_node(json, "sectorTime");
    if (sectorTime != NULL)
        setChannelConfig(serial, sectorTime + 1, &lapCfg->sectorTimeCfg, NULL, NULL);

    const jsmntok_t *elapsed = jsmn_find_node(json, "elapsedTime");
    if (elapsed != NULL)
        setChannelConfig(serial, elapsed + 1,
                         &lapCfg->elapsed_time_cfg,
                         NULL, NULL);

    const jsmntok_t *current_lap = jsmn_find_node(json, "currentLap");
    if (current_lap != NULL)
        setChannelConfig(serial, current_lap + 1,
                         &lapCfg->current_lap_cfg,
                         NULL, NULL);

    set_consistent_sample_rates(lapCfg);
    configChanged();
    return API_SUCCESS;
}

int api_getLapConfig(struct Serial *serial, const jsmntok_t *json)
{
    LapConfig *lapCfg = &(getWorkingLoggerConfig()->LapConfigs);

    json_objStart(serial);
    json_objStartString(serial, "lapCfg");

    json_objStartString(serial, "lapCount");
    json_channelConfig(serial, &lapCfg->lapCountCfg, 0);
    json_objEnd(serial, 1);

    json_objStartString(serial, "lapTime");
    json_channelConfig(serial, &lapCfg->lapTimeCfg, 0);
    json_objEnd(serial, 1);

    json_objStartString(serial, "predTime");
    json_channelConfig(serial, &lapCfg->predTimeCfg, 0);
    json_objEnd(serial, 1);

    json_objStartString(serial, "sector");
    json_channelConfig(serial, &lapCfg->sectorCfg, 0);
    json_objEnd(serial, 1);

    json_objStartString(serial, "sectorTime");
    json_channelConfig(serial, &lapCfg->sectorTimeCfg, 0);
    json_objEnd(serial, 1);

    json_objStartString(serial, "elapsed");
    json_channelConfig(serial, &lapCfg->elapsed_time_cfg, 0);
    json_objEnd(serial, 1);

    json_objStartString(serial, "currentLap");
    json_channelConfig(serial, &lapCfg->current_lap_cfg, 0);
    json_objEnd(serial, 0);

    json_objEnd(serial, 0);
    json_objEnd(serial, 0);
    return API_SUCCESS_NO_RETURN;
}

static void json_geoPointArray(struct Serial *serial, const char *name, const GeoPoint *point, int more)
{
    json_arrayStart(serial, name);
    json_arrayElementFloat(serial, point->latitude, DEFAULT_GPS_POSITION_PRECISION, 1);
    json_arrayElementFloat(serial, point->longitude, DEFAULT_GPS_POSITION_PRECISION, 0);
    json_arrayEnd(serial, more);
}

static void json_track(struct Serial *serial, const Track *track)
{
    json_int(serial, "id", track->trackId, 1);
    json_int(serial, "type", track->track_type, 1);
    if (track->track_type == TRACK_TYPE_CIRCUIT) {
        json_geoPointArray(serial, "sf", &track->circuit.startFinish, 1);
        json_arrayStart(serial, "sec");
        for (size_t i = 0; i < CIRCUIT_SECTOR_COUNT; i++) {
            json_geoPointArray(serial, NULL, &track->circuit.sectors[i], i < CIRCUIT_SECTOR_COUNT - 1);
        }
        json_arrayEnd(serial, 0);
    } else {
        GeoPoint start = getStartPoint(track);
        GeoPoint finish = getFinishPoint(track);
        json_geoPointArray(serial, "st", &start, 1);
        json_geoPointArray(serial, "fin", &finish, 1);
        json_arrayStart(serial, "sec");
        for (size_t i = 0; i < STAGE_SECTOR_COUNT; i++) {
            json_geoPointArray(serial, NULL, &track->stage.sectors[i], i < STAGE_SECTOR_COUNT - 1);
        }
        json_arrayEnd(serial, 0);
    }
}

int api_getTrackConfig(struct Serial *serial, const jsmntok_t *json)
{
    TrackConfig *trackCfg = &(getWorkingLoggerConfig()->TrackConfigs);

    json_objStart(serial);
    json_objStartString(serial, "trackCfg");
    json_float(serial, "rad", trackCfg->radius, DEFAULT_GPS_RADIUS_PRECISION, 1);
    json_int(serial, "autoDetect", trackCfg->auto_detect, 1);
    json_objStartString(serial, "track");
    json_track(serial, &trackCfg->track);
    json_objEnd(serial, 0);
    json_objEnd(serial, 0);
    json_objEnd(serial, 0);

    return API_SUCCESS_NO_RETURN;
}

static int setGeoPointIfExists(const jsmntok_t *root, const char * name, GeoPoint *geoPoint)
{
    int success = 0;
    const jsmntok_t *geoPointNode  = jsmn_find_node(root, name);
    if (geoPointNode) {
        geoPointNode++;
        if (geoPointNode && geoPointNode->type == JSMN_ARRAY && geoPointNode->size == 2) {
            geoPointNode += 1;
            jsmn_trimData(geoPointNode);
            geoPoint->latitude = atof(geoPointNode->data);
            geoPointNode += 1;
            jsmn_trimData(geoPointNode);
            geoPoint->longitude = atof(geoPointNode->data);
            success = 1;
        }
    }
    return success;
}

static void setTrack(const jsmntok_t *trackNode, Track *track)
{
    jsmn_exists_set_val_int(trackNode, "id", (int*)&(track->trackId));
    unsigned char trackType;
    if (setUnsignedCharValueIfExists(trackNode, "type", &trackType, NULL)) {
        track->track_type = (enum TrackType) trackType;
        GeoPoint *sectorsList = track->circuit.sectors;
        size_t maxSectors = CIRCUIT_SECTOR_COUNT;
        if (trackType == TRACK_TYPE_CIRCUIT) {
            setGeoPointIfExists(trackNode, "sf", &track->circuit.startFinish);
        } else {
            setGeoPointIfExists(trackNode, "st", &(track->stage.start));
            setGeoPointIfExists(trackNode, "fin", &(track->stage.finish));
            sectorsList = track->stage.sectors;
            maxSectors = STAGE_SECTOR_COUNT;
        }
        const jsmntok_t *sectors = jsmn_find_node(trackNode, "sec");
        if (sectors != NULL) {
            sectors++;
            if (sectors != NULL && sectors->type == JSMN_ARRAY) {
                sectors++;
                size_t sectorIndex = 0;
                while (sectors != NULL && sectors->type == JSMN_ARRAY && sectors->size == 2 && sectorIndex < maxSectors) {
                    GeoPoint *sector = sectorsList + sectorIndex;
                    const jsmntok_t *lat = sectors + 1;
                    const jsmntok_t *lon = sectors + 2;
                    jsmn_trimData(lat);
                    jsmn_trimData(lon);
                    sector->latitude = atof(lat->data);
                    sector->longitude = atof(lon->data);
                    sectorIndex++;
                    sectors +=3;
                }
                while (sectorIndex < maxSectors) {
                    GeoPoint *sector = sectorsList + sectorIndex;
                    sector->latitude = 0;
                    sector->longitude = 0;
                    sectorIndex++;
                }
            }
        }
    }
}

int api_resetMCU(struct Serial *serial, const jsmntok_t *json)
{
    return API_SUCCESS;
}

int api_setTrackConfig(struct Serial *serial, const jsmntok_t *json)
{

    TrackConfig *trackCfg = &(getWorkingLoggerConfig()->TrackConfigs);
    jsmn_exists_set_val_float(json, "rad", &trackCfg->radius);
    setUnsignedCharValueIfExists(json, "autoDetect", &trackCfg->auto_detect, NULL);

    const jsmntok_t *track = jsmn_find_node(json, "track");
    if (track != NULL)
        setTrack(track + 1, &trackCfg->track);

    configChanged();
    lapstats_config_changed();

    return API_SUCCESS;
}

int api_calibrateImu(struct Serial *serial, const jsmntok_t *json)
{
    imu_calibrate_zero();
    return API_SUCCESS;
}

int api_flashConfig(struct Serial *serial, const jsmntok_t *json)
{
    int rc = flashLoggerConfig();
    return (rc == 0 ? 1 : rc); //success means on internal command; other errors passed through
}

int api_addTrackDb(struct Serial *serial, const jsmntok_t *json)
{

    unsigned char mode = 0;
    int index = 0;

    if (setUnsignedCharValueIfExists(json, "mode", &mode, NULL) && jsmn_exists_set_val_int(json, "index", &index)) {
        Track track;
        const jsmntok_t *trackNode = jsmn_find_node(json, "track");
        if (trackNode != NULL)
            setTrack(trackNode + 1, &track);
        const int result = (int) add_track(&track, index,
                                     (enum track_add_mode) mode);
        if (result == TRACK_ADD_RESULT_OK) {
            if (mode == TRACK_ADD_MODE_COMPLETE) {
                lapstats_config_changed();
            }
            return API_SUCCESS;
        } else {
            return API_ERROR_SEVERE;
        }
    }
    return API_ERROR_MALFORMED;
}

int api_getTrackDb(struct Serial *serial, const jsmntok_t *json)
{
    const Tracks * tracks = get_tracks();

    size_t track_count = tracks->count;
    json_objStart(serial);
    json_objStartString(serial, "trackDb");
    json_int(serial,"size", track_count, 1);
    json_int(serial, "max", MAX_TRACK_COUNT, 1);
    json_arrayStart(serial, "tracks");
    for (size_t track_index = 0; track_index < track_count; track_index++) {
        const Track *track = tracks->tracks + track_index;
        json_objStart(serial);
        json_track(serial, track);
        json_objEnd(serial, track_index < track_count - 1);
    }
    json_arrayEnd(serial, 0);
    json_objEnd(serial, 0);
    json_objEnd(serial, 0);

    return API_SUCCESS_NO_RETURN;
}

#if LUA_SUPPORT

int api_getScript(struct Serial *serial, const jsmntok_t *json)
{
    const char *script = getScript();

    json_objStart(serial);
    json_objStartString(serial, "scriptCfg");
    json_null(serial, "page", 1);
    json_escapedString(serial, "data", script,0);
    json_objEnd(serial, 0);
    json_objEnd(serial, 0);

    return API_SUCCESS_NO_RETURN;
}

int api_setScript(struct Serial *serial, const jsmntok_t *json)
{
        const jsmntok_t *dataTok = jsmn_find_node(json, "data");
        const jsmntok_t *pageTok = jsmn_find_node(json, "page");
        const jsmntok_t *modeTok = jsmn_find_node(json, "mode");

        if (dataTok == NULL || pageTok == NULL || modeTok == NULL)
                return API_ERROR_PARAMETER;

        jsmn_trimData(++dataTok);
        jsmn_trimData(++pageTok);
        jsmn_trimData(++modeTok);

        const size_t page = atoi(pageTok->data);
        if (page >= MAX_SCRIPT_PAGES)
                return API_ERROR_PARAMETER;

        const enum script_add_mode mode =
                (enum script_add_mode) atoi(modeTok->data);
        char *script = dataTok->data;
        unescapeScript(script);
        return flashScriptPage(page, script, mode) ?
                API_SUCCESS : API_ERROR_SEVERE;
}

int api_runScript(struct Serial *serial, const jsmntok_t *json)
{
        lua_task_stop();
        lua_task_start();
        return API_SUCCESS;
}
#endif /* LUA_SUPPORT */

static void set_wifi_client_cfg(const jsmntok_t *json,
				struct wifi_client_cfg* cfg,
				const bool apply)
{
	jsmn_exists_set_val_bool(json, "active", &cfg->active);
	jsmn_exists_set_val_string(json, "ssid", cfg->ssid,
				   ARRAY_LEN(cfg->ssid), true);
	jsmn_exists_set_val_string(json, "password", cfg->passwd,
				   ARRAY_LEN(cfg->passwd), false);

	if (apply)
		wifi_update_client_config(cfg);
}

static bool set_wifi_ap_cfg(const jsmntok_t *json,
			    struct wifi_ap_cfg* cfg,
			    const bool apply)
{
	struct wifi_ap_cfg tmp_cfg;
	memcpy(&tmp_cfg, cfg, sizeof(struct wifi_ap_cfg));

	jsmn_exists_set_val_bool(json, "active", &tmp_cfg.active);
	jsmn_exists_set_val_string(json, "ssid", tmp_cfg.ssid,
				   ARRAY_LEN(tmp_cfg.ssid), true);
	jsmn_exists_set_val_string(json, "password", tmp_cfg.password,
				   ARRAY_LEN(tmp_cfg.password), false);
	jsmn_exists_set_val_int(json, "channel", (int*) &tmp_cfg.channel);

	char enc_str[12];
	jsmn_exists_set_val_string(json, "encryption", enc_str,
				   ARRAY_LEN(enc_str), true);
	tmp_cfg.encryption = wifi_api_get_encryption_enum_val(enc_str);

	if (!wifi_validate_ap_config(&tmp_cfg)) {
		pr_info("Invalid Wifi AP config given\r\n");
		return false;
	}

	/* Copy the validated config to our real config */
	memcpy(cfg, &tmp_cfg, sizeof(struct wifi_ap_cfg));

	/* Inform the Wifi device that settings may have changed */
	if (apply)
		wifi_update_ap_config(cfg);

	return true;
}

int api_set_wifi_cfg(struct Serial *serial, const jsmntok_t *json)
{
        LoggerConfig *lc = getWorkingLoggerConfig();
        struct wifi_cfg *cfg = &lc->ConnectivityConfigs.wifi;
        struct wifi_client_cfg *client_cfg = &cfg->client;
        struct wifi_ap_cfg *ap_cfg = &cfg->ap;

        const jsmntok_t* client_json_root = jsmn_find_node(json, "client");
        const jsmntok_t* ap_json_root = jsmn_find_node(json, "ap");

	jsmn_exists_set_val_bool(json, "active", &cfg->active);

	bool apply = true;
	jsmn_exists_set_val_bool(json, "apply", &apply);

        if (ap_json_root)
                if (!set_wifi_ap_cfg(ap_json_root, ap_cfg, apply))
                        return API_ERROR_PARAMETER;

        if (client_json_root)
                set_wifi_client_cfg(client_json_root, client_cfg, apply);

        return API_SUCCESS;
}

static void get_wifi_client_cfg(struct Serial *serial,
                                const struct wifi_client_cfg* cfg,
                                const bool more)

{
        json_objStartString(serial, "client");

        json_bool(serial, "active", cfg->active, true);
        json_string(serial, "ssid", cfg->ssid, true);
        json_string(serial, "password", cfg->passwd, false);

        json_objEnd(serial, more);
}

static void get_wifi_ap_cfg(struct Serial *serial,
                            const struct wifi_ap_cfg* cfg,
                            const bool more)
{
        json_objStartString(serial, "ap");

        json_bool(serial, "active", cfg->active, 1);
        json_string(serial, "ssid", cfg->ssid, 1);
        json_string(serial, "password", cfg->password, 1);
        json_int(serial, "channel", cfg->channel, 1);
        json_string(serial, "encryption",
                    wifi_api_get_encryption_str_val(cfg->encryption), 0);

        json_objEnd(serial, more);
}

int api_get_wifi_cfg(struct Serial *serial, const jsmntok_t *json)
{
        const LoggerConfig *lc = getWorkingLoggerConfig();
        const struct wifi_cfg *cfg = &lc->ConnectivityConfigs.wifi;
        const struct wifi_client_cfg *client_cfg = &cfg->client;
        const struct wifi_ap_cfg *ap_cfg = &cfg->ap;

        json_objStart(serial);
        json_objStartString(serial, "wifiCfg");

        json_bool(serial, "active", cfg->active, true);
        get_wifi_client_cfg(serial, client_cfg, true);
        get_wifi_ap_cfg(serial, ap_cfg, false);

        json_objEnd(serial, false);
        json_objEnd(serial, false);

        return API_SUCCESS_NO_RETURN;
}

int api_set_telemetry(struct Serial *serial, const jsmntok_t *json)
{
        int sample_rate = 0;
        jsmn_exists_set_val_int(json, "rate", &sample_rate);
	void* data = (void*) (long) sample_rate;

        const enum serial_ioctl_status status =
		serial_ioctl(serial, SERIAL_IOCTL_TELEMETRY, data);

	switch(status) {
        case SERIAL_IOCTL_STATUS_OK:
                return API_SUCCESS;
        case SERIAL_IOCTL_STATUS_UNSUPPORTED:
                return API_ERROR_UNSUPPORTED;
	case SERIAL_IOCTL_STATUS_ERR:
        default:
                return API_ERROR_UNSPECIFIED;
	}
}

int api_set_active_track(struct Serial *serial, const jsmntok_t *json)
{
        Track track;
        float tmp;
        float radius_m;

        const jsmntok_t *json_track = jsmn_find_node(json, "track");
        /* If no track is given, then fail */
        if (json_track == NULL)
                return API_ERROR_PARAMETER;

        memset(&track, 0, sizeof(track));
        setTrack(json_track + 1, &track);

        const bool rad_set = jsmn_exists_set_val_float(json, "rad", &tmp);
        const bool radius_set = jsmn_exists_set_val_float(json, "radius", &radius_m);
        if (radius_set) {
                /* Then radius_m already has value we want.  No-op */
        } else if (rad_set) {
                /* Supported only for legacy b/c setTrack */
                radius_m = lapstats_degrees_to_meters(tmp);
        } else {
                /* Default to user config, which is in degrees lat */
                const TrackConfig *trackCfg =
                        &(getWorkingLoggerConfig()->TrackConfigs);
                radius_m = lapstats_degrees_to_meters(trackCfg->radius);
        }

        lapstats_set_active_track(&track, radius_m);
        return API_SUCCESS;
}

int api_get_auto_logger_cfg(struct Serial *serial, const jsmntok_t *json)
{
        struct auto_logger_config* cfg =
                &getWorkingLoggerConfig()->auto_logger_cfg;

        json_objStart(serial);
        auto_logger_get_config(cfg, serial, false);
        json_objEnd(serial, false);

        return API_SUCCESS_NO_RETURN;
}

int api_set_auto_logger_cfg(struct Serial *serial, const jsmntok_t *json)
{
        struct auto_logger_config* cfg =
                &getWorkingLoggerConfig()->auto_logger_cfg;

        return auto_logger_set_config(cfg, json) ?
                API_SUCCESS : API_ERROR_UNSPECIFIED;
}
