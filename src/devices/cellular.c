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

#include "LED.h"
#include "api.h"
#include "capabilities.h"
#include "cellular.h"
#include "constants.h"
#include "cpu.h"
#include "dateTime.h"
#include "loggerConfig.h"
#include "mod_string.h"
#include "printk.h"
#include "sim900.h"
#include "taskUtil.h"

#define TELEMETRY_SERVER_PORT "8080"

static struct {
        telemetry_status_t status;
        tiny_millis_t active_since;
} telemetry_info;

static struct cellular_info cell_info;

enum cellmodem_status cellmodem_get_status( void )
{
        return cell_info.status;
}

int cell_get_signal_strength()
{
        return cell_info.signal;
}

const char* cell_get_subscriber_number()
{
        return (const char*) cell_info.number;
}

const char* cell_get_IMEI()
{
        return (const char*) cell_info.imei;
}

telemetry_status_t cellular_get_connection_status()
{
        return telemetry_info.status;
}

int32_t cellular_active_time()
{
        const tiny_millis_t since = telemetry_info.active_since;
        return 0 == since ? 0 : (int) getUptime() - since;
}

static int writeAuthJSON(Serial *serial, const char *deviceId)
{
        /*
         * Send linefeed at slow intervals until we have the auth packet
         * ack from server.
         * XXX STIEG: WTF!!!  What the hell is the point of this?
         */
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
        const char* ok_resp = "{\"status\":\"ok\"}";
        while (attempts-- > 0) {
                const char * data = readsCell(serial, 1000);
                if (0 == strncmp(data, ok_resp, sizeof(ok_resp) - 1))
                        return 0;
        }

        return -1;
}

int cellular_disconnect(DeviceConfig *config)
{
    setCellBuffer(config->buffer, config->length);
    telemetry_info.status = TELEMETRY_STATUS_IDLE;
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
	telemetry_info.active_since = 0;
    CellularConfig *cellCfg = &(loggerConfig->ConnectivityConfigs.cellularConfig);
    TelemetryConfig *telemetryConfig = &(loggerConfig->ConnectivityConfigs.telemetryConfig);
    if (0 == initCellModem(serial, cellCfg, &cell_info)){
		if (0 == configureNet(serial)){
			pr_info("cell: network configured\r\n");
			if( 0 == connectNet(serial, telemetryConfig->telemetryServerHost, TELEMETRY_SERVER_PORT, 0)){
				pr_info("cell: server connected\r\n");
				if (0 == writeAuthJSON(serial, telemetryConfig->telemetryDeviceId)){
					pr_info("cell: server authenticated\r\n");
					initResult = DEVICE_INIT_SUCCESS;
					telemetry_info.status = TELEMETRY_STATUS_CONNECTED;
					telemetry_info.active_since = getUptime();
				}
				else{
					telemetry_info.status = TELEMETRY_STATUS_REJECTED_DEVICE_ID;
					pr_error_str_msg("err: auth- token: ", telemetryConfig->telemetryDeviceId);
				}
			}
			else{
				telemetry_info.status = TELEMETRY_STATUS_SERVER_CONNECTION_FAILED;
				pr_error_str_msg("err: server connect ", telemetryConfig->telemetryServerHost);
			}
		}
		else{
			telemetry_info.status = TELEMETRY_STATUS_INTERNET_CONFIG_FAILED;
			pr_error("Failed to configure network\r\n");
		}
	}
	else{
		telemetry_info.status = TELEMETRY_STATUS_CELL_REGISTRATION_FAILED;
		pr_warning("Failed to init cell connection\r\n");
	}
	return initResult;
}

int cellular_check_connection_status(DeviceConfig *config)
{
    setCellBuffer(config->buffer, config->length);
    int status = isNetConnectionErrorOrClosed() ? DEVICE_STATUS_DISCONNECTED : DEVICE_STATUS_NO_ERROR;
    if (status == DEVICE_STATUS_DISCONNECTED) {
        telemetry_info.status = TELEMETRY_STATUS_CURRENT_CONNECTION_TERMINATED;
        pr_debug("cell disconnected\r\n");

    }
    return status;
}
