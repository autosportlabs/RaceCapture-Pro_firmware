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
#include "array_utils.h"
#include "capabilities.h"
#include "cellular.h"
#include "constants.h"
#include "cpu.h"
#include "dateTime.h"
#include "loggerConfig.h"
#include "mod_string.h"
#include "printk.h"
#include "serial_buffer.h"
#include "serial.h"
#include "sara_u280.h"
#include "taskUtil.h"

#include <stdbool.h>

/* The pause time (in ms) for Unexpected Response Codes */
#define CELLULAR_URC_PAUSE_MS 25
#define TELEMETRY_SERVER_PORT "8080"

static struct {
        telemetry_status_t status;
        tiny_millis_t active_since;
        int socket;
} telemetry_info;

static struct cellular_info cell_info;

enum cellular_modem {
        CELLULAR_MODEM_UNKNOWN = 0,
        CELLULAR_MODEM_SIM900,
        CELLULAR_MODEM_UBLOX_SARA,
};

static const enum log_level serial_dbg_lvl = INFO;

#if 0
static bool cell_rx_is_useful_msg(const char *msg)
{
        if (NULL == msg)
                return false;

        /*
         * If the first character of the message is \r or \n
         * then its not going to be a useful message to us.
         */
        switch(*msg) {
        case('\r'):
        case('\n'):
                return false;
        default:
                return true;
        }
}
#endif

static void cell_serial_tx_cb(const char *data)
{
        static bool pr_tx_pfx = true;

        for(; *data; ++data) {
                if (pr_tx_pfx) {
                        printk(serial_dbg_lvl, "[cell] tx: ");
                        LED_toggle(0);
                        pr_tx_pfx = false;
                }

                switch(*data) {
                case('\r'):
                case('\n'):
                        printk_crlf(serial_dbg_lvl);
                        pr_tx_pfx = true;
                        break;
                default:
                        printk_char(serial_dbg_lvl, *data);
                        break;
                }
        }
}

static void cell_serial_rx_cb(const char *data)
{
        static bool pr_rx_pfx = true;

        if (NULL == data)
                return;

        printk_str_msg(serial_dbg_lvl, "[cell] rx: ", data);
        return;

        for(; *data; ++data) {
                if (pr_rx_pfx) {
                        printk(serial_dbg_lvl, "[cell] rx: ");
                        pr_rx_pfx = false;
                }

                switch(*data) {
                case('\r'):
                        printk(serial_dbg_lvl, "<cr>\r");
                        break;
                case('\n'):
                        printk(serial_dbg_lvl, "<lf>\n");
                        pr_rx_pfx = true;
                        break;
                default:
                        printk_char(serial_dbg_lvl, *data);
                        break;
                }
        }
}

size_t cellular_exec_cmd(struct serial_buffer *sb,
                         const size_t wait,
                         const char **rx_msgs,
                         const size_t rx_msgs_size)
{
        serial_buffer_append(sb, "\r");
        serial_buffer_tx(sb);
        serial_buffer_clear(sb);
        const size_t rv = serial_buffer_rx_msgs(sb, wait, rx_msgs,
                                                rx_msgs_size);

        /*
         * URC Pause (must be > 20ms)
         * TODO: Add support for handling URCs.  Right now we ignore them.
         *       This will likely require a re-write of how cellular is
         *       done.
         */
        delayMs(CELLULAR_URC_PAUSE_MS);

        return rv;
}

/**
 * Checks that a response is the provided value.  Assumes the status message
 * is the lats message in the reply chain.
 * @param msgs The messages you got back.
 * @param count How many messages you got back.
 * @param ans The expected answer.
 */
bool is_rsp(const char **msgs, const size_t count, const char *ans)
{
        /* Check to ensure last msg we got was an OK */
        return msgs && count && 0 == strcmp(ans, msgs[count - 1]);
}

/**
 * Checks that a response got the OK.  Assumes the status message
 * is the lats message in the reply chain.
 * @param msgs The messages you got back.
 * @param count How many messages you got back.
 */
bool is_rsp_ok(const char **msgs, const size_t count)
{
        return is_rsp(msgs, count, "OK");
}


