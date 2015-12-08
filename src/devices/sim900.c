/*
 * Race Capture Firmware
 *
 * Copyright (C) 2016 Autosport Labs
 *
 * This file is part of the Race Capture firmware suite
 *
 * This is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details. You should
 * have received a copy of the GNU General Public License along with
 * this code. If not, see <http://www.gnu.org/licenses/>.
 */

#include "sim900.h"
#include "cellModem.h"
#include "loggerConfig.h"
#include "mod_string.h"
#include "printk.h"
#include "LED.h"
#include "dateTime.h"
#include "taskUtil.h"
#include "api.h"
#include "capabilities.h"
#include "constants.h"
#include "cpu.h"

#define TELEMETRY_SERVER_PORT "8080"

static telemetry_status_t g_connection_status = TELEMETRY_STATUS_IDLE;
static int32_t g_active_since = 0;

telemetry_status_t cellular_get_connection_status()
{
    return g_connection_status;
}

int32_t cellular_active_time()
{
    if (g_active_since) {
        int uptime = getUptimeAsInt();
        int duration = uptime - g_active_since;
        return duration;
    }
    return 0;
}

static int writeAuthJSON(Serial *serial, const char *deviceId)
{
    //send linefeed at slow intervals until we have the auth packet ack from server
    for (int i = 0; i < 5; i++) {
        serial->put_s(" ");
        delayMs(250);
    }
    json_objStart(serial);
    json_objStartString(serial, "auth");
    json_string(serial, "deviceId", deviceId, 1);
    json_int(serial, "apiVer", API_REV, 1);
    json_string(serial, "device", DEVICE_NAME, 1);
    json_string(serial, "ver", VERSION_STR, 1);
    json_string(serial, "sn", cpu_get_serialnumber(), 0);
    json_objEnd(serial, 0);
    json_objEnd(serial, 0);
    serial->put_c('\n');

    pr_debug_str_msg("sending auth- deviceId: ", deviceId);

    int attempts = 20;
    while (attempts-- > 0) {
        const char * data = readsCell(serial, 1000);
        if (strncmp(data, "{\"status\":\"ok\"}",15) == 0) return 0;
    }
    return -1;
}

int cellular_disconnect(DeviceConfig *config)
{
    setCellBuffer(config->buffer, config->length);
    g_connection_status = TELEMETRY_STATUS_IDLE;
    pr_info("cell: disconnected\r\n");
    return closeNet(config->serial);
}

int cellular_init_connection(DeviceConfig *config)
{
    LoggerConfig *loggerConfig = getWorkingLoggerConfig();
    setCellBuffer(config->buffer, config->length);
    Serial *serial = config->serial;

	pr_debug("init cell connection\r\n");
	int initResult = DEVICE_INIT_FAIL;
	g_active_since = 0;
    CellularConfig *cellCfg = &(loggerConfig->ConnectivityConfigs.cellularConfig);
    TelemetryConfig *telemetryConfig = &(loggerConfig->ConnectivityConfigs.telemetryConfig);
	if (0 == initCellModem(serial, cellCfg)){
		if (0 == configureNet(serial)){
			pr_info("cell: network configured\r\n");
			if( 0 == connectNet(serial, telemetryConfig->telemetryServerHost, TELEMETRY_SERVER_PORT, 0)){
				pr_info("cell: server connected\r\n");
				if (0 == writeAuthJSON(serial, telemetryConfig->telemetryDeviceId)){
					pr_info("cell: server authenticated\r\n");
					initResult = DEVICE_INIT_SUCCESS;
					g_connection_status = TELEMETRY_STATUS_CONNECTED;
					g_active_since = getUptimeAsInt();
				}
				else{
					g_connection_status = TELEMETRY_STATUS_REJECTED_DEVICE_ID;
					pr_error_str_msg("err: auth- token: ", telemetryConfig->telemetryDeviceId);
				}
			}
			else{
				g_connection_status = TELEMETRY_STATUS_SERVER_CONNECTION_FAILED;
				pr_error_str_msg("err: server connect ", telemetryConfig->telemetryServerHost);
			}
		}
		else{
			g_connection_status = TELEMETRY_STATUS_INTERNET_CONFIG_FAILED;
			pr_error("Failed to configure network\r\n");
		}
	}
	else{
		g_connection_status = TELEMETRY_STATUS_CELL_REGISTRATION_FAILED;
		pr_warning("Failed to init cell connection\r\n");
	}
	return initResult;
}

int cellular_check_connection_status(DeviceConfig *config)
{
    setCellBuffer(config->buffer, config->length);
    int status = isNetConnectionErrorOrClosed() ? DEVICE_STATUS_DISCONNECTED : DEVICE_STATUS_NO_ERROR;
    if (status == DEVICE_STATUS_DISCONNECTED) {
        g_connection_status = TELEMETRY_STATUS_CURRENT_CONNECTION_TERMINATED;
        pr_debug("cell disconnected\r\n");

    }
    return status;
}
