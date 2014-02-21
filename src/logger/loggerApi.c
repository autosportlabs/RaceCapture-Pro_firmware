/*
 * loggerApi.c
 *
 *  Created on: Jun 10, 2013
 *      Author: brent
 */
#include "loggerApi.h"
#include "loggerConfig.h"
#include "modp_atonum.h"
#include "mod_string.h"
#include "sampleRecord.h"
#include "loggerSampleData.h"
#include "loggerData.h"
#include "loggerTaskEx.h"
#include "accelerometer.h"
#include "loggerHardware.h"
#include "serial.h"
#include "mem_mang.h"
#include "printk.h"


#define NAME_EQU(A, B) (strcmp(A, B) == 0)

typedef void (*getConfigs_func)(size_t channelId, void ** baseCfg, ChannelConfig ** channelCfg);
typedef const jsmntok_t * (*setExtField_func)(const jsmntok_t *json, const char *name, const char *value, void *cfg);



void unescapeTextField(char *data){
	char *result = data;
	while (*data){
		if (*data == '\\'){
			switch(*(data + 1)){
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
					break;
				default: // unknown escape char?
					*result = ' ';
					break;
			}
			result++;
			data+=2;
		}
		else{
			*result++ = *data++;
		}
	}
	*result='\0';
}

const static jsmntok_t * findNode(const jsmntok_t *node, const char * name){

	while (!(node->start == 0 && node->end == 0)){
		if (strcmp(name, jsmn_trimData(node)->data) == 0) return node;
		node++;
	}
	return NULL;
}

int api_sampleData(Serial *serial, const jsmntok_t *json){

	int sendMeta = 0;
	if (json->type == JSMN_OBJECT && json->size == 2){
		const jsmntok_t * meta = json + 1;
		const jsmntok_t * value = json + 2;

		jsmn_trimData(meta);
		jsmn_trimData(value);

		if (NAME_EQU("meta",meta->data)){
			sendMeta = modp_atoi(value->data);
		}
	}
	LoggerConfig * config = getWorkingLoggerConfig();
	size_t channelCount = get_enabled_channel_count(config);
	ChannelSample *samples = create_channel_sample_buffer(config, channelCount);

	if (samples == 0) return API_ERROR_SEVERE;

	init_channel_sample_buffer(config, samples, channelCount);
	populate_sample_buffer(samples, channelCount, 0);
	api_sendSampleRecord(serial, samples, channelCount, 0, sendMeta);
	portFree(samples);
	return API_SUCCESS_NO_RETURN;
}

void api_sendLogStart(Serial *serial){
	json_messageStart(serial);
	json_int(serial, "logStart", 1, 0);
	json_messageEnd(serial);
}

void api_sendLogEnd(Serial *serial){
	json_messageStart(serial);
	json_int(serial, "logEnd", 1, 0);
	json_messageEnd(serial);
}

int api_log(Serial *serial, const jsmntok_t *json){
	int doLogging = 0;
	if (json->type == JSMN_PRIMITIVE && json->size == 0){
		jsmn_trimData(json);
		doLogging = modp_atoi(json->data);
		if (doLogging){
			//startLogging();
		}
		else{
			//stopLogging();
		}
	}
	return API_SUCCESS;
}

static void writeSampleMeta(Serial *serial, ChannelSample *channelSamples, size_t channelCount, int sampleRateLimit, int more){
	int sampleCount = 0;
	json_arrayStart(serial, "meta");
	ChannelSample *sample = channelSamples;
	for (int i = 0; i < channelCount; i++){
		ChannelConfig * channelConfig = sample->channelConfig;
		if (SAMPLE_DISABLED == channelConfig->sampleRate) continue;
		if (sampleCount++ > 0) serial->put_c(',');
		serial->put_c('{');
		json_string(serial, "nm", channelConfig->label, 1);
		json_string(serial, "ut", channelConfig->units, 1);
		json_int(serial, "sr", decodeSampleRate(LOWER_SAMPLE_RATE(channelConfig->sampleRate, sampleRateLimit)), 0);
		serial->put_c('}');
		sample++;
	}
	json_arrayEnd(serial, more);
}

int api_getMeta(Serial *serial, const jsmntok_t *json){
	json_messageStart(serial);

	LoggerConfig *loggerConfig = getWorkingLoggerConfig();
	size_t channelCount = get_enabled_channel_count(loggerConfig);
	ChannelSample *channelSamples = create_channel_sample_buffer(loggerConfig, channelCount);
	if (channelSamples == 0) return API_ERROR_SEVERE;

	init_channel_sample_buffer(loggerConfig, channelSamples, channelCount);
	writeSampleMeta(serial, channelSamples, channelCount, getConnectivitySampleRateLimit(), 0);

	portFree(channelSamples);
	json_blockEnd(serial, 0);
	return API_SUCCESS_NO_RETURN;
}

