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
typedef void (*setExtField_func)(const char *name, const char *value, void *cfg);

void api_sampleData(Serial *serial, const jsmntok_t *json){


}

void api_getCellConfig(Serial *serial, const jsmntok_t *json){
	CellularConfig *c = &(getWorkingLoggerConfig()->ConnectivityConfigs.cellularConfig);
	put_nameString(serial, "apnHost", c->apnHost);
	put_nameString(serial, "apnUser", c->apnUser);
	put_nameString(serial, "apnPass", c->apnPass);
}

static void setChannelConfig(Serial *serial, const jsmntok_t *cfg, ChannelConfig *channelCfg, setExtField_func setExtField, void *extCfg){
	if (cfg->type == JSMN_OBJECT && cfg->size % 2 == 0){
		for (int i = 1; i <= cfg->size; i += 2 ){
			const jsmntok_t *nameTok = cfg + i;
			jsmn_trimData(nameTok);
			const jsmntok_t *valueTok = cfg + i + 1;
			if (valueTok->type == JSMN_PRIMITIVE || valueTok->type == JSMN_STRING) jsmn_trimData(valueTok);

			const char *name = nameTok->data;
			const char *value = valueTok->data;

			if (NAME_EQU("nm",name)) setLabelGeneric(channelCfg->label, value);
			else if (NAME_EQU("ut", name)) setLabelGeneric(channelCfg->units, value);
			else if (NAME_EQU("sr", name)) channelCfg->sampleRate = encodeSampleRate(modp_atoi(value));

			else setExtField(name, value, extCfg);
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
//			void *baseCfg = getConfigFunc(id);
			//ChannelConfig *channelCfg = getChannelConfigFunc(id);
			setChannelConfig(serial, cfgTok, channelCfg, setExtFieldFunc, baseCfg);
		}
	}
}

static void getAnalogConfigs(size_t channelId, void ** baseCfg, ChannelConfig ** channelCfg){
	ADCConfig *c =&(getWorkingLoggerConfig()->ADCConfigs[channelId]);
	*baseCfg = c;
	*channelCfg = &c->cfg;
}

static void setAnalogExtendedField(const char *name, const char *value, void *cfg){
	ADCConfig *adcCfg = (ADCConfig *)cfg;
	if (NAME_EQU("prec",name)) adcCfg->loggingPrecision = modp_atoi(value);
	else if (NAME_EQU("scalMod", name)) adcCfg->scalingMode = filterAnalogScalingMode(modp_atoi(value));
	else if (NAME_EQU("linScal", name)) adcCfg->linearScaling = modp_atof(value);
	else if (NAME_EQU("scalMap", name)) {/*todo scaling map*/ }
}

void api_setAnalogConfig(Serial *serial, const jsmntok_t * json){
	setMultiChannelConfigGeneric(serial, json, getAnalogConfigs, setAnalogExtendedField);
}
