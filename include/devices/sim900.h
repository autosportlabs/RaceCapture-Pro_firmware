/*
 * null_device.h
 *
 *  Created on: Oct 29, 2013
 *      Author: brent
 */

#ifndef SIM900_H_
#define SIM900_H_

#include "stddef.h"
#include "devices_common.h"

typedef enum {
    TELEMETRY_STATUS_IDLE = 0,
    TELEMETRY_STATUS_CONNECTED,
    TELEMETRY_STATUS_CURRENT_CONNECTION_TERMINATED,
    TELEMETRY_STATUS_REJECTED_DEVICE_ID,
    TELEMETRY_STATUS_DATA_PLAN_NOT_AVAILABLE,
    TELEMETRY_STATUS_SERVER_CONNECTION_FAILED,
    TELEMETRY_STATUS_INTERNET_CONFIG_FAILED,
    TELEMETRY_STATUS_CELL_REGISTRATION_FAILED
} telemetry_status_t;

telemetry_status_t sim900_get_connection_status();
int32_t sim900_active_time();
int sim900_disconnect(DeviceConfig *config);
int sim900_init_connection(DeviceConfig *config);
int sim900_check_connection_status(DeviceConfig *config);

#endif /* SIM900_H_ */
