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
#include "serial_buffer.h"
#include "sim900.h"
#include "taskUtil.h"

#define TELEMETRY_SERVER_PORT "8080"

static struct {
        telemetry_status_t status;
        tiny_millis_t active_since;
} telemetry_info;

static struct cellular_info cell_info;

enum cellular_modem {
        CELLULAR_MODEM_UNKNOWN = 0,
        CELLULAR_MODEM_SIM900,
};

const char* readsCell(struct serial_buffer *sb, size_t timeout)
{
    serial_buffer_read_wait(sb, timeout);
    return sb->buffer;
}

void putsCell(struct serial_buffer *sb, const char *data)
{
	LED_toggle(0);
	sb->serial->put_s(data);
	pr_debug_str_msg("cellWrite: ", data);
}

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

enum cellular_modem probe_cellular_modem(struct serial_buffer *sb)
{
        /* XXX STIEG: STUB!  Finish me */
        return CELLULAR_MODEM_UNKNOWN;
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

static int writeAuthJSON(struct serial_buffer *sb, const char *deviceId)
{
        /*
         * Send linefeed at slow intervals until we have the auth packet
         * ack from server.
         * XXX STIEG: WTF!!!  What the hell is the point of this?
         */
        for (int i = 0; i < 5; i++) {
                serial_buffer_puts(sb, " ");
                delayMs(250);
        }

        Serial *serial = sb->serial;
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
                const char * data = readsCell(sb, 1000);
                if (0 == strncmp(data, ok_resp, sizeof(ok_resp) - 1))
                        return 0;
        }

        return -1;
}

int cellular_disconnect(DeviceConfig *config)
{
        telemetry_info.status = TELEMETRY_STATUS_IDLE;
        pr_info("[cell] disconnected\r\n");
        return closeNet((struct serial_buffer*) config);
}

int cellular_init_connection(DeviceConfig *config)
{
	telemetry_info.active_since = 0;

        LoggerConfig *loggerConfig = getWorkingLoggerConfig();
        CellularConfig *cellCfg =
                &(loggerConfig->ConnectivityConfigs.cellularConfig);
        TelemetryConfig *telemetryConfig =
                &(loggerConfig->ConnectivityConfigs.telemetryConfig);

        struct serial_buffer sb;
        serial_buffer_create(&sb, config->serial, config->length,
                             config->buffer);

        /* Figure out what Modem we are using */
        probe_cellular_modem(&sb);

	pr_debug("init cell connection\r\n");

        if (0 != initCellModem(&sb, cellCfg, &cell_info)) {
                telemetry_info.status = TELEMETRY_STATUS_CELL_REGISTRATION_FAILED;
		pr_warning("Failed to init cell connection\r\n");
                return DEVICE_INIT_FAIL;
        }

        pr_info("[cell] modem initialized\r\n");

        if (0 != configureNet(&sb)) {
                telemetry_info.status = TELEMETRY_STATUS_INTERNET_CONFIG_FAILED;
                pr_warning("[cell] Failed to configure network\r\n");
                return DEVICE_INIT_FAIL;
        }

        pr_info("[cell] network configured\r\n");

        if(0 != connectNet(&sb, telemetryConfig->telemetryServerHost, TELEMETRY_SERVER_PORT, 0)){
                telemetry_info.status = TELEMETRY_STATUS_SERVER_CONNECTION_FAILED;
                pr_error_str_msg("err: server connect ", telemetryConfig->telemetryServerHost);
                return DEVICE_INIT_FAIL;
        }

        pr_info("[cell] server connected\r\n");

        if (0 != writeAuthJSON(&sb, telemetryConfig->telemetryDeviceId)){
                telemetry_info.status = TELEMETRY_STATUS_REJECTED_DEVICE_ID;
                pr_error_str_msg("err: auth- token: ", telemetryConfig->telemetryDeviceId);
                return DEVICE_INIT_FAIL;

        }

        pr_info("[cell] server authenticated\r\n");
        telemetry_info.status = TELEMETRY_STATUS_CONNECTED;
        telemetry_info.active_since = getUptime();

	return DEVICE_INIT_SUCCESS;
}

int cellular_check_connection_status(DeviceConfig *config)
{
        const int status = isNetConnectionErrorOrClosed((struct serial_buffer*) config) ?
                DEVICE_STATUS_DISCONNECTED : DEVICE_STATUS_NO_ERROR;

        if (status == DEVICE_STATUS_DISCONNECTED) {
                telemetry_info.status = TELEMETRY_STATUS_CURRENT_CONNECTION_TERMINATED;
                pr_debug("cell disconnected\r\n");

        }

        return status;
}
