/*
 * Race Capture Firmware
 *
 * Copyright (C) 2015 Autosport Labs
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
#include "cellModem.h"
#include "constants.h"
#include "cpu.h"
#include "dateTime.h"
#include "geopoint.h"
#include "gps.h"
#include "imu.h"
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
#include "mem_mang.h"
#include "mod_string.h"
#include "modp_atonum.h"
#include "printk.h"
#include "sampleRecord.h"
#include "serial.h"
#include "sim900.h"
#include "task.h"
#include "taskUtil.h"
#include "timer.h"
#include "tracks.h"

#include <stdbool.h>

/* Max number of PIDs that can be specified in the setOBD2Cfg message */
#define MAX_OBD2_MESSAGE_PIDS 10

#define NAME_EQU(A, B) (strcmp(A, B) == 0)

typedef void (*getConfigs_func)(size_t channeId, void ** baseCfg, ChannelConfig ** channelCfg);
typedef const jsmntok_t * (*setExtField_func)(const jsmntok_t *json, const char *name, const char *value, void *cfg);
typedef int (*reInitConfig_func)(LoggerConfig *config);


void unescapeTextField(char *data)
{
    char *result = data;
    while (*data) {
        if (*data == '\\') {
            switch(*(data + 1)) {
            case 'n':
                *result = '\n';
                break;
            case 'r':
                *result = '\r';
                break;
            case '\\':
                *result = '\\';
                break;
            case '"':
                *result = '\"';
                break;
            case '\0': //this should *NOT* happen
                *result = '\0';
                return;
            default: // unknown escape char?
                *result = ' ';
                break;
            }
            result++;
            data+=2;
        } else {
            *result++ = *data++;
        }
    }
    *result='\0';
}

static const jsmntok_t * findNode(const jsmntok_t *node, const char * name)
{
    while (!(node->start == 0 && node->end == 0)) {
        if (strcmp(name, jsmn_trimData(node)->data) == 0)
            return node;
        node++;
    }
    return NULL;
}

static const jsmntok_t * findStringValueNode(const jsmntok_t *node, const char *name)
{
    const jsmntok_t *field = findNode(node, name);
    if (field != NULL) {
        field++;
        if (field->type == JSMN_STRING) {
            jsmn_trimData(field);
            return field;
        }
    }
    return NULL;
}

static const jsmntok_t * findValueNode(const jsmntok_t *node, const char *name)
{
    const jsmntok_t *field = findNode(node, name);
    if (field != NULL) {
        field++;
        if (field->type == JSMN_PRIMITIVE) {
            jsmn_trimData(field);
            return field;
        }
    }
    return NULL;
}

static int setUnsignedCharValueIfExists(const jsmntok_t *root, const char * fieldName, unsigned char *target, unsigned char (*filter)(unsigned char))
{
    const jsmntok_t *valueNode = findValueNode(root, fieldName);
    if (valueNode) {
        unsigned char value = modp_atoi(valueNode->data);
        if (filter != NULL)
            value = filter(value);
        * target = value;
    }
    return (valueNode != NULL);
}

static int setIntValueIfExists(const jsmntok_t *root, const char * fieldName, int *target)
{
    const jsmntok_t *valueNode = findValueNode(root, fieldName);
    if (valueNode)
        * target = modp_atoi(valueNode->data);
    return (valueNode != NULL);
}

static int setFloatValueIfExists(const jsmntok_t *root, const char * fieldName, float *target )
{
    const jsmntok_t *valueNode = findValueNode(root, fieldName);
    if (valueNode)
        * target = modp_atof(valueNode->data);
    return (valueNode != NULL);
}

static int setStringValueIfExists(const jsmntok_t *root, const char * fieldName, char *target, size_t maxLen )
{
    const jsmntok_t *valueNode = findStringValueNode(root, fieldName);
    if (valueNode)
        strlcpy(target, valueNode->data, maxLen);
    return (valueNode != NULL);
}

int api_systemReset(Serial *serial, const jsmntok_t *json)
{
    int loader = 0;
    int reset_delay_ms = 0;
    setIntValueIfExists(json, "loader", &loader);
    setIntValueIfExists(json, "delay", &reset_delay_ms);

    if (reset_delay_ms > 0) {
        vTaskDelay(reset_delay_ms / portTICK_RATE_MS);
    }
    cpu_reset(loader);
    return API_SUCCESS_NO_RETURN;
}

int api_factoryReset(Serial *serial, const jsmntok_t *json)
{
        flash_default_logger_config();
        flash_default_script();
        flash_default_tracks();

        cpu_reset(0);
        return API_SUCCESS_NO_RETURN;
}