void api_sendSampleRecord(Serial *serial, ChannelSample *channelSamples, size_t channelCount, unsigned int tick, int sendMeta){
	json_messageStart(serial);
	json_blockStart(serial, "s");

	json_uint(serial,"t",tick,1);
	if (sendMeta) writeSampleMeta(serial, channelSamples, channelCount, getConnectivitySampleRateLimit(), 1);

	unsigned int channelsBitmask = 0;
	json_arrayStart(serial, "d");
	ChannelSample *sample = channelSamples;
	for (size_t i = 0; i < channelCount; i++){
		if (NIL_SAMPLE != sample->intValue){
			channelsBitmask = channelsBitmask | (1 << i);
			int precision = sample->precision;
			if (precision > 0){
				put_float(serial, sample->floatValue, precision);
			}
			else{
				put_int(serial, sample->intValue);
			}
			serial->put_c(',');
		}
		sample++;
	}
	put_uint(serial, channelsBitmask);
	json_arrayEnd(serial, 0);

	json_blockEnd(serial, 0);
	json_blockEnd(serial, 0);
}

static const jsmntok_t * setChannelConfig(Serial *serial, const jsmntok_t *cfg, ChannelConfig *channelCfg, setExtField_func setExtField, void *extCfg){
	if (cfg->type == JSMN_OBJECT && cfg->size % 2 == 0){
		int size = cfg->size;
		cfg++;
		for (int i = 0; i < size; i += 2 ){
			const jsmntok_t *nameTok = cfg;
			jsmn_trimData(nameTok);
			cfg++;
			const jsmntok_t *valueTok = cfg;
			cfg++;
			if (valueTok->type == JSMN_PRIMITIVE || valueTok->type == JSMN_STRING) jsmn_trimData(valueTok);

			char *name = nameTok->data;
			char *value = valueTok->data;
			unescapeTextField(value);

			if (NAME_EQU("nm",name)) setLabelGeneric(channelCfg->label, value);
			else if (NAME_EQU("ut", name)) setLabelGeneric(channelCfg->units, value);
			else if (NAME_EQU("sr", name)) channelCfg->sampleRate = encodeSampleRate(modp_atoi(value));
			else if (setExtField != NULL) cfg = setExtField(valueTok, name, value, extCfg);
		}
	}
	return cfg;
}

static void setMultiChannelConfigGeneric(Serial *serial, const jsmntok_t * json, getConfigs_func getConfigs, setExtField_func setExtFieldFunc){
	if (json->type == JSMN_OBJECT && json->size % 2 == 0){
		for (int i = 1; i <= json->size; i += 2){
			const jsmntok_t *idTok = json + i;
			const jsmntok_t *cfgTok = json + i + 1;
			jsmn_trimData(idTok);
			size_t id = modp_atoi(idTok->data);
			void *baseCfg;
			ChannelConfig *channelCfg;
			getConfigs(id, &baseCfg, &channelCfg);
			setChannelConfig(serial, cfgTok, channelCfg, setExtFieldFunc, baseCfg);
		}
	}
}

static const jsmntok_t * setScalingMapRaw(ADCConfig *adcCfg, const jsmntok_t *mapArrayTok){
	if (mapArrayTok->type == JSMN_ARRAY){
		int size = mapArrayTok->size;
		for (int i = 0; i < size; i++){
			mapArrayTok++;
			if (mapArrayTok->type == JSMN_PRIMITIVE){
				jsmn_trimData(mapArrayTok);
				if (i < ANALOG_SCALING_BINS){
					adcCfg->scalingMap.rawValues[i] = modp_atoi(mapArrayTok->data);
				}
			}
		}
	}
	return mapArrayTok + 1;
}

static const jsmntok_t * setScalingMapValues(ADCConfig *adcCfg, const jsmntok_t *mapArrayTok){
	if (mapArrayTok->type == JSMN_ARRAY){
		int size = mapArrayTok->size;
		for (int i = 0; i < size; i++){
			mapArrayTok++;
			if (mapArrayTok->type == JSMN_PRIMITIVE){
				jsmn_trimData(mapArrayTok);
				if (i < ANALOG_SCALING_BINS){
					adcCfg->scalingMap.scaledValues[i] = modp_atof(mapArrayTok->data);
				}
			}
		}
	}
	return mapArrayTok + 1;
}

static const jsmntok_t * setScalingRow(ADCConfig *adcCfg, const jsmntok_t *mapRow){
	if (mapRow->type == JSMN_STRING){
		jsmn_trimData(mapRow);
		if (NAME_EQU(mapRow->data, "raw")) mapRow = setScalingMapRaw(adcCfg, mapRow + 1);
		else if (NAME_EQU("scal", mapRow->data)) mapRow = setScalingMapValues(adcCfg, mapRow + 1);
	}
	else{
		mapRow++;
	}
	return mapRow;
}

