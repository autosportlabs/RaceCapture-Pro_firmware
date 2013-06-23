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
#include "loggerData.h"

#define NAME_EQU(A, B) (strcmp(A, B) == 0)

typedef void (*getConfigs_func)(size_t channelId, void ** baseCfg, ChannelConfig ** channelCfg);
typedef const jsmntok_t * (*setExtField_func)(const jsmntok_t *json, const char *name, const char *value, void *cfg);



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
	SampleRecord sr;
	LoggerConfig * config = getWorkingLoggerConfig();
	initSampleRecord(config, &sr);
	populateSampleRecord(&sr,0,config);
	writeSampleRecord(serial, &sr, sendMeta);
	return API_SUCCESS_NO_RETURN;
}

void writeSampleRecord(Serial *serial, SampleRecord *sr, int sendMeta){

	json_asyncMessageStart(serial);

	if (sendMeta){
		json_arrayStart(serial, "m", 1);
		for (int i = 0; i < SAMPLE_RECORD_CHANNELS; i++){
			int more = i < SAMPLE_RECORD_CHANNELS - 1;
			ChannelSample *sample = &(sr->Samples[i]);
			ChannelConfig * channelConfig = sample->channelConfig;
			if (SAMPLE_DISABLED == channelConfig->sampleRate) continue;
			serial->put_c('{');
			json_string(serial, "nm", channelConfig->label, 1);
			json_string(serial, "ut", channelConfig->units, 1);
			json_int(serial, "sr", channelConfig->sampleRate, 0);
			serial->put_c('}');
			if (more) serial->put_c(',');
		}
		json_arrayEnd(serial, 1);
	}

	json_arrayStart(serial, "s",1);

	for (int i = 0; i < SAMPLE_RECORD_CHANNELS; i++){
		int more = i < SAMPLE_RECORD_CHANNELS - 1;
		ChannelSample *sample = &(sr->Samples[i]);
		ChannelConfig * channelConfig = sample->channelConfig;

		if (SAMPLE_DISABLED == channelConfig->sampleRate) continue;
		if (sample->intValue == NIL_SAMPLE){
			serial->put_s("null");
		}
		else{
			int precision = sample->precision;
			if (precision > 0){
				put_float(serial, sample->floatValue,precision);
			}
			else{
				put_int(serial, sample->intValue);
			}
		}
		if (more) serial->put_c(',');
	}
	json_arrayEnd(serial, 0);
	json_blockEnd(serial, 0);
}

static void setChannelConfig(Serial *serial, const jsmntok_t *cfg, ChannelConfig *channelCfg, setExtField_func setExtField, void *extCfg){
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

			const char *name = nameTok->data;
			const char *value = valueTok->data;

			if (NAME_EQU("nm",name)) setLabelGeneric(channelCfg->label, value);
			else if (NAME_EQU("ut", name)) setLabelGeneric(channelCfg->units, value);
			else if (NAME_EQU("sr", name)) channelCfg->sampleRate = encodeSampleRate(modp_atoi(value));

			else cfg = setExtField(valueTok, name, value, extCfg);
		}
	}
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
	CellularConfig *c = &(getWorkingLoggerConfig()->ConnectivityConfigs.cellularConfig);
/*	put_nameString(serial, "apnHost", c->apnHost);
	put_nameString(serial, "apnUser", c->apnUser);
	put_nameString(serial, "apnPass", c->apnPass);
	*/
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
	if (NAME_EQU("btName", name)) setTextField(btCfg->deviceName, value, BT_DEVICE_NAME_LENGTH);
	else if (NAME_EQU("btPass", name)) setTextField(btCfg->passcode, value, BT_PASSCODE_LENGTH);
	return valueTok + 1;
}

int api_setBluetoothConfig(Serial *serial, const jsmntok_t *json){
	BluetoothConfig *cfg = &(getWorkingLoggerConfig()->ConnectivityConfigs.bluetoothConfig);
	setConfigGeneric(serial, json, cfg, setBluetoothExtendedField);
	return API_SUCCESS;
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

int api_setTimerConfig(Serial *serial, const jsmntok_t *json){
	setMultiChannelConfigGeneric(serial, json, getTimerConfigs, setTimerExtendedField);
	return API_SUCCESS;
}