int api_getVersion(Serial *serial, const jsmntok_t *json)
{
    json_objStart(serial);
    json_objStartString(serial,"ver");
    json_string(serial, "name", DEVICE_NAME, 1);
    json_string(serial, "fname", FRIENDLY_DEVICE_NAME, 1);
    json_int(serial, "major", MAJOR_REV, 1);
    json_int(serial, "minor", MINOR_REV, 1);
    json_int(serial, "bugfix", BUGFIX_REV, 1);
    json_string(serial, "serial", cpu_get_serialnumber(), 0);
    json_objEnd(serial, 0);
    json_objEnd(serial, 0);
    return API_SUCCESS_NO_RETURN;
}

int api_getCapabilities(Serial *serial, const jsmntok_t *json)
{
    json_objStart(serial);
    json_objStartString(serial,"capabilities");

    json_objStartString(serial,"channels");
    json_int(serial, "analog", ANALOG_CHANNELS, 1);
    json_int(serial, "imu", IMU_CHANNELS, 1);
    json_int(serial, "gpio", GPIO_CHANNELS, 1);
    json_int(serial, "timer", TIMER_CHANNELS, 1);
    json_int(serial, "pwm", PWM_CHANNELS, 1);
    json_int(serial, "can", CAN_CHANNELS, 0);
    json_objEnd(serial, 1);

    json_objStartString(serial,"sampleRates");
    json_int(serial, "sensor", MAX_SENSOR_SAMPLE_RATE, 1);
    json_int(serial, "gps", MAX_GPS_SAMPLE_RATE, 0);
    json_objEnd(serial, 1);

    json_objStartString(serial,"db");
    json_int(serial, "tracks", MAX_TRACKS, 1);
    json_int(serial, "sectors", MAX_SECTORS, 1);
    json_int(serial, "script", SCRIPT_MEMORY_LENGTH, 0);
    json_objEnd(serial, 0);

    json_objEnd(serial, 0);
    json_objEnd(serial, 0);
    return API_SUCCESS_NO_RETURN;
}

int api_getStatus(Serial *serial, const jsmntok_t *json)
{
    json_objStart(serial);
    json_objStartString(serial, "status");

    json_objStartString(serial, "system");
    json_string(serial, "model", FRIENDLY_DEVICE_NAME, 1);
    json_int(serial, "ver_major", MAJOR_REV, 1);
    json_int(serial, "ver_minor", MINOR_REV, 1);
    json_int(serial, "ver_bugfix", BUGFIX_REV, 1);
    json_string(serial, "serial", cpu_get_serialnumber(), 1);
    json_uint(serial, "uptime", getUptimeAsInt(), 0);
    json_objEnd(serial, 1);

    json_objStartString(serial, "GPS");
    json_int(serial, "init", (int)GPS_getStatus(), 1);
    json_int(serial, "qual", GPS_getQuality(), 1);
    json_float(serial, "lat", GPS_getLatitude(), DEFAULT_GPS_POSITION_PRECISION, 1);
    json_float(serial, "lon", GPS_getLongitude(), DEFAULT_GPS_POSITION_PRECISION, 1);
    json_int(serial, "sats", GPS_getSatellitesUsedForPosition(), 1);
    json_int(serial, "DOP", GPS_getDOP(), 0);
    json_objEnd(serial, 1);

    json_objStartString(serial, "cell");
    json_int(serial, "init", cellmodem_get_status(), 1);
    json_string(serial, "IMEI", cell_get_IMEI(), 1);
    json_int(serial, "sig_str", cell_get_signal_strength(), 1);
    json_string(serial, "number", cell_get_subscriber_number(), 0);
    json_objEnd(serial, 1);

    json_objStartString(serial, "bt");
    json_int(serial, "init", (int)bt_get_status(), 0);
    json_objEnd(serial, 1);

    json_objStartString(serial, "logging");
    json_int(serial, "status", (int)logging_get_status(), 1);
    json_int(serial, "dur", logging_active_time(), 0);
    json_objEnd(serial, 1);

    json_objStartString(serial, "track");
    json_int(serial, "status", lapstats_get_track_status(), 1);
    json_int(serial, "trackId", lapstats_get_selected_track_id(), 1);
    json_int(serial, "inLap", (int)lapstats_lap_in_progress(), 1);
    json_int(serial, "armed", lc_is_armed(), 0);
    json_objEnd(serial, 1);

    json_objStartString(serial, "telemetry");
    json_int(serial, "status", (int)sim900_get_connection_status(), 1);
    json_int(serial, "dur", sim900_active_time(), 0);
    json_objEnd(serial, 0);

    json_objEnd(serial, 0);
    json_objEnd(serial, 0);
    return API_SUCCESS_NO_RETURN;
}