static const jsmntok_t * setAnalogExtendedField(const jsmntok_t *valueTok, const char *name, const char *value, void *cfg){
	ADCConfig *adcCfg = (ADCConfig *)cfg;
	if (NAME_EQU("prec",name)) adcCfg->loggingPrecision = modp_atoi(value);
	else if (NAME_EQU("scalMod", name)) adcCfg->scalingMode = filterAnalogScalingMode(modp_atoi(value));
	else if (NAME_EQU("linScal", name)) adcCfg->linearScaling = modp_atof(value);
	else if (NAME_EQU("map", name)){
		if (valueTok->type == JSMN_OBJECT) {
			valueTok++;
			valueTok = setScalingRow(adcCfg, valueTok);
			valueTok = setScalingRow(adcCfg, valueTok);
			valueTok--;
		}
	}
	return valueTok + 1;
}

static void getAnalogConfigs(size_t channelId, void ** baseCfg, ChannelConfig ** channelCfg){
	ADCConfig *c =&(getWorkingLoggerConfig()->ADCConfigs[channelId]);
	*baseCfg = c;
	*channelCfg = &c->cfg;
}

int api_setAnalogConfig(Serial *serial, const jsmntok_t * json){
	setMultiChannelConfigGeneric(serial, json, getAnalogConfigs, setAnalogExtendedField);
	return API_SUCCESS;
}

static void json_channelConfig(Serial *serial, ChannelConfig *cfg, int more){
	json_string(serial, "nm", cfg->label, 1);
	json_string(serial, "ut", cfg->units, 1);
	json_int(serial, "sr", cfg->sampleRate, more);
}

static void sendAnalogConfig(Serial *serial, size_t startIndex, size_t endIndex){

	json_messageStart(serial);
	json_blockStart(serial, "getAnalogCfg");
	for (size_t i = startIndex; i <= endIndex; i++){

		ADCConfig *cfg = &(getWorkingLoggerConfig()->ADCConfigs[i]);
		json_blockStartInt(serial, i);
		json_channelConfig(serial, &(cfg->cfg), 1);
		json_int(serial, "scalMod", cfg->scalingMode, 1);

		json_int(serial, "prec", cfg->loggingPrecision, 1);
		json_float(serial, "linScal", cfg->linearScaling, ANALOG_SCALING_PRECISION, 1);

		json_blockStart(serial, "map");
		json_arrayStart(serial, "raw");

		for (size_t b = 0; b < ANALOG_SCALING_BINS; b++){
			put_int(serial,  cfg->scalingMap.rawValues[b]);
			if (b < ANALOG_SCALING_BINS - 1) serial->put_c(',');
		}
		json_arrayEnd(serial, 1);
		json_arrayStart(serial, "scal");
		for (size_t b = 0; b < ANALOG_SCALING_BINS; b++){
			put_float(serial, cfg->scalingMap.scaledValues[b], ANALOG_SCALING_PRECISION);
			if (b < ANALOG_SCALING_BINS - 1) serial->put_c(',');
		}
		json_arrayEnd(serial, 0);
		json_blockEnd(serial, 0); //map
		json_blockEnd(serial, i != endIndex); //index
	}
	json_blockEnd(serial, 0);
	json_blockEnd(serial, 0);
}

int api_getAnalogConfig(Serial *serial, const jsmntok_t * json){
	size_t startIndex = 0;
	size_t endIndex = 0;
	if (json->type == JSMN_PRIMITIVE){
		if (jsmn_isNull(json)){
			startIndex = 0;
			endIndex = CONFIG_ADC_CHANNELS - 1;
		}
		else{
			jsmn_trimData(json);
			startIndex = endIndex = modp_atoi(json->data);
		}
	}
	if (startIndex >= 0 && startIndex <= CONFIG_ADC_CHANNELS){
		sendAnalogConfig(serial, startIndex, endIndex);
		return API_SUCCESS_NO_RETURN;
	}
	else{
		return API_ERROR_PARAMETER;
	}
}

static const jsmntok_t * setAccelExtendedField(const jsmntok_t *valueTok, const char *name, const char *value, void *cfg){
	AccelConfig *accelCfg = (AccelConfig *)cfg;

	if (NAME_EQU("mode",name)) accelCfg->mode = filterAccelMode(modp_atoi(value));
	else if (NAME_EQU("chan",name)) accelCfg->accelChannel = filterAccelChannel(modp_atoi(value));
	else if (NAME_EQU("zeroVal",name)) accelCfg->zeroValue = modp_atoi(value);
	return valueTok + 1;
}

