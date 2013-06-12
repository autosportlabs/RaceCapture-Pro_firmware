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

static int getNumberField(const jsmntok_t *parent, const char *name, int *value){

	for (int i = 1; i <= parent->size; i++){
		const jsmntok_t *child = &parent[i];
		if (child->type == JSMN_PRIMITIVE && strcmp(name,child->data)){

		}
	}

	return 0;
}

void api_sampleData(Serial *serial, const jsmntok_t *json){


}

void api_getCellConfig(Serial *serial, const jsmntok_t *json){
	CellularConfig *c = &(getWorkingLoggerConfig()->ConnectivityConfigs.cellularConfig);
	put_nameString(serial, "apnHost", c->apnHost);
	put_nameString(serial, "apnUser", c->apnUser);
	put_nameString(serial, "apnPass", c->apnPass);
}

static void setAnalogConfigChannel(Serial *serial, int id, const jsmntok_t *cfg){
	ADCConfig *c = &(getWorkingLoggerConfig()->ADCConfigs[id]);
	int size = cfg->size;
	if (cfg->type == JSMN_OBJECT && cfg->size % 2 == 0){
		for (int i = 1; i <= cfg->size; i += 2 ){
			const jsmntok_t *name = cfg + i;
			jsmn_trimData(name);
			const jsmntok_t *value = cfg + i + 1;
			jsmn_trimData(value);
		}
	}
}

void api_setAnalogConfig(Serial *serial, const jsmntok_t * json){
	if (json->type == JSMN_OBJECT && json->size % 2 == 0){
		for (int i = 1; i <= json->size; i += 2){
			const jsmntok_t *idTok = json + i;
			const jsmntok_t *cfgTok = json + i + 1;
			jsmn_trimData(idTok);
			int id = modp_atoi(idTok->data);
			setAnalogConfigChannel(serial, id, cfgTok);
		}
	}
}
