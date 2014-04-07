/*
 * loggerApi.c
 *
 *  Created on: Jun 10, 2013
 *      Author: brent
 */
#include "loggerApi.h"
#include "loggerConfig.h"
#include "channelMeta.h"
#include "channelMeta.h"
#include "modp_atonum.h"
#include "mod_string.h"
#include "sampleRecord.h"
#include "loggerSampleData.h"
#include "loggerData.h"
#include "loggerNotifications.h"
#include "accelerometer.h"
#include "tracks.h"
#include "loggerHardware.h"
#include "serial.h"
#include "mem_mang.h"
#include "printk.h"
#include "geopoint.h"


#define NAME_EQU(A, B) (strcmp(A, B) == 0)

typedef void (*getConfigs_func)(size_t channeId, void ** baseCfg, ChannelConfig ** channelCfg);
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

const static jsmntok_t * findValueNode(const jsmntok_t *node, const char *name){
	const jsmntok_t *field = findNode(node, name);
	if (field != NULL){
		field++;
		if (field->type == JSMN_PRIMITIVE){
			jsmn_trimData(field);
			return field;
		}
	}
	return NULL;
}

static void setUnsignedCharValueIfExists(const jsmntok_t *root, const char * fieldName, unsigned char *target){
	const jsmntok_t *valueNode = findValueNode(root, fieldName);
	if (valueNode) * target = modp_atoi(valueNode->data);
}

static void setIntValueIfExists(const jsmntok_t *root, const char * fieldName, int *target){
	const jsmntok_t *valueNode = findValueNode(root, fieldName);
	if (valueNode) * target = modp_atoi(valueNode->data);
}

static void setFloatValueIfExists(const jsmntok_t *root, const char * fieldName, float *target ){
	const jsmntok_t *valueNode = findValueNode(root, fieldName);
	if (valueNode) * target = modp_atof(valueNode->data);
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
		if (SAMPLE_DISABLED == sample->sampleRate) continue;
		if (sampleCount++ > 0) serial->put_c(',');
		serial->put_c('{');
		const Channel *field = get_channel(sample->channelId);
		json_string(serial, "nm", field->label, 1);
		json_string(serial, "ut", field->units, 1);
		json_int(serial, "sr", decodeSampleRate(LOWER_SAMPLE_RATE(sample->sampleRate, sampleRateLimit)), 0);
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
	json_objEnd(serial, 0);
	return API_SUCCESS_NO_RETURN;
}