static void getAccelConfigs(size_t channelId, void ** baseCfg, ChannelConfig ** channelCfg){
	AccelConfig *c = &(getWorkingLoggerConfig()->AccelConfigs[channelId]);
	*baseCfg = c;
	*channelCfg = &c->cfg;
}

int api_setAccelConfig(Serial *serial, const jsmntok_t *json){
	setMultiChannelConfigGeneric(serial, json, getAccelConfigs, setAccelExtendedField);
	return API_SUCCESS;
}

static void sendAccelConfig(Serial *serial, size_t startIndex, size_t endIndex){
	json_messageStart(serial);
	json_blockStart(serial, "getAccelCfg");
	for (size_t i = startIndex; i <= endIndex; i++){
		AccelConfig *cfg = &(getWorkingLoggerConfig()->AccelConfigs[i]);
		json_blockStartInt(serial, i);
		json_channelConfig(serial, &(cfg->cfg), 1);
		json_uint(serial, "mode", cfg->mode, 1);
		json_uint(serial, "chan", cfg->accelChannel, 1);
		json_uint(serial, "zeroVal", cfg->zeroValue, 0);
		json_blockEnd(serial, i != endIndex); //index
	}
	json_blockEnd(serial, 0);
	json_blockEnd(serial, 0);
}

int api_getAccelConfig(Serial *serial, const jsmntok_t *json){
	size_t startIndex = 0;
	size_t endIndex = 0;
	if (json->type == JSMN_PRIMITIVE){
		if (jsmn_isNull(json)){
			startIndex = 0;
			endIndex = CONFIG_ACCEL_CHANNELS - 1;
		}
		else{
			jsmn_trimData(json);
			startIndex = endIndex = modp_atoi(json->data);
		}
	}
	if (startIndex >= 0 && startIndex <= CONFIG_ACCEL_CHANNELS){
		sendAccelConfig(serial, startIndex, endIndex);
		return API_SUCCESS_NO_RETURN;
	}
	else{
		return API_ERROR_PARAMETER;
	}
}


static void setConfigGeneric(Serial *serial, const jsmntok_t * json, void *cfg, setExtField_func setExtField){
	int size = json->size;
	if (json->type == JSMN_OBJECT && json->size % 2 == 0){
		json++;
		for (int i = 0; i < size; i += 2 ){
			const jsmntok_t *nameTok = json;
			jsmn_trimData(nameTok);
			json++;
			const jsmntok_t *valueTok = json;
			json++;
			if (valueTok->type == JSMN_PRIMITIVE || valueTok->type == JSMN_STRING) jsmn_trimData(valueTok);

			const char *name = nameTok->data;
			const char *value = valueTok->data;

			setExtField(valueTok, name, value, cfg);
		}
	}

}

int api_getCellConfig(Serial *serial, const jsmntok_t *json){
	CellularConfig *cfg = &(getWorkingLoggerConfig()->ConnectivityConfigs.cellularConfig);
	json_messageStart(serial);
	json_blockStart(serial, "getCellCfg");
	json_string(serial, "apnHost", cfg->apnHost, 1);
	json_string(serial, "apnUser", cfg->apnUser, 1);
	json_string(serial, "apnPass", cfg->apnPass, 0);
	json_blockEnd(serial, 0);
	json_blockEnd(serial, 0);
	return API_SUCCESS_NO_RETURN;
}

static const jsmntok_t * setCellExtendedField(const jsmntok_t *valueTok, const char *name, const char *value, void *cfg){
	CellularConfig *cellCfg = (CellularConfig *)cfg;
	if (NAME_EQU("apnHost", name))  setTextField(cellCfg->apnHost, value, CELL_APN_HOST_LENGTH);
	else if (NAME_EQU("apnUser", name)) setTextField(cellCfg->apnUser, value, CELL_APN_USER_LENGTH);
	else if (NAME_EQU("apnPass", name)) setTextField(cellCfg->apnPass, value, CELL_APN_PASS_LENGTH);
	return valueTok + 1;
}

int api_setCellConfig(Serial *serial, const jsmntok_t *json){
	CellularConfig *cfg = &(getWorkingLoggerConfig()->ConnectivityConfigs.cellularConfig);
	setConfigGeneric(serial, json, cfg, setCellExtendedField);
	return API_SUCCESS;
}

static const jsmntok_t * setBluetoothExtendedField(const jsmntok_t *valueTok, const char *name, const char *value, void *cfg){
	BluetoothConfig *btCfg = (BluetoothConfig *)cfg;
	if (NAME_EQU("name", name)) setTextField(btCfg->deviceName, value, BT_DEVICE_NAME_LENGTH);
	else if (NAME_EQU("pass", name)) setTextField(btCfg->passcode, value, BT_PASSCODE_LENGTH);
	return valueTok + 1;
}

