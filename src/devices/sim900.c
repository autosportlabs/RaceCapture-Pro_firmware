#include "sim900.h"
#include "cellModem.h"
#include "loggerConfig.h"
#include "mod_string.h"
#include "race_capture/printk.h"

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

int sim900_init_connection(DeviceConfig *config){
	LoggerConfig *loggerConfig = getWorkingLoggerConfig();
	Serial *serial = config->serial;

	int initResult = DEVICE_INIT_FAIL;
	if (0 == initCellModem(serial)){
		CellularConfig *cellCfg = &(loggerConfig->ConnectivityConfigs.cellularConfig);
		TelemetryConfig *telemetryConfig = &(loggerConfig->ConnectivityConfigs.telemetryConfig);
		if (0 == configureNet(serial, cellCfg->apnHost, cellCfg->apnUser, cellCfg->apnPass)){
			if( 0 == connectNet(serial, telemetryConfig->telemetryServerHost,"8080",0)){
				if (0 == writeAuthJSON(serial, telemetryConfig->telemetryDeviceId)){
					initResult = DEVICE_INIT_SUCCESS;
				}
				else{
					pr_error("error auth- token: ");
					pr_error(telemetryConfig->telemetryDeviceId);
					pr_error("\n");
				}
			}
			else{
				pr_error("Failed to connect server: ");
				pr_error(telemetryConfig->telemetryServerHost);
				pr_error("\n");
			}
		}
		else{
			pr_error("Failed to configure network\n");
		}
	}
	else{
		pr_warning("Failed to init cell connection\n");
	}
	return initResult;
}

int sim900_check_connection_status(DeviceConfig *config){
	return DEVICE_STATUS_NO_ERROR;
}


