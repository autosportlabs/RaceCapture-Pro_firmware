#include "sim900.h"
#include "cellModem.h"
#include "loggerConfig.h"
#include "mod_string.h"

static int writeAuthJSON(Serial *serial, const char *deviceId){
	//send linefeed at slow intervals until we have the auth packet ack from server
	for (int i = 0; i < 5; i++){
		putsCell(serial, " ");
		vTaskDelay(84); //250ms pause
	}
	putsCell(serial, "{\"cmd\":{\"auth\":{\"deviceId\":\"");
	putsCell(serial, deviceId);
	putsCell(serial, "\"}}}\n");

	int attempts = 20;
	while (attempts-- > 0){
		const char * data = readsCell(serial, 334); //~1000ms
		if (strncmp(data, "{\"status\":\"ok\"}",15) == 0) return 0;
	}
	return -1;
}

static void initCell(DeviceConfig *config){

	LoggerConfig *loggerConfig = getWorkingLoggerConfig();
	Serial *serial = config->serial;

	int cellReady = 0;
	while (cellReady == 0){
		if (0 == initCellModem(serial)){
			CellularConfig *cellCfg = &(loggerConfig->ConnectivityConfigs.cellularConfig);
			if (0 == configureNet(serial, cellCfg->apnHost, cellCfg->apnUser, cellCfg->apnPass)){
				cellReady = 1;
			}
		}
	}
}

int sim900_init_connection(DeviceConfig *config){
	return DEVICE_INIT_SUCCESS;
}

int sim900_check_connection_status(DeviceConfig *config){
	return DEVICE_STATUS_NO_ERROR;
}