int api_sampleData(Serial *serial, const jsmntok_t *json)
{
    int sendMeta = 0;
    if (json->type == JSMN_OBJECT && json->size == 2) {
        const jsmntok_t * meta = json + 1;
        const jsmntok_t * value = json + 2;

        jsmn_trimData(meta);
        jsmn_trimData(value);

        if (NAME_EQU("meta",meta->data)) {
            sendMeta = modp_atoi(value->data);
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

int api_heart_beat(Serial *serial, const jsmntok_t *json)
{
    json_objStart(serial);
    json_int(serial, "hb", getUptimeAsInt(), 0);
    json_objEnd(serial, 0);
    return API_SUCCESS_NO_RETURN;
}

void api_sendLogStart(Serial *serial)
{
    json_objStart(serial);
    json_int(serial, "logStart", 1, 0);
    json_objEnd(serial, 0);
}

void api_sendLogEnd(Serial *serial)
{
    json_objStart(serial);
    json_int(serial, "logEnd", 1, 0);
    json_objEnd(serial, 0);
}

int api_log(Serial *serial, const jsmntok_t *json)
{
    if (json->type == JSMN_PRIMITIVE && json->size == 0) {
        jsmn_trimData(json);
        int doLogging = modp_atoi(json->data);
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

static void json_channelConfig(Serial *serial, ChannelConfig *cfg, int more)
{
    json_string(serial, "nm", cfg->label, 1);
    json_string(serial, "ut", cfg->units, 1);
    json_float(serial, "min", cfg->min, cfg->precision, 1);
    json_float(serial, "max", cfg->max, cfg->precision, 1);
    json_int(serial, "prec", (int) cfg->precision, 1);
    json_int(serial, "sr", decodeSampleRate(cfg->sampleRate), more);
}

static void write_sample_meta(Serial *serial, const struct sample *sample,
                              int sampleRateLimit, int more)
{
        json_arrayStart(serial, "meta");
        ChannelSample *channel_sample = sample->channel_samples;

        for (size_t i = 0; i < sample->channel_count; ++i, ++channel_sample) {
                if (0 < i)
                        serial->put_c(',');

                serial->put_c('{');
                json_channelConfig(serial, channel_sample->cfg, 0);
                serial->put_c('}');
        }

        json_arrayEnd(serial, more);
}

int api_getMeta(Serial *serial, const jsmntok_t *json)
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

void api_send_sample_record(Serial *serial, struct sample *sample,
                            unsigned int tick, int sendMeta)
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
                        channelBitmask[channelBitmaskIndex] =
                                channelBitmask[channelBitmaskIndex] |
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
                                pr_warning("sendSampleRec: unknown sample "
                                           "data type\r\n");
                                break;
                        }
                        serial->put_c(',');
                }
        }

        size_t channelBitmaskCount = channelBitmaskIndex + 1;
        for (size_t i = 0; i < channelBitmaskCount; i++) {
                put_uint(serial, channelBitmask[i]);
                if (i < channelBitmaskCount - 1)
                        serial->put_c(',');
        }

        json_arrayEnd(serial, 0);
        json_objEnd(serial, 0);
        json_objEnd(serial, 0);
}

static const jsmntok_t * setChannelConfig(Serial *serial, const jsmntok_t *cfg,
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
        unescapeTextField(value);

        if (NAME_EQU("nm", name))
            memcpy(channelCfg->label, value, DEFAULT_LABEL_LENGTH);
        else if (NAME_EQU("ut", name))
            memcpy(channelCfg->units, value, DEFAULT_UNITS_LENGTH);
        else if (NAME_EQU("min", name))
            channelCfg->min = modp_atof(value);
        else if (NAME_EQU("max", name))
            channelCfg->max = modp_atof(value);
        else if (NAME_EQU("sr", name))
            channelCfg->sampleRate = encodeSampleRate(modp_atoi(value));
        else if (NAME_EQU("prec", name))
            channelCfg->precision = (unsigned char) modp_atoi(value);
        else if (setExtField != NULL)
            cfg = setExtField(valueTok, name, value, extCfg);
    }

    return cfg;
}