int api_setBluetoothConfig(Serial *serial, const jsmntok_t *json){
	BluetoothConfig *cfg = &(getWorkingLoggerConfig()->ConnectivityConfigs.bluetoothConfig);
	setConfigGeneric(serial, json, cfg, setBluetoothExtendedField);
	return API_SUCCESS;
}

int api_getBluetoothConfig(Serial *serial, const jsmntok_t *json){
	BluetoothConfig *cfg = &(getWorkingLoggerConfig()->ConnectivityConfigs.bluetoothConfig);
	json_messageStart(serial);
	json_blockStart(serial, "getBtCfg");
	json_string(serial, "name", cfg->deviceName, 1);
	json_string(serial, "pass", cfg->passcode, 0);
	json_blockEnd(serial, 0);
	json_blockEnd(serial, 0);
	return API_SUCCESS_NO_RETURN;
}

int api_getLogfile(Serial *serial, const jsmntok_t *json){
	json_messageStart(serial);
	json_valueStart(serial, "logfile");
	serial->put_c('"');
	read_log_to_serial(serial, 1);
	serial->put_c('"');
	json_blockEnd(serial,0);
	return API_SUCCESS_NO_RETURN;
}

static const jsmntok_t * setConnectivityExtendedField(const jsmntok_t *valueTok, const char *name, const char *value, void *cfg){
	ConnectivityConfig *connCfg = (ConnectivityConfig *)cfg;
	if (NAME_EQU("sdMode", name)) connCfg->sdLoggingMode = filterSdLoggingMode(modp_atoi(value));
	else if (NAME_EQU("connMode", name)) connCfg->connectivityMode =  filterConnectivityMode(modp_atoi(value));
	else if (NAME_EQU("bgStream", name)) connCfg->backgroundStreaming = (modp_atoi(value) == 1);
	return valueTok + 1;
}

int api_setConnectivityConfig(Serial *serial, const jsmntok_t *json){
	ConnectivityConfig *cfg = &(getWorkingLoggerConfig()->ConnectivityConfigs);
	setConfigGeneric(serial, json, cfg, setConnectivityExtendedField);
	return API_SUCCESS;
}

int api_getConnectivityConfig(Serial *serial, const jsmntok_t *json){
	ConnectivityConfig *cfg = &(getWorkingLoggerConfig()->ConnectivityConfigs);
	json_messageStart(serial);
	json_blockStart(serial, "getConnCfg");
	json_int(serial, "sdMode", cfg->sdLoggingMode, 1);
	json_int(serial, "connMode", cfg->connectivityMode, 1);
	json_int(serial, "bgStream", cfg->backgroundStreaming, 0);
	json_blockEnd(serial, 0);
	json_blockEnd(serial, 0);
	return API_SUCCESS_NO_RETURN;
}

static void sendPwmConfig(Serial *serial, size_t startIndex, size_t endIndex){

	json_messageStart(serial);
	json_blockStart(serial, "getPwmCfg");
	for (size_t i = startIndex; i <= endIndex; i++){
		PWMConfig *cfg = &(getWorkingLoggerConfig()->PWMConfigs[i]);
		json_blockStartInt(serial, i);
		json_channelConfig(serial, &(cfg->cfg), 1);
		json_uint(serial, "logPrec", cfg->loggingPrecision, 1);
		json_uint(serial, "outMode", cfg->outputMode, 1);
		json_uint(serial, "logMode", cfg->loggingMode, 1);
		json_uint(serial, "stDutyCyc", cfg->startupDutyCycle, 1);
		json_uint(serial, "stPeriod", cfg->startupPeriod, 1);
		json_float(serial, "vScal", cfg->voltageScaling, DEFAULT_PWM_LOGGING_PRECISION, 0);
		json_blockEnd(serial, i != endIndex); //index
	}
	json_blockEnd(serial, 0);
	json_blockEnd(serial, 0);
}


int api_getPwmConfig(Serial *serial, const jsmntok_t *json){
	size_t startIndex = 0;
	size_t endIndex = 0;
	if (json->type == JSMN_PRIMITIVE){
		if (jsmn_isNull(json)){
			startIndex = 0;
			endIndex = CONFIG_PWM_CHANNELS - 1;
		}
		else{
			jsmn_trimData(json);
			startIndex = endIndex = modp_atoi(json->data);
		}
	}
	if (startIndex >= 0 && startIndex <= CONFIG_PWM_CHANNELS){
		sendPwmConfig(serial, startIndex, endIndex);
		return API_SUCCESS_NO_RETURN;
	}
	else{
		return API_ERROR_PARAMETER;
	}
}

