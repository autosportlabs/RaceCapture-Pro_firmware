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

#include <stdbool.h>

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

static void cell_serial_tx_cb(const char *data)
{
        static bool pr_pfx = true;
        const enum log_level lvl = INFO;

        for(; *data; ++data) {
                if (pr_pfx) {
                        printk(lvl, "[cell] tx: ");
                        LED_toggle(0);
                        pr_pfx = false;
                }

                switch(*data) {
                case('\r'):
                case('\n'):
                        printk_crlf(lvl);
                        pr_pfx = true;
                        break;
                default:
                        printk_char(lvl, *data);
                        break;
                }
        }
}

static void cell_serial_rx_cb(const char *data)
{
        static bool pr_pfx = true;
        const enum log_level lvl = INFO;

        for(; *data; ++data) {
                if (pr_pfx) {
                        printk(lvl, "[cell] rx: ");
                        LED_toggle(0);
                        pr_pfx = false;
                }

                switch(*data) {
                case('\r'):
                case('\n'):
                        printk_crlf(lvl);
                        pr_pfx = true;
                        break;
                default:
                        printk_char(lvl, *data);
                        break;
                }
        }
}

/* XXX STIEG: Review sanity of this */
static void cellular_rstrip(char *data)
{
        for(; *data >= 32; ++data);
        *data = 0;
}

int cellular_wait_cmd_rsp(struct serial_buffer *sb, const char *expectedRsp,
                               size_t wait)
{
        int res = NO_CELL_RESPONSE;
        serial_buffer_read_wait(sb, wait);
        int len = serial_buffer_read_wait(sb, READ_TIMEOUT);
        delayMs(PAUSE_DELAY); //this is a magic delay that sim900 needs for proper communications

        if (len) {
                cellular_rstrip(sb->buffer);
                if (strlen(sb->buffer) > 0) {
                        res = (strstr(expectedRsp, sb->buffer) != NULL);
                }
        }

        return res;
}

int cellular_send_cmd_wait(struct serial_buffer *sb, const char *cmd,
                           const char *expectedRsp, size_t wait)
{
        serial_buffer_flush(sb);
        serial_buffer_puts(sb, cmd);
        return cellular_wait_cmd_rsp(sb, expectedRsp, wait);
}

int cellular_send_cmd(struct serial_buffer *sb, const char * cmd,
                      const char *expectedRsp)
{
        return cellular_send_cmd_wait(sb, cmd, expectedRsp, READ_TIMEOUT);
}

int cellular_send_cmd_ok(struct serial_buffer *sb, const char * cmd)
{
        return cellular_send_cmd(sb, cmd, "OK");
}

int cellular_send_cmd_retry(struct serial_buffer *sb, const char * cmd,
                            const char * expectedRsp, size_t maxAttempts,
                            size_t maxNoResponseAttempts)
{
        int result = 0;
        size_t attempts = 0;

        while (attempts++ < maxAttempts) {
                result = cellular_send_cmd(sb, cmd, expectedRsp);

                if (result == 1)
                        break;

                if (result == NO_CELL_RESPONSE && attempts > maxNoResponseAttempts)
                        break;

                delayMs(1000);
        }

        return result;
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
        /* XXX STIEG: HACK... setting callback here */
        config->serial->tx_callback = cell_serial_tx_cb;
        config->serial->rx_callback = cell_serial_rx_cb;

	telemetry_info.active_since = 0;

        LoggerConfig *loggerConfig = getWorkingLoggerConfig();
        CellularConfig *cellCfg =
                &(loggerConfig->ConnectivityConfigs.cellularConfig);
        TelemetryConfig *telemetryConfig =
                &(loggerConfig->ConnectivityConfigs.telemetryConfig);

        /* This is sane since DeviceConfig is typedef'd as a serial_buffer */
        struct serial_buffer *sb = (struct serial_buffer*) config;

        /* Figure out what Modem we are using */
        probe_cellular_modem(sb);

	pr_debug("init cell connection\r\n");

        if (0 != initCellModem(sb, cellCfg, &cell_info)) {
                telemetry_info.status = TELEMETRY_STATUS_CELL_REGISTRATION_FAILED;
		pr_warning("Failed to init cell connection\r\n");
                return DEVICE_INIT_FAIL;
        }

        pr_info("[cell] modem initialized\r\n");

        if (0 != configureNet(sb)) {
                telemetry_info.status = TELEMETRY_STATUS_INTERNET_CONFIG_FAILED;
                pr_warning("[cell] Failed to configure network\r\n");
                return DEVICE_INIT_FAIL;
        }

        pr_info("[cell] network configured\r\n");

        if(0 != connectNet(sb, telemetryConfig->telemetryServerHost, TELEMETRY_SERVER_PORT, 0)){
                telemetry_info.status = TELEMETRY_STATUS_SERVER_CONNECTION_FAILED;
                pr_error_str_msg("err: server connect ", telemetryConfig->telemetryServerHost);
                return DEVICE_INIT_FAIL;
        }

        pr_info("[cell] server connected\r\n");

        if (0 != writeAuthJSON(sb, telemetryConfig->telemetryDeviceId)){
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