static int setMultiChannelConfigGeneric(Serial *serial, const jsmntok_t * json,
                                        getConfigs_func getConfigsFunc,
                                        setExtField_func setExtFieldFunc,
                                        reInitConfig_func reInitConfigFunc)
{
    if (json->type == JSMN_OBJECT && json->size % 2 == 0) {
        for (int i = 1; i <= json->size; i += 2) {
            const jsmntok_t *idTok = json + i;
            const jsmntok_t *cfgTok = json + i + 1;
            jsmn_trimData(idTok);
            size_t id = modp_atoi(idTok->data);
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
                    adcCfg->scalingMap.rawValues[i] = modp_atof(mapArrayTok->data);
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
                    adcCfg->scalingMap.scaledValues[i] = modp_atof(mapArrayTok->data);
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
        if (NAME_EQU(mapRow->data, "raw"))
            mapRow = setScalingMapRaw(adcCfg, mapRow + 1);
        else if (NAME_EQU("scal", mapRow->data))
            mapRow = setScalingMapValues(adcCfg, mapRow + 1);
    } else {
        mapRow++;
    }
    return mapRow;
}

static const jsmntok_t * setAnalogExtendedField(const jsmntok_t *valueTok, const char *name, const char *value, void *cfg)
{
    ADCConfig *adcCfg = (ADCConfig *)cfg;
    if (NAME_EQU("scalMod", name))
        adcCfg->scalingMode = filterAnalogScalingMode(modp_atoi(value));
    else if (NAME_EQU("scaling", name))
        adcCfg->linearScaling = modp_atof(value);
    else if (NAME_EQU("offset", name))
        adcCfg->linearOffset = modp_atof(value);
    else if (NAME_EQU("alpha", name))
        adcCfg->filterAlpha = modp_atof(value);
    else if (NAME_EQU("cal", name))
        adcCfg->calibration = modp_atof(value);
    else if (NAME_EQU("map", name)) {
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

int api_setAnalogConfig(Serial *serial, const jsmntok_t * json)
{
    int res = setMultiChannelConfigGeneric(serial, json, getAnalogConfigs, setAnalogExtendedField, ADC_init);
    return res;
}

static void sendAnalogConfig(Serial *serial, size_t startIndex, size_t endIndex)
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
                serial->put_c(',');
        }

        json_arrayEnd(serial, 1);
        json_arrayStart(serial, "scal");

        for (size_t b = 0; b < ANALOG_SCALING_BINS; b++) {
            put_float(serial, adcCfg->scalingMap.scaledValues[b], DEFAULT_ANALOG_SCALING_PRECISION);
            if (b < ANALOG_SCALING_BINS - 1)
                serial->put_c(',');
        }

        json_arrayEnd(serial, 0);
        json_objEnd(serial, 0); //map
        json_objEnd(serial, i != endIndex); //index
    }
    json_objEnd(serial, 0);
    json_objEnd(serial, 0);
}

