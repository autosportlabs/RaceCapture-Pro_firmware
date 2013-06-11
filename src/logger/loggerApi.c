/*
 * loggerApi.c
 *
 *  Created on: Jun 10, 2013
 *      Author: brent
 */
#include "loggerApi.h"
#include "loggerConfig.h"

void api_sample_data(Serial *serial, const jsmntok_t *json){


}

void api_get_cell_config(Serial *serial, const jsmntok_t *json){
	CellularConfig *c = &(getWorkingLoggerConfig()->ConnectivityConfigs.cellularConfig);
	put_nameString(serial, "apnHost", c->apnHost);
	put_nameString(serial, "apnUser", c->apnUser);
	put_nameString(serial, "apnPass", c->apnPass);

}


