#include "sim900.h"
#include "cellModem.h"
#include "loggerConfig.h"
#include "mod_string.h"
#include "printk.h"
#include "LED.h"
#include <stdio.h>

#define TELEMETRY_SERVER_PORT "8080"


int sim900_init_connection(DeviceConfig *config){
   printf("\r\nsim900_init_connection");
	return  DEVICE_INIT_SUCCESS;
}

int sim900_check_connection_status(DeviceConfig *config){
	return DEVICE_STATUS_NO_ERROR;
}