static void getPwmConfigs(size_t channelId, void ** baseCfg, ChannelConfig ** channelCfg){
	PWMConfig *c =&(getWorkingLoggerConfig()->PWMConfigs[channelId]);
	*baseCfg = c;
	*channelCfg = &c->cfg;
}

static const jsmntok_t * setPwmExtendedField(const jsmntok_t *valueTok, const char *name, const char *value, void *cfg){
	PWMConfig *pwmCfg = (PWMConfig *)cfg;

	if (NAME_EQU("logPrec", name)) pwmCfg->loggingPrecision = modp_atoi(value);
	if (NAME_EQU("outMode", name)) pwmCfg->outputMode = filterPwmOutputMode(modp_atoi(value));
	if (NAME_EQU("logMode", name)) pwmCfg->loggingMode = filterPwmLoggingMode(modp_atoi(value));
	if (NAME_EQU("stDutyCyc", name)) pwmCfg->startupDutyCycle = filterPwmDutyCycle(modp_atoi(value));
	if (NAME_EQU("stPeriod", name)) pwmCfg->startupPeriod = filterPwmPeriod(modp_atoi(value));
	if (NAME_EQU("vScal", name)) pwmCfg->voltageScaling = modp_atof(value);
	return valueTok + 1;
}

int api_setPwmConfig(Serial *serial, const jsmntok_t *json){
	setMultiChannelConfigGeneric(serial, json, getPwmConfigs, setPwmExtendedField);
	return API_SUCCESS;
}

static void getGpioConfigs(size_t channelId, void ** baseCfg, ChannelConfig ** channelCfg){
	GPIOConfig *c =&(getWorkingLoggerConfig()->GPIOConfigs[channelId]);
	*baseCfg = c;
	*channelCfg = &c->cfg;
}

static const jsmntok_t * setGpioExtendedField(const jsmntok_t *valueTok, const char *name, const char *value, void *cfg){
	GPIOConfig *gpioCfg = (GPIOConfig *)cfg;

	if (NAME_EQU("mode", name)) gpioCfg->mode = filterGpioMode(modp_atoi(value));
	return valueTok + 1;
}

static void sendGpioConfig(Serial *serial, size_t startIndex, size_t endIndex){
	json_messageStart(serial);
	json_blockStart(serial, "getGpioCfg");
	for (size_t i = startIndex; i <= endIndex; i++){
		GPIOConfig *cfg = &(getWorkingLoggerConfig()->GPIOConfigs[i]);
		json_blockStartInt(serial, i);
		json_channelConfig(serial, &(cfg->cfg), 1);
		json_uint(serial, "mode", cfg->mode, 0);
		json_blockEnd(serial, i != endIndex);
	}
	json_blockEnd(serial, 0);
	json_blockEnd(serial, 0);
}


int api_getGpioConfig(Serial *serial, const jsmntok_t *json){
	size_t startIndex = 0;
	size_t endIndex = 0;
	if (json->type == JSMN_PRIMITIVE){
		if (jsmn_isNull(json)){
			startIndex = 0;
			endIndex = CONFIG_GPIO_CHANNELS - 1;
		}
		else{
			jsmn_trimData(json);
			startIndex = endIndex = modp_atoi(json->data);
		}
	}
	if (startIndex >= 0 && startIndex <= CONFIG_GPIO_CHANNELS){
		sendGpioConfig(serial, startIndex, endIndex);
		return API_SUCCESS_NO_RETURN;
	}
	else{
		return API_ERROR_PARAMETER;
	}
}

int api_setGpioConfig(Serial *serial, const jsmntok_t *json){
	setMultiChannelConfigGeneric(serial, json, getGpioConfigs, setGpioExtendedField);
	return API_SUCCESS;
}

static void getTimerConfigs(size_t channelId, void ** baseCfg, ChannelConfig ** channelCfg){
	TimerConfig *c =&(getWorkingLoggerConfig()->TimerConfigs[channelId]);
	*baseCfg = c;
	*channelCfg = &c->cfg;
}

static const jsmntok_t * setTimerExtendedField(const jsmntok_t *valueTok, const char *name, const char *value, void *cfg){
	TimerConfig *timerCfg = (TimerConfig *)cfg;

	int iValue = modp_atoi(value);
	if (NAME_EQU("prec", name)) timerCfg->loggingPrecision = iValue;
	if (NAME_EQU("sTimer", name)) timerCfg->slowTimerEnabled = (iValue != 0);
	if (NAME_EQU("mode", name)) timerCfg->mode = filterTimerMode(iValue);
	if (NAME_EQU("ppRev", name)) {
		timerCfg->pulsePerRevolution = iValue;
		calculateTimerScaling(BOARD_MCK, timerCfg);
	}
	if (NAME_EQU("timDiv", name)) timerCfg->timerDivider = filterTimerDivider(iValue);

	return valueTok + 1;
}