int api_getAnalogConfig(Serial *serial, const jsmntok_t * json)
{
    size_t startIndex = 0;
    size_t endIndex = 0;
    if (json->type == JSMN_PRIMITIVE) {
        if (jsmn_isNull(json)) {
            startIndex = 0;
            endIndex = CONFIG_ADC_CHANNELS - 1;
        } else {
            jsmn_trimData(json);
            startIndex = endIndex = modp_atoi(json->data);
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

    if (NAME_EQU("mode",name))
        imuCfg->mode = filterImuMode(modp_atoi(value));
    else if (NAME_EQU("chan",name))
        imuCfg->physicalChannel = filterImuChannel(modp_atoi(value));
    else if (NAME_EQU("zeroVal",name))
        imuCfg->zeroValue = modp_atoi(value);
    else if (NAME_EQU("alpha", name))
        imuCfg->filterAlpha = modp_atof(value);
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

int api_setImuConfig(Serial *serial, const jsmntok_t *json)
{
    int res = setMultiChannelConfigGeneric(serial, json, getImuConfigs, setImuExtendedField, imu_soft_init);
    return res;
}

static void sendImuConfig(Serial *serial, size_t startIndex, size_t endIndex)
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

int api_getImuConfig(Serial *serial, const jsmntok_t *json)
{
    size_t startIndex = 0;
    size_t endIndex = 0;
    if (json->type == JSMN_PRIMITIVE) {
        if (jsmn_isNull(json)) {
            startIndex = 0;
            endIndex = CONFIG_IMU_CHANNELS - 1;
        } else {
            jsmn_trimData(json);
            startIndex = endIndex = modp_atoi(json->data);
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
static void setConfigGeneric(Serial *serial, const jsmntok_t * json, void *cfg, setExtField_func setExtField)
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

int api_getCellConfig(Serial *serial, const jsmntok_t *json)
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

int api_getBluetoothConfig(Serial *serial, const jsmntok_t *json)
{
    BluetoothConfig *cfg = &(getWorkingLoggerConfig()->ConnectivityConfigs.bluetoothConfig);
    json_objStart(serial);
    json_objStartString(serial, "btCfg");
    json_string(serial, "name", cfg->new_name, 1);
    json_string(serial, "pass", cfg->new_pin, 0);
    json_objEnd(serial, 0);
    json_objEnd(serial, 0);
    return API_SUCCESS_NO_RETURN;
}

int api_getLogfile(Serial *serial, const jsmntok_t *json)
{
    json_objStart(serial);
    json_valueStart(serial, "logfile");
    serial->put_c('"');
    read_log_to_serial(serial, 1);
    serial->put_c('"');
    json_objEnd(serial,0);
    return API_SUCCESS_NO_RETURN;
}

int api_setLogfileLevel(Serial *serial, const jsmntok_t *json)
{
    int level;
    if (setIntValueIfExists(json, "level", &level)) {
        set_log_level((enum log_level) level);
        return API_SUCCESS;
    } else {
        return API_ERROR_PARAMETER;
    }
}

static void setCellConfig(const jsmntok_t *root)
{
    const jsmntok_t *cellCfgNode = findNode(root, "cellCfg");
    if (cellCfgNode) {
        CellularConfig *cellCfg = &(getWorkingLoggerConfig()->ConnectivityConfigs.cellularConfig);
        cellCfgNode++;
        setUnsignedCharValueIfExists(cellCfgNode, "cellEn", &cellCfg->cellEnabled, NULL);
        setStringValueIfExists(cellCfgNode, "apnHost", cellCfg->apnHost, CELL_APN_HOST_LENGTH);
        setStringValueIfExists(cellCfgNode, "apnUser", cellCfg->apnUser, CELL_APN_USER_LENGTH);
        setStringValueIfExists(cellCfgNode, "apnPass", cellCfg->apnPass, CELL_APN_PASS_LENGTH);
    }
}

static void setBluetoothConfig(const jsmntok_t *root)
{
    const jsmntok_t *btCfgNode = findNode(root, "btCfg");
    if (btCfgNode != NULL) {
        btCfgNode++;
        BluetoothConfig *btCfg = &(getWorkingLoggerConfig()->ConnectivityConfigs.bluetoothConfig);
        setUnsignedCharValueIfExists(btCfgNode, "btEn", &btCfg->btEnabled, NULL);
        setStringValueIfExists(btCfgNode, "name", btCfg->new_name, BT_DEVICE_NAME_LENGTH);
        setStringValueIfExists(btCfgNode, "pass", btCfg->new_pin, BT_PASSCODE_LENGTH);
    }
}

static void setTelemetryConfig(const jsmntok_t *root)
{
    const jsmntok_t *telemetryCfgNode = findNode(root, "telCfg");
    if (telemetryCfgNode) {
        telemetryCfgNode++;
        TelemetryConfig *telemetryCfg = &(getWorkingLoggerConfig()->ConnectivityConfigs.telemetryConfig);
        setStringValueIfExists(telemetryCfgNode, "deviceId", telemetryCfg->telemetryDeviceId, DEVICE_ID_LENGTH);
        setStringValueIfExists(telemetryCfgNode, "host", telemetryCfg->telemetryServerHost, TELEMETRY_SERVER_HOST_LENGTH);
        setUnsignedCharValueIfExists(telemetryCfgNode, "bgStream", &telemetryCfg->backgroundStreaming, filterBgStreamingMode);
    }
}

int api_setConnectivityConfig(Serial *serial, const jsmntok_t *json)
{
    setBluetoothConfig(json);
    setCellConfig(json);
    setTelemetryConfig(json);
    configChanged();
    return API_SUCCESS;
}

int api_getConnectivityConfig(Serial *serial, const jsmntok_t *json)
{
    ConnectivityConfig *cfg = &(getWorkingLoggerConfig()->ConnectivityConfigs);
    json_objStart(serial);
    json_objStartString(serial, "connCfg");

    json_objStartString(serial, "btCfg");
    json_int(serial, "btEn", cfg->bluetoothConfig.btEnabled, 1);
    json_string(serial, "name", cfg->bluetoothConfig.new_name, 1);
    json_string(serial, "pass", cfg->bluetoothConfig.new_pin, 0);
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

static void sendPwmConfig(Serial *serial, size_t startIndex, size_t endIndex)
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


int api_getPwmConfig(Serial *serial, const jsmntok_t *json)
{
    size_t startIndex = 0;
    size_t endIndex = 0;
    if (json->type == JSMN_PRIMITIVE) {
        if (jsmn_isNull(json)) {
            startIndex = 0;
            endIndex = CONFIG_PWM_CHANNELS - 1;
        } else {
            jsmn_trimData(json);
            startIndex = endIndex = modp_atoi(json->data);
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

    if (NAME_EQU("outMode", name))
        pwmCfg->outputMode = filterPwmOutputMode(modp_atoi(value));
    if (NAME_EQU("logMode", name))
        pwmCfg->loggingMode = filterPwmLoggingMode(modp_atoi(value));
    if (NAME_EQU("stDutyCyc", name))
        pwmCfg->startupDutyCycle = filterPwmDutyCycle(modp_atoi(value));
    if (NAME_EQU("stPeriod", name))
        pwmCfg->startupPeriod = filterPwmPeriod(modp_atoi(value));
    return valueTok + 1;
}

int api_setPwmConfig(Serial *serial, const jsmntok_t *json)
{
    int res = setMultiChannelConfigGeneric(serial, json, getPwmConfigs, setPwmExtendedField, PWM_update_config);
    return res;
}

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

    if (NAME_EQU("mode", name))
        gpioCfg->mode = filterGpioMode(modp_atoi(value));
    return valueTok + 1;
}

static void sendGpioConfig(Serial *serial, size_t startIndex, size_t endIndex)
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


int api_getGpioConfig(Serial *serial, const jsmntok_t *json)
{
    size_t startIndex = 0;
    size_t endIndex = 0;
    if (json->type == JSMN_PRIMITIVE) {
        if (jsmn_isNull(json)) {
            startIndex = 0;
            endIndex = CONFIG_GPIO_CHANNELS - 1;
        } else {
            jsmn_trimData(json);
            startIndex = endIndex = modp_atoi(json->data);
        }
    }
    if (startIndex <= CONFIG_GPIO_CHANNELS) {
        sendGpioConfig(serial, startIndex, endIndex);
        return API_SUCCESS_NO_RETURN;
    } else {
        return API_ERROR_PARAMETER;
    }
}

int api_setGpioConfig(Serial *serial, const jsmntok_t *json)
{
    int res = setMultiChannelConfigGeneric(serial, json, getGpioConfigs, setGpioExtendedField, GPIO_init);
    return res;
}

static void getTimerConfigs(size_t channelId, void ** baseCfg, ChannelConfig ** channelCfg)
{
    if (channelId < TIMER_CHANNELS) {
        TimerConfig *c =&(getWorkingLoggerConfig()->TimerConfigs[channelId]);
        *baseCfg = c;
        *channelCfg = &c->cfg;
    }
}

static const jsmntok_t * setTimerExtendedField(const jsmntok_t *valueTok, const char *name, const char *value, void *cfg)
{
    TimerConfig *timerCfg = (TimerConfig *)cfg;

    int iValue = modp_atoi(value);
    if (NAME_EQU("st", name))
        timerCfg->slowTimerEnabled = (iValue != 0);
    if (NAME_EQU("mode", name))
        timerCfg->mode = filterTimerMode(iValue);
    if (NAME_EQU("alpha", name))
        timerCfg->filterAlpha = modp_atof(value);
    if (NAME_EQU("ppr", name))
        timerCfg->pulsePerRevolution = filterPulsePerRevolution(iValue);
    if (NAME_EQU("speed", name))
        timerCfg->timerSpeed = filterTimerDivider(iValue);

    return valueTok + 1;
}

static void sendTimerConfig(Serial *serial, size_t startIndex, size_t endIndex)
{
    json_objStart(serial);
    json_objStartString(serial, "timerCfg");
    for (size_t i = startIndex; i <= endIndex; i++) {
        TimerConfig *cfg = &(getWorkingLoggerConfig()->TimerConfigs[i]);
        json_objStartInt(serial, i);
        json_channelConfig(serial, &(cfg->cfg), 1);
        json_uint(serial, "st", cfg->slowTimerEnabled, 1);
        json_uint(serial, "mode", cfg->mode, 1);
        json_float(serial, "alpha", cfg->filterAlpha, FILTER_ALPHA_PRECISION, 1);
        json_uint(serial, "ppr", cfg->pulsePerRevolution, 1);
        json_uint(serial, "speed", cfg->timerSpeed, 0);
        json_objEnd(serial, i != endIndex);
    }
    json_objEnd(serial, 0);
    json_objEnd(serial, 0);
}

int api_getTimerConfig(Serial *serial, const jsmntok_t *json)
{
    size_t startIndex = 0;
    size_t endIndex = 0;
    if (json->type == JSMN_PRIMITIVE) {
        if (jsmn_isNull(json)) {
            startIndex = 0;
            endIndex = CONFIG_TIMER_CHANNELS - 1;
        } else {
            jsmn_trimData(json);
            startIndex = endIndex = modp_atoi(json->data);
        }
    }
    if (startIndex <= CONFIG_TIMER_CHANNELS) {
        sendTimerConfig(serial, startIndex, endIndex);
        return API_SUCCESS_NO_RETURN;
    } else {
        return API_ERROR_PARAMETER;
    }
}

int api_setTimerConfig(Serial *serial, const jsmntok_t *json)
{
    int res = setMultiChannelConfigGeneric(serial, json, getTimerConfigs, setTimerExtendedField, timer_init);
    return res;
}

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

int api_getGpsConfig(Serial *serial, const jsmntok_t *json)
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
    json_int(serial, "dop", gpsCfg->DOP.sampleRate != SAMPLE_DISABLED, 0);

    json_objEnd(serial, 0);
    json_objEnd(serial, 0);
    return API_SUCCESS_NO_RETURN;
}

static void gpsConfigTestAndSet(const jsmntok_t *json, ChannelConfig *cfg,
                                const char *str, const unsigned short sr)
{
    unsigned char test = 0;
    setUnsignedCharValueIfExists(json, str, &test, NULL);
    cfg->sampleRate = test == 0 ? SAMPLE_DISABLED : sr;

}

int api_setGpsConfig(Serial *serial, const jsmntok_t *json)
{
    GPSConfig *gpsCfg = &(getWorkingLoggerConfig()->GPSConfigs);

    unsigned short sr = SAMPLE_DISABLED;
    int tmp = 0;
    if (setIntValueIfExists(json, "sr", &tmp))
        sr = encodeSampleRate(tmp);

    gpsConfigTestAndSet(json, &(gpsCfg->latitude), "pos", sr);
    gpsConfigTestAndSet(json, &(gpsCfg->longitude), "pos", sr);
    gpsConfigTestAndSet(json, &(gpsCfg->speed), "speed", sr);
    gpsConfigTestAndSet(json, &(gpsCfg->distance), "dist", sr);
    gpsConfigTestAndSet(json, &(gpsCfg->altitude), "alt", sr);
    gpsConfigTestAndSet(json, &(gpsCfg->satellites), "sats", sr);
    gpsConfigTestAndSet(json, &(gpsCfg->quality), "qual", sr);
    gpsConfigTestAndSet(json, &(gpsCfg->DOP), "dop", sr);

    configChanged();
    return API_SUCCESS;
}

int api_getCanConfig(Serial *serial, const jsmntok_t *json)
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

int api_setCanConfig(Serial *serial, const jsmntok_t *json)
{

    CANConfig *canCfg = &getWorkingLoggerConfig()->CanConfig;
    setUnsignedCharValueIfExists( json, "en", &canCfg->enabled, NULL);

    const jsmntok_t *baudTok = findNode(json, "baud");
    if (baudTok != NULL && (++baudTok)->type == JSMN_ARRAY) {
        size_t arrSize = json->size;
        if (arrSize > CONFIG_CAN_CHANNELS)
            arrSize = CONFIG_CAN_CHANNELS;
        size_t can_index = 0;
        for (baudTok++; can_index < arrSize; can_index++, baudTok++) {
            canCfg->baud[can_index] = modp_atoi(baudTok->data);
        }
    }
    return API_SUCCESS;
}

int api_getObd2Config(Serial *serial, const jsmntok_t *json)
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

    if (NAME_EQU("pid", name))
        pidCfg->pid = (unsigned short) modp_atoi(value);

    return valueTok + 1;
}

int api_setObd2Config(Serial *serial, const jsmntok_t *json)
{
    OBD2Config *obd2Cfg = &(getWorkingLoggerConfig()->OBD2Configs);

    int pidIndex = 0;
    setIntValueIfExists(json, "index", &pidIndex);

    if (pidIndex >= OBD2_CHANNELS) {
        return API_ERROR_PARAMETER;
    }

    const jsmntok_t *pidsTok = findNode(json, "pids");
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

int api_setLapConfig(Serial *serial, const jsmntok_t *json)
{
    LapConfig *lapCfg = &(getWorkingLoggerConfig()->LapConfigs);

    const jsmntok_t *lapCount = findNode(json, "lapCount");
    if (lapCount != NULL)
        setChannelConfig(serial, lapCount + 1, &lapCfg->lapCountCfg, NULL, NULL);

    const jsmntok_t *lapTime = findNode(json, "lapTime");
    if (lapTime != NULL)
        setChannelConfig(serial, lapTime + 1, &lapCfg->lapTimeCfg, NULL, NULL);

    const jsmntok_t *predTime = findNode(json, "predTime");
    if (predTime != NULL)
        setChannelConfig(serial, predTime + 1, &lapCfg->predTimeCfg, NULL, NULL);

    const jsmntok_t *sector = findNode(json, "sector");
    if (sector != NULL)
        setChannelConfig(serial, sector + 1, &lapCfg->sectorCfg, NULL, NULL);

    const jsmntok_t *sectorTime = findNode(json, "sectorTime");
    if (sectorTime != NULL)
        setChannelConfig(serial, sectorTime + 1, &lapCfg->sectorTimeCfg, NULL, NULL);

    const jsmntok_t *elapsed = findNode(json, "elapsedTime");
    if (elapsed != NULL)
        setChannelConfig(serial, elapsed + 1,
                         &lapCfg->elapsed_time_cfg,
                         NULL, NULL);

    const jsmntok_t *current_lap = findNode(json, "currentLap");
    if (current_lap != NULL)
        setChannelConfig(serial, current_lap + 1,
                         &lapCfg->current_lap_cfg,
                         NULL, NULL);

    configChanged();
    return API_SUCCESS;
}

int api_getLapConfig(Serial *serial, const jsmntok_t *json)
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

static void json_geoPointArray(Serial *serial, const char *name, const GeoPoint *point, int more)
{
    json_arrayStart(serial, name);
    json_arrayElementFloat(serial, point->latitude, DEFAULT_GPS_POSITION_PRECISION, 1);
    json_arrayElementFloat(serial, point->longitude, DEFAULT_GPS_POSITION_PRECISION, 0);
    json_arrayEnd(serial, more);
}

static void json_track(Serial *serial, const Track *track)
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

int api_getTrackConfig(Serial *serial, const jsmntok_t *json)
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
    const jsmntok_t *geoPointNode  = findNode(root, name);
    if (geoPointNode) {
        geoPointNode++;
        if (geoPointNode && geoPointNode->type == JSMN_ARRAY && geoPointNode->size == 2) {
            geoPointNode += 1;
            jsmn_trimData(geoPointNode);
            geoPoint->latitude = modp_atof(geoPointNode->data);
            geoPointNode += 1;
            jsmn_trimData(geoPointNode);
            geoPoint->longitude = modp_atof(geoPointNode->data);
            success = 1;
        }
    }
    return success;
}

static void setTrack(const jsmntok_t *trackNode, Track *track)
{
    setIntValueIfExists(trackNode, "id", (int*)&(track->trackId));
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
        const jsmntok_t *sectors = findNode(trackNode, "sec");
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
                    sector->latitude = modp_atof(lat->data);
                    sector->longitude = modp_atof(lon->data);
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

int api_resetMCU(Serial *serial, const jsmntok_t *json)
{
    return API_SUCCESS;
}

int api_setTrackConfig(Serial *serial, const jsmntok_t *json)
{

    TrackConfig *trackCfg = &(getWorkingLoggerConfig()->TrackConfigs);
    setFloatValueIfExists(json, "rad", &trackCfg->radius);
    setUnsignedCharValueIfExists(json, "autoDetect", &trackCfg->auto_detect, NULL);

    const jsmntok_t *track = findNode(json, "track");
    if (track != NULL)
        setTrack(track + 1, &trackCfg->track);

    configChanged();
    lapstats_config_changed();

    return API_SUCCESS;
}

int api_calibrateImu(Serial *serial, const jsmntok_t *json)
{
    imu_calibrate_zero();
    return API_SUCCESS;
}

int api_flashConfig(Serial *serial, const jsmntok_t *json)
{
    int rc = flashLoggerConfig();
    return (rc == 0 ? 1 : rc); //success means on internal command; other errors passed through
}

int api_addTrackDb(Serial *serial, const jsmntok_t *json)
{

    unsigned char mode = 0;
    int index = 0;

    if (setUnsignedCharValueIfExists(json, "mode", &mode, NULL) && setIntValueIfExists(json, "index", &index)) {
        Track track;
        const jsmntok_t *trackNode = findNode(json, "track");
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

int api_getTrackDb(Serial *serial, const jsmntok_t *json)
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

int api_getScript(Serial *serial, const jsmntok_t *json)
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

int api_setScript(Serial *serial, const jsmntok_t *json)
{
    int rc = API_ERROR_UNSPECIFIED;

    const jsmntok_t *dataTok = findNode(json, "data");
    const jsmntok_t *pageTok = findNode(json, "page");
    const jsmntok_t *modeTok = findNode(json, "mode");

    if (dataTok != NULL && pageTok != NULL && modeTok !=NULL) {
        dataTok++;
        pageTok++;
        modeTok++;

        jsmn_trimData(dataTok);
        jsmn_trimData(pageTok);
        jsmn_trimData(modeTok);

        size_t page = modp_atoi(pageTok->data);
        size_t mode = modp_atoi(modeTok->data);
        if (page < MAX_SCRIPT_PAGES) {
            char *script = dataTok->data;
            unescapeScript(script);
            const int flashResult =
               flashScriptPage(page, script, (enum script_add_mode) mode);

            rc = flashResult == 1 ? API_SUCCESS : API_ERROR_SEVERE;
        } else {
            rc = API_ERROR_PARAMETER;
        }
    } else {
        rc = API_ERROR_PARAMETER;
    }

    return rc;
}

int api_runScript(Serial *serial, const jsmntok_t *json)
{
        terminate_lua();
        initialize_lua();
        return API_SUCCESS;
}
