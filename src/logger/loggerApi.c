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

#define NAME_EQU(A, B) (strcmp(A, B) == 0)

typedef void (*getConfigs_func)(size_t channelId, void ** baseCfg, ChannelConfig ** channelCfg);
typedef const jsmntok_t * (*setExtField_func)(const jsmntok_t *json, const char *name, const char *value, void *cfg);


void api_sampleData(Serial *serial, const jsmntok_t *json){


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

void api_setAnalogConfig(Serial *serial, const jsmntok_t * json){
	setMultiChannelConfigGeneric(serial, json, getAnalogConfigs, setAnalogExtendedField);
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

void api_setAccelConfig(Serial *serial, const jsmntok_t *json){
	setMultiChannelConfigGeneric(serial, json, getAccelConfigs, setAccelExtendedField);
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

void api_getCellConfig(Serial *serial, const jsmntok_t *json){
	CellularConfig *c = &(getWorkingLoggerConfig()->ConnectivityConfigs.cellularConfig);
/*	put_nameString(serial, "apnHost", c->apnHost);
	put_nameString(serial, "apnUser", c->apnUser);
	put_nameString(serial, "apnPass", c->apnPass);
	*/
}

static const jsmntok_t * setCellExtendedField(const jsmntok_t *valueTok, const char *name, const char *value, void *cfg){
	CellularConfig *cellCfg = (CellularConfig *)cfg;
	if (NAME_EQU("apnHost", name))  setTextField(cellCfg->apnHost, value, CELL_APN_HOST_LENGTH);
	else if (NAME_EQU("apnUser", name)) setTextField(cellCfg->apnUser, value, CELL_APN_USER_LENGTH);
	else if (NAME_EQU("apnPass", name)) setTextField(cellCfg->apnPass, value, CELL_APN_PASS_LENGTH);
	return valueTok + 1;
}

void api_setCellConfig(Serial *serial, const jsmntok_t *json){
	CellularConfig *cfg = &(getWorkingLoggerConfig()->ConnectivityConfigs.cellularConfig);
	setConfigGeneric(serial, json, cfg, setCellExtendedField);
}