static void sendTimerConfig(Serial *serial, size_t startIndex, size_t endIndex){
	json_messageStart(serial);
	json_blockStart(serial, "getTimerCfg");
	for (size_t i = startIndex; i <= endIndex; i++){
		TimerConfig *cfg = &(getWorkingLoggerConfig()->TimerConfigs[i]);
		json_blockStartInt(serial, i);
		json_channelConfig(serial, &(cfg->cfg), 1);
		json_uint(serial, "prec", cfg->loggingPrecision, 1);
		json_uint(serial, "sTimer", cfg->slowTimerEnabled, 1);
		json_uint(serial, "mode", cfg->mode, 1);
		json_uint(serial, "ppRev", cfg->pulsePerRevolution, 1);
		json_uint(serial, "timDiv", cfg->timerDivider, 0);
		json_blockEnd(serial, i != endIndex);
	}
	json_blockEnd(serial, 0);
	json_blockEnd(serial, 0);
}

int api_getTimerConfig(Serial *serial, const jsmntok_t *json){
	size_t startIndex = 0;
	size_t endIndex = 0;
	if (json->type == JSMN_PRIMITIVE){
		if (jsmn_isNull(json)){
			startIndex = 0;
			endIndex = CONFIG_TIMER_CHANNELS - 1;
		}
		else{
			jsmn_trimData(json);
			startIndex = endIndex = modp_atoi(json->data);
		}
	}
	if (startIndex >= 0 && startIndex <= CONFIG_TIMER_CHANNELS){
		sendTimerConfig(serial, startIndex, endIndex);
		return API_SUCCESS_NO_RETURN;
	}
	else{
		return API_ERROR_PARAMETER;
	}
}

int api_setTimerConfig(Serial *serial, const jsmntok_t *json){
	setMultiChannelConfigGeneric(serial, json, getTimerConfigs, setTimerExtendedField);
	return API_SUCCESS;
}

int api_getGpsConfig(Serial *serial, const jsmntok_t *json){

	GPSConfig *gpsCfg = &(getWorkingLoggerConfig()->GPSConfigs);

	json_messageStart(serial);
	json_blockStart(serial, "getGpsCfg");

	json_blockStart(serial, "lat");
	json_channelConfig(serial, &gpsCfg->latitudeCfg, 0);
	json_blockEnd(serial, 1);

	json_blockStart(serial, "long");
	json_channelConfig(serial, &gpsCfg->longitudeCfg, 0);
	json_blockEnd(serial, 1);

	json_blockStart(serial, "speed");
	json_channelConfig(serial, &gpsCfg->speedCfg, 0);
	json_blockEnd(serial, 1);

	json_blockStart(serial, "sats");
	json_channelConfig(serial, &gpsCfg->satellitesCfg, 0);
	json_blockEnd(serial, 1);

	json_blockStart(serial, "time");
	json_channelConfig(serial, &gpsCfg->timeCfg, 0);
	json_blockEnd(serial, 0);

	json_blockEnd(serial, 0);
	json_blockEnd(serial, 0);
	return API_SUCCESS_NO_RETURN;
}

int api_setGpsConfig(Serial *serial, const jsmntok_t *json){

	GPSConfig *gpsCfg = &(getWorkingLoggerConfig()->GPSConfigs);
	const jsmntok_t * channelData = json + 1;

	const jsmntok_t * latitudeNode = findNode(channelData, "lat");
	if (latitudeNode != NULL) setChannelConfig(serial, latitudeNode + 1, &gpsCfg->latitudeCfg, NULL, NULL);

	const jsmntok_t * longitudeNode = findNode(channelData, "long");
	if (longitudeNode != NULL) setChannelConfig(serial, longitudeNode + 1, &gpsCfg->longitudeCfg, NULL, NULL);

	const jsmntok_t * speedNode = findNode(channelData, "speed");
	if (speedNode != NULL) setChannelConfig(serial, speedNode + 1, &gpsCfg->speedCfg, NULL, NULL);

	const jsmntok_t * timeNode = findNode(channelData, "time");
	if (timeNode != NULL) setChannelConfig(serial, timeNode + 1, &gpsCfg->timeCfg, NULL, NULL);

	const jsmntok_t * satsNode = findNode(channelData, "sats");
	if (satsNode != NULL) setChannelConfig(serial, satsNode + 1, &gpsCfg->satellitesCfg, NULL, NULL);

	return API_SUCCESS;
}