int cellular_exec_match(struct serial_buffer *sb, const size_t wait,
                        const char **rx_msgs, const size_t rx_msgs_size,
                        const char *answrs[], const size_t answrs_size,
                        const size_t rx_msg_idx)
{
        const size_t count = cellular_exec_cmd(sb, wait, rx_msgs,
                                               rx_msgs_size);

        if (!is_rsp_ok(rx_msgs, count)) {
                pr_warning_str_msg("[cell] Command failed. Response: ",
                                   rx_msgs[count - 1]);
                return -1;
        }

        for (int i = 0; i < answrs_size; ++i) {
                const size_t len = strlen(answrs[i]);
                if (!strncmp(answrs[i], rx_msgs[rx_msg_idx], len))
                        return i;
        }

        /* If here, no match.  Return -2 */
        pr_debug("[cell] No answer matched\r\n");
        return -2;
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

bool ping_modem(struct serial_buffer *sb)
{
        const char *msgs[2];
        const size_t msgs_len = ARRAY_LEN(msgs);

        serial_buffer_reset(sb);
        serial_buffer_append(sb, "AT");
        const size_t count =cellular_exec_cmd(sb, READ_TIMEOUT, msgs,
                                              msgs_len);
        return is_rsp_ok(msgs, count);
}

/**
 * Does auto-bauding.  This allows the chip to figure out what serial
 * settings to use (default 8N1) along with what baud rate to use.  This
 * should complete within 3 seconds according to spec.  Also ensures
 * that the modem is still there.
 *
 * NOTE: We don't use conveinence methods here as echo may be on still.
 */
bool autobaud_modem(struct serial_buffer *sb, size_t tries,
                    const size_t backoff_ms)
{
        for(; tries > 0; --tries) {
                if (ping_modem(sb)) {
                        pr_info("[cell] auto-baud successful\r\n");
                        return true;
                }

                pr_info("[cell] Unexpected reply. Backing off...\r\n");
                delayMs(backoff_ms);
        }

        pr_warning("[cell] Failed to auto-baud device\r\n");
        return false;
}

/**
 * Turns on/off echo from the modem.
 */
static bool set_echo(struct serial_buffer *sb, bool on)
{
        const char *cmd = on ? "ATE1" : "ATE0";
        const char *msgs[2];
        const size_t msgs_len = ARRAY_LEN(msgs);

        serial_buffer_reset(sb);
        serial_buffer_append(sb, cmd);
        const size_t count = cellular_exec_cmd(sb, READ_TIMEOUT, msgs, msgs_len);
        const bool status = is_rsp_ok(msgs, count);

        if (status) {
                pr_info_str_msg("[cell] Echo status: ", on ? "on" : "off");
                return true;
        }

        pr_warning("[cell] Failed to alter echoing.\r\n");
        return false;
}


enum cellular_modem probe_cellular_manuf(struct serial_buffer *sb)
{
        const char *msgs[2];
        const size_t msgs_len = ARRAY_LEN(msgs);

        serial_buffer_reset(sb);
        serial_buffer_append(sb, "AT+CGMI");
        const size_t count = cellular_exec_cmd(sb, READ_TIMEOUT, msgs, msgs_len);

        if (!is_rsp_ok(msgs, count))
                return CELLULAR_MODEM_UNKNOWN;

        pr_info_str_msg("[cell] manufacturer is ", msgs[0]);

        /* TODO: Finish me.  Only support UBLOX_SARA now.*/
        return CELLULAR_MODEM_UBLOX_SARA;
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

int cellular_disconnect(DeviceConfig *config)
{
        /* Sane because DeviceConfig is typedef from struct serial_buffer* */
        struct serial_buffer *sb = (struct serial_buffer*) config;

        if (!sara_u280_stop_direct_mode(sb)) {
                /* Then we don't know if can issue commands */
                pr_warning("[cell] Failed to escape Direct Mode\r\n");
                return -1;
        }

        const int sid = telemetry_info.socket;
        if (!sara_u280_close_tcp_socket(sb, sid)) {
                pr_warning_int_msg("[cell] Failed to close socket ", sid);
                return -2;
        }

        telemetry_info.socket = -1;
        telemetry_info.status = TELEMETRY_STATUS_IDLE;
        return 0;
}

/**
 * Puts the device in a state that is ready to use.
 */
static int cellular_init_modem(struct serial_buffer *sb)
{
        /* First pause for 3 seconds to ensure device is ready */
        pr_info("[cell] Waiting 3 seconds for device to be ready\r\n");
        delayMs(3000);

        if (!autobaud_modem(sb, 30, 1000))
                return DEVICE_INIT_FAIL;

        if(!set_echo(sb, false))
                return DEVICE_INIT_FAIL;

        sara_u280_get_subscriber_number(sb, &cell_info);
        sara_u280_get_imei(sb, &cell_info);
        sara_u280_get_signal_strength(sb, &cell_info);

        return DEVICE_INIT_SUCCESS;
}

static bool register_on_network(struct serial_buffer *sb)
{
        /* Check our status on the network */
        enum cellular_net_status status;
        for (size_t tries = 30; tries; --tries) {
                 status = get_net_reg_status(sb, &cell_info);
                if (CELLULAR_NETWORK_REGISTERED == status)
                        break;

                /* Wait before trying again */
                delayMs(3000);
        }

        if (status != CELLULAR_NETWORK_REGISTERED) {
                /* If here, we failed :( */
                pr_info("[cell] Network registration failed: ");
                switch(status) {
                case CELLULAR_NETWORK_SEARCHING:
                        pr_info("Network not found");
                        break;
                case CELLULAR_NETWORK_DENIED:
                        pr_info("Access denied.  Call provider.");
                        break;
                default:
                        break;
                }
                pr_info("\r\n");
                cell_info.status = CELLMODEM_STATUS_NO_NETWORK;
                return false;
        }

        pr_info("[cell] Network registered\r\n");
        get_network_reg_info(sb, &cell_info);
        cell_info.status = CELLMODEM_STATUS_PROVISIONED;

        return cell_info.net_status;
}

static bool setup_gprs(struct serial_buffer *sb,
                       CellularConfig *cc)
{
        /* Check GPRS attached */
        bool gprs_attached;
        for (size_t tries = 10; tries; --tries) {
                gprs_attached = sara_u280_is_gprs_attached(sb);

                if (gprs_attached)
                        break;

                /* Wait before trying again.  Arbitrary backoff */
                delayMs(3000);
        }
        pr_info_str_msg("[cell] GPRS Attached: ", gprs_attached ? "yes" : "no");
        if (!gprs_attached)
                return false;

        /* Setup APN */
        const bool status =
                sara_u280_put_apn_config(sb, cc->apnHost,cc->apnUser,
                                         cc->apnPass) &&
                sara_u280_set_dynamic_ip(sb) &&
                sara_u280_put_dns_config(sb, "8.8.8.8", "8.8.4.4");

        if (!status) {
                pr_warning("[cell] APN/IP/DNS config failed\r\n");
                return false;
        }

        bool gprs_active;
        for (size_t tries = 10; tries; --tries) {
                gprs_active = sara_u280_activate_gprs(sb);

                if (gprs_active)
                        break;

                /* Wait before trying again.  Arbitrary backoff */
                delayMs(3000);
        }
        if (!gprs_active) {
                pr_warning("[cell] Failed connect GPRS\r\n");
                return false;
        }

        pr_debug("[cell] GPRS connected\r\n");
        sara_u280_get_ip_address(sb);
        return true;
}

static bool connect_rcl_telem(struct serial_buffer *sb,
                              const TelemetryConfig *tc)
{
        const int socket_id = sara_u280_create_tcp_socket(sb);
        if (socket_id < 0) {
                pr_warning("[cell] Failed to create socket\r\n");
                telemetry_info.socket = -1;
                return false;
        }
        telemetry_info.socket = socket_id;

        const char *host = tc->telemetryServerHost;
        const int port = 8080;
        if (!sara_u280_connect_tcp_socket(sb, socket_id, host, port)) {
                pr_warning_str_msg("[cell] Failed to connect to ", host);
                return false;
        }

        return sara_u280_start_direct_mode(sb, socket_id);
}

static bool auth_telem_stream(struct serial_buffer *sb,
                              const TelemetryConfig *tc)
{
        serial_buffer_reset(sb);

        const char *deviceId = tc->telemetryDeviceId;

        /* TODO Replace with creation of buffer in memory */
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
        /* XXX: Replace with real JSON parser */
        const char* ok_resp = "{\"status\":\"ok\"}";
        while (attempts-- > 0)
                if (0 == serial_buffer_rx(sb, 1000))
                        continue;

        return strncmp(sb->buffer, ok_resp, strlen(ok_resp));
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

        const int init_status = cellular_init_modem(sb);
        if (DEVICE_INIT_SUCCESS != init_status)
                return DEVICE_INIT_FAIL;

        /* Figure out what Modem we are using */
        probe_cellular_manuf(sb);
	pr_debug("init cell connection\r\n");

        /* First register on the network */
        if (CELLULAR_NETWORK_REGISTERED != register_on_network(sb)) {
                telemetry_info.status = TELEMETRY_STATUS_CELL_REGISTRATION_FAILED;
                return DEVICE_INIT_FAIL;
        }

        /* Now setup a GPRS connection */
        if (!setup_gprs(sb, cellCfg)) {
                telemetry_info.status = TELEMETRY_STATUS_INTERNET_CONFIG_FAILED;
                return DEVICE_INIT_FAIL;
        }

        /* Connect to RCL */
        if(!connect_rcl_telem(sb, telemetryConfig)) {
                telemetry_info.status = TELEMETRY_STATUS_SERVER_CONNECTION_FAILED;
                return DEVICE_INIT_FAIL;
        }

        /* Auth against RCL */
        if (0 != auth_telem_stream(sb, telemetryConfig)){
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
	if (strncmp(config->buffer,"DISCONNECT", 10) == 0){
                pr_info("[cell] socket disconnect\r\n");
                return 1;
	}
	return 0;
}