void api_sendSampleRecord(Serial *serial, ChannelSample *channelSamples, size_t channelCount, unsigned int tick, int sendMeta){
	json_messageStart(serial);
	json_objStart(serial, "s");

	json_uint(serial,"t",tick,1);
	if (sendMeta) writeSampleMeta(serial, channelSamples, channelCount, getConnectivitySampleRateLimit(), 1);

	unsigned int channelsBitmask = 0;
	json_arrayStart(serial, "d");
	ChannelSample *sample = channelSamples;
	for (size_t i = 0; i < channelCount; i++){
		const Channel *channel = get_channel(sample->channelId);
		if (NIL_SAMPLE != sample->intValue){
			channelsBitmask = channelsBitmask | (1 << i);
			int precision = channel->precision;
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

	json_objEnd(serial, 0);
	json_objEnd(serial, 0);
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

			if (NAME_EQU("id", name)) channelCfg->channeId = filter_channel_id(modp_atoi(value));
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
	if (NAME_EQU("scalMod", name)) adcCfg->scalingMode = filterAnalogScalingMode(modp_atoi(value));
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
	configChanged();
	return API_SUCCESS;
}

static void json_channelConfig(Serial *serial, ChannelConfig *cfg, int more){
	json_int(serial, "id", cfg->channeId, 1);
	json_int(serial, "sr", decodeSampleRate(cfg->sampleRate), more);
}

static void sendAnalogConfig(Serial *serial, size_t startIndex, size_t endIndex){

	json_messageStart(serial);
	json_objStart(serial, "analogCfg");
	for (size_t i = startIndex; i <= endIndex; i++){

		ADCConfig *cfg = &(getWorkingLoggerConfig()->ADCConfigs[i]);
		json_objStartInt(serial, i);
		json_channelConfig(serial, &(cfg->cfg), 1);
		json_int(serial, "scalMod", cfg->scalingMode, 1);
		json_float(serial, "linScal", cfg->linearScaling, LINEAR_SCALING_PRECISION, 1);

		json_objStart(serial, "map");
		json_arrayStart(serial, "raw");

		for (size_t b = 0; b < ANALOG_SCALING_BINS; b++){
			put_int(serial,  cfg->scalingMap.rawValues[b]);
			if (b < ANALOG_SCALING_BINS - 1) serial->put_c(',');
		}
		json_arrayEnd(serial, 1);
		json_arrayStart(serial, "scal");
		for (size_t b = 0; b < ANALOG_SCALING_BINS; b++){
			put_float(serial, cfg->scalingMap.scaledValues[b], DEFAULT_ANALOG_SCALING_PRECISION);
			if (b < ANALOG_SCALING_BINS - 1) serial->put_c(',');
		}
		json_arrayEnd(serial, 0);
		json_objEnd(serial, 0); //map
		json_objEnd(serial, i != endIndex); //index
	}
	json_objEnd(serial, 0);
	json_objEnd(serial, 0);
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
	else if (NAME_EQU("chan",name)) accelCfg->physicalChannel = filterAccelChannel(modp_atoi(value));
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
	configChanged();
	return API_SUCCESS;
}

static void sendAccelConfig(Serial *serial, size_t startIndex, size_t endIndex){
	json_messageStart(serial);
	json_objStart(serial, "accelCfg");
	for (size_t i = startIndex; i <= endIndex; i++){
		AccelConfig *cfg = &(getWorkingLoggerConfig()->AccelConfigs[i]);
		json_objStartInt(serial, i);
		json_channelConfig(serial, &(cfg->cfg), 1);
		json_uint(serial, "mode", cfg->mode, 1);
		json_uint(serial, "chan", cfg->physicalChannel, 1);
		json_uint(serial, "zeroVal", cfg->zeroValue, 0);
		json_objEnd(serial, i != endIndex); //index
	}
	json_objEnd(serial, 0);
	json_objEnd(serial, 0);
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
	json_objStart(serial, "cellCfg");
	json_string(serial, "apnHost", cfg->apnHost, 1);
	json_string(serial, "apnUser", cfg->apnUser, 1);
	json_string(serial, "apnPass", cfg->apnPass, 0);
	json_objEnd(serial, 0);
	json_objEnd(serial, 0);
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
	configChanged();
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
	configChanged();
	return API_SUCCESS;
}

int api_getBluetoothConfig(Serial *serial, const jsmntok_t *json){
	BluetoothConfig *cfg = &(getWorkingLoggerConfig()->ConnectivityConfigs.bluetoothConfig);
	json_messageStart(serial);
	json_objStart(serial, "btCfg");
	json_string(serial, "name", cfg->deviceName, 1);
	json_string(serial, "pass", cfg->passcode, 0);
	json_objEnd(serial, 0);
	json_objEnd(serial, 0);
	return API_SUCCESS_NO_RETURN;
}

int api_getLogfile(Serial *serial, const jsmntok_t *json){
	json_messageStart(serial);
	json_valueStart(serial, "logfile");
	serial->put_c('"');
	read_log_to_serial(serial, 1);
	serial->put_c('"');
	json_objEnd(serial,0);
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
	configChanged();
	return API_SUCCESS;
}

int api_getConnectivityConfig(Serial *serial, const jsmntok_t *json){
	ConnectivityConfig *cfg = &(getWorkingLoggerConfig()->ConnectivityConfigs);
	json_messageStart(serial);
	json_objStart(serial, "connCfg");
	json_int(serial, "sdMode", cfg->sdLoggingMode, 1);
	json_int(serial, "connMode", cfg->connectivityMode, 1);
	json_int(serial, "bgStream", cfg->backgroundStreaming, 0);
	json_objEnd(serial, 0);
	json_objEnd(serial, 0);
	return API_SUCCESS_NO_RETURN;
}

static void sendPwmConfig(Serial *serial, size_t startIndex, size_t endIndex){

	json_messageStart(serial);
	json_objStart(serial, "pwmCfg");
	for (size_t i = startIndex; i <= endIndex; i++){
		PWMConfig *cfg = &(getWorkingLoggerConfig()->PWMConfigs[i]);
		json_objStartInt(serial, i);
		json_channelConfig(serial, &(cfg->cfg), 1);
		json_uint(serial, "outMode", cfg->outputMode, 1);
		json_uint(serial, "logMode", cfg->loggingMode, 1);
		json_uint(serial, "stDutyCyc", cfg->startupDutyCycle, 1);
		json_uint(serial, "stPeriod", cfg->startupPeriod, 1);
		json_float(serial, "vScal", cfg->voltageScaling, DEFAULT_VOLTAGE_SCALING_PRECISION, 0);
		json_objEnd(serial, i != endIndex); //index
	}
	json_objEnd(serial, 0);
	json_objEnd(serial, 0);
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

	if (NAME_EQU("outMode", name)) pwmCfg->outputMode = filterPwmOutputMode(modp_atoi(value));
	if (NAME_EQU("logMode", name)) pwmCfg->loggingMode = filterPwmLoggingMode(modp_atoi(value));
	if (NAME_EQU("stDutyCyc", name)) pwmCfg->startupDutyCycle = filterPwmDutyCycle(modp_atoi(value));
	if (NAME_EQU("stPeriod", name)) pwmCfg->startupPeriod = filterPwmPeriod(modp_atoi(value));
	if (NAME_EQU("vScal", name)) pwmCfg->voltageScaling = modp_atof(value);
	return valueTok + 1;
}

int api_setPwmConfig(Serial *serial, const jsmntok_t *json){
	setMultiChannelConfigGeneric(serial, json, getPwmConfigs, setPwmExtendedField);
	configChanged();
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
	json_objStart(serial, "gpioCfg");
	for (size_t i = startIndex; i <= endIndex; i++){
		GPIOConfig *cfg = &(getWorkingLoggerConfig()->GPIOConfigs[i]);
		json_objStartInt(serial, i);
		json_channelConfig(serial, &(cfg->cfg), 1);
		json_uint(serial, "mode", cfg->mode, 0);
		json_objEnd(serial, i != endIndex);
	}
	json_objEnd(serial, 0);
	json_objEnd(serial, 0);
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
	configChanged();
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
	json_objStart(serial, "timerCfg");
	for (size_t i = startIndex; i <= endIndex; i++){
		TimerConfig *cfg = &(getWorkingLoggerConfig()->TimerConfigs[i]);
		json_objStartInt(serial, i);
		json_channelConfig(serial, &(cfg->cfg), 1);
		json_uint(serial, "sTimer", cfg->slowTimerEnabled, 1);
		json_uint(serial, "mode", cfg->mode, 1);
		json_uint(serial, "ppRev", cfg->pulsePerRevolution, 1);
		json_uint(serial, "timDiv", cfg->timerDivider, 0);
		json_objEnd(serial, i != endIndex);
	}
	json_objEnd(serial, 0);
	json_objEnd(serial, 0);
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
	configChanged();
	return API_SUCCESS;
}

int api_getGpsConfig(Serial *serial, const jsmntok_t *json){

	GPSConfig *gpsCfg = &(getWorkingLoggerConfig()->GPSConfigs);

	json_messageStart(serial);
	json_objStart(serial, "gpsCfg");

	json_int(serial, "sr", decodeSampleRate(gpsCfg->sampleRate), 1);

	json_int(serial, "pos", gpsCfg->positionEnabled, 1);
	json_int(serial, "speed", gpsCfg->speedEnabled, 1);
	json_int(serial, "time", gpsCfg->timeEnabled, 1);
	json_int(serial, "dist", gpsCfg->distanceEnabled, 1);
	json_int(serial, "sats", gpsCfg->satellitesEnabled, 0);

	json_objEnd(serial, 0);
	json_objEnd(serial, 0);
	return API_SUCCESS_NO_RETURN;
}

int api_setGpsConfig(Serial *serial, const jsmntok_t *json){

	GPSConfig *gpsCfg = &(getWorkingLoggerConfig()->GPSConfigs);
	int sr = 0;
	setIntValueIfExists(json, "sr", &sr);
	if (sr) gpsCfg->sampleRate = encodeSampleRate(sr);
	setUnsignedCharValueIfExists(json, "pos", &gpsCfg->positionEnabled);
	setUnsignedCharValueIfExists(json, "speed", &gpsCfg->speedEnabled);
	setUnsignedCharValueIfExists(json, "time", &gpsCfg->timeEnabled);
	setUnsignedCharValueIfExists(json, "dist", &gpsCfg->distanceEnabled);
	setUnsignedCharValueIfExists(json, "sats", &gpsCfg->satellitesEnabled);

	configChanged();
	return API_SUCCESS;
}

int api_setLapConfig(Serial *serial, const jsmntok_t *json){
	LapConfig *lapCfg = &(getWorkingLoggerConfig()->LapConfigs);

	const jsmntok_t *lapCount = findNode(json, "lapCount");
	if (lapCount != NULL) setChannelConfig(serial, lapCount + 1, &lapCfg->lapCountCfg, NULL, NULL);

	const jsmntok_t *lapTime = findNode(json, "lapTime");
	if (lapTime != NULL) setChannelConfig(serial, lapTime + 1, &lapCfg->lapTimeCfg, NULL, NULL);

	const jsmntok_t *predTime = findNode(json, "predTime");
	if (predTime != NULL) setChannelConfig(serial, predTime + 1, &lapCfg->predTimeCfg, NULL, NULL);

	configChanged();
	return API_SUCCESS;
}

int api_getLapConfig(Serial *serial, const jsmntok_t *json){
	LapConfig *lapCfg = &(getWorkingLoggerConfig()->LapConfigs);

	json_messageStart(serial);
	json_objStart(serial, "lapCfg");

	json_objStart(serial, "lapCount");
	json_channelConfig(serial, &lapCfg->lapCountCfg, 0);
	json_objEnd(serial, 1);

	json_objStart(serial, "lapTime");
	json_channelConfig(serial, &lapCfg->lapTimeCfg, 0);
	json_objEnd(serial, 1);

	json_objStart(serial, "predTime");
	json_channelConfig(serial, &lapCfg->predTimeCfg, 0);
	json_objEnd(serial, 0);

	json_objEnd(serial, 0);
	json_objEnd(serial, 0);
	return API_SUCCESS_NO_RETURN;
}

static void json_track(Serial *serial, const Track *track){
	json_arrayStart(serial, "sf");
	json_arrayElementFloat(serial, track->startFinish.latitude, DEFAULT_GPS_POSITION_PRECISION, 1);
	json_arrayElementFloat(serial, track->startFinish.longitude, DEFAULT_GPS_POSITION_PRECISION, 0);
	json_arrayEnd(serial, 1);
	json_arrayStart(serial, "sec");
	for (size_t i = 0; i < SECTOR_COUNT; i++){
		const GeoPoint *p = &track->sectors[i];
		json_arrayStart(serial, NULL);
		json_arrayElementFloat(serial, p->latitude, DEFAULT_GPS_POSITION_PRECISION, 1);
		json_arrayElementFloat(serial, p->longitude, DEFAULT_GPS_POSITION_PRECISION, 0);
		json_arrayEnd(serial, i < SECTOR_COUNT - 1);
	}
	json_arrayEnd(serial, 0);
}

int api_getTrackConfig(Serial *serial, const jsmntok_t *json){
	TrackConfig *trackCfg = &(getWorkingLoggerConfig()->TrackConfigs);

	json_messageStart(serial);
	json_objStart(serial, "trackCfg");
	json_float(serial, "rad", trackCfg->track.radius, DEFAULT_GPS_RADIUS_PRECISION, 1);
	json_objStart(serial, "track");
	json_track(serial, &trackCfg->track);
	json_objEnd(serial, 0);
	json_objEnd(serial, 0);
	json_objEnd(serial, 0);

	return API_SUCCESS_NO_RETURN;
}

void setTrack(const jsmntok_t *cfg, Track *track){
	{
		const jsmntok_t *startFinish = findNode(cfg, "sf");
		if (startFinish != NULL){
			const jsmntok_t *sfValues = startFinish + 1;
			if (sfValues != NULL && sfValues->type == JSMN_ARRAY && sfValues->size == 2){
				sfValues++;
				if (sfValues->type == JSMN_PRIMITIVE){
					jsmn_trimData(sfValues);
					track->startFinish.latitude = modp_atof(sfValues->data);
				}
				sfValues++;
				if (sfValues->type == JSMN_PRIMITIVE){
					jsmn_trimData(sfValues);
					track->startFinish.longitude = modp_atof(sfValues->data);
				}
			}
		}
	}
	setFloatValueIfExists(cfg, "rad", &track->radius);
}

int api_setTrackConfig(Serial *serial, const jsmntok_t *json){

	TrackConfig *trackCfg = &(getWorkingLoggerConfig()->TrackConfigs);

	const jsmntok_t *track = findNode(json, "track");
	if (track != NULL) setTrack(track + 1, &trackCfg->track);

	configChanged();

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

int api_getTracks(Serial *serial, const jsmntok_t *json){
	const Tracks * tracks = get_tracks();

	json_messageStart(serial);
	json_objStart(serial, "tracks");
	size_t track_count = tracks->count;
	json_int(serial,"size", track_count, 1);
	for (size_t track_index = 0; track_index < track_count; track_index++){
		const Track *track = tracks->tracks + track_index;
		json_objStartInt(serial, track_index);
		json_track(serial, track);
		json_objEnd(serial, track_index < track_count - 1);
	}
	json_objEnd(serial, 0);
	json_objEnd(serial, 0);
	return API_SUCCESS_NO_RETURN;
}

int api_getChannels(Serial *serial, const jsmntok_t *json){

	const Channels *channelsInfo = get_channels();

	json_messageStart(serial);
	json_objStart(serial, "channels");
	size_t channels_count = channelsInfo->count;
	json_int(serial, "size", channels_count, 1);
	for (size_t channel_index = 0; channel_index < channels_count; channel_index++){
		const Channel *channel = channelsInfo->channels + channel_index;
		json_objStartInt(serial, channel_index);
		json_string(serial, "nm", channel->label, 1);
		json_string(serial, "ut", channel->units, 1);
		json_int(serial, "prec", channel->precision, 0);
		json_objEnd(serial, channel_index < channels_count - 1);
	}
	json_objEnd(serial, 0);
	json_objEnd(serial, 0);
	return API_SUCCESS_NO_RETURN;
}