static void json_gpsTarget(Serial *serial, const char *name,  GPSTargetConfig *gpsTarget, int more){
	json_blockStart(serial, name);
	json_float(serial, "lat", gpsTarget->latitude, DEFAULT_GPS_POSITION_LOGGING_PRECISION, 1);
	json_float(serial, "long", gpsTarget->longitude, DEFAULT_GPS_POSITION_LOGGING_PRECISION, 1);
	json_float(serial, "rad", gpsTarget->targetRadius, DEFAULT_GPS_RADIUS_LOGGING_PRECISION, 0);
	json_blockEnd(serial, more);
}

int api_getTrackConfig(Serial *serial, const jsmntok_t *json){
	TrackConfig *trackCfg = &(getWorkingLoggerConfig()->TrackConfigs);

	json_messageStart(serial);
	json_blockStart(serial, "getTrackCfg");
	json_gpsTarget(serial, "startFinish", &trackCfg->startFinishConfig, 1);
	json_gpsTarget(serial, "split", &trackCfg->splitConfig, 1);
	json_blockStart(serial, "lapCount");
	json_channelConfig(serial, &trackCfg->lapCountCfg, 0);
	json_blockEnd(serial, 1);
	json_blockStart(serial, "lapTime");
	json_channelConfig(serial, &trackCfg->lapTimeCfg, 0);
	json_blockEnd(serial, 1);
	json_blockStart(serial, "splitTime");
	json_channelConfig(serial, &trackCfg->splitTimeCfg, 0);
	json_blockEnd(serial, 1);
	json_blockStart(serial, "dist");
	json_channelConfig(serial, &trackCfg->distanceCfg, 0);
	json_blockEnd(serial, 1);
	json_blockStart(serial, "predTime");
	json_channelConfig(serial, &trackCfg->predTimeCfg, 0);
	json_blockEnd(serial, 0);
	json_blockEnd(serial, 0);
	json_blockEnd(serial, 0);

	return API_SUCCESS_NO_RETURN;
}

void setTargetConfig(const jsmntok_t *cfg, GPSTargetConfig *targetConfig){
	if (cfg->type == JSMN_OBJECT && cfg->size % 2 == 0){
		int size = cfg->size;
		cfg++;
		for (int i = 0; i < size; i += 2 ){
			const jsmntok_t *nameTok = cfg;
			jsmn_trimData(nameTok);
			cfg++;
			const jsmntok_t *valueTok = cfg;
			cfg++;
			if (valueTok->type == JSMN_PRIMITIVE || valueTok->type == JSMN_STRING) jsmn_trimData(valueTok);

			char *name = nameTok->data;
			char *value = valueTok->data;

			if (NAME_EQU("lat",name)) targetConfig->latitude = modp_atof(value);
			else if (NAME_EQU("long", name)) targetConfig->longitude = modp_atof(value);
			else if (NAME_EQU("rad", name)) targetConfig->targetRadius = modp_atof(value);
		}
	}
}

int api_setTrackConfig(Serial *serial, const jsmntok_t *json){

	TrackConfig *trackCfg = &(getWorkingLoggerConfig()->TrackConfigs);
	const jsmntok_t * targetData = json + 1;

	const jsmntok_t *startFinish = findNode(targetData,"startFinish");
	if (startFinish != NULL) setTargetConfig(startFinish + 1, &trackCfg->startFinishConfig);

	const jsmntok_t *split = findNode(targetData, "split");
	if (split != NULL) setTargetConfig(split + 1, &trackCfg->splitConfig);

	const jsmntok_t *lapCount = findNode(targetData, "lapCount");
	if (lapCount != NULL) setChannelConfig(serial, lapCount + 1, &trackCfg->lapCountCfg, NULL, NULL);

	const jsmntok_t *lapTime = findNode(targetData, "lapTime");
	if (lapTime != NULL) setChannelConfig(serial, lapTime + 1, &trackCfg->lapTimeCfg, NULL, NULL);

	const jsmntok_t *splitTime = findNode(targetData, "splitTime");
	if (splitTime != NULL) setChannelConfig(serial, splitTime + 1, &trackCfg->splitTimeCfg, NULL, NULL);

	const jsmntok_t *distance = findNode(targetData, "dist");
	if (distance != NULL) setChannelConfig(serial, distance + 1, &trackCfg->distanceCfg, NULL, NULL);

	const jsmntok_t *predTime = findNode(targetData, "predTime");
	if (predTime != NULL) setChannelConfig(serial, predTime + 1, &trackCfg->predTimeCfg, NULL, NULL);

	return API_SUCCESS;
}

int api_calibrateAccel(Serial *serial, const jsmntok_t *json){
	calibrateAccelZero();
	return API_SUCCESS;
}

int api_flashConfig(Serial *serial, const jsmntok_t *json){
	int rc = flashLoggerConfig();
	return (rc == 0 ? 1 : rc); //success means on internal command; other errors passed through
}
