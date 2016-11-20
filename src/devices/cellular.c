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

#include "led.h"
#include "api.h"
#include "capabilities.h"
#include "cellular.h"
#include "cell_pwr_btn.h"
#include "constants.h"
#include "cpu.h"
#include "dateTime.h"
#include "gsm.h"
#include "loggerConfig.h"
#include "macros.h"
#include <string.h>
#include "printk.h"
#include "serial_buffer.h"
#include "serial.h"
#include "sara_u280.h"
#include "sim900.h"
#include "taskUtil.h"

#include <stdbool.h>

#define AUTOBAUD_ATTEMPTS	5
#define AUTOBAUD_BACKOFF_MS	200
#define PROBE_READ_TIMEOUT_MS	500
/* Good balance between SIM900 and Ublox sara module */
#define RESET_MODEM_DELAY_MS	1100
#define TELEM_AUTH_JSMN_TOKENS	5
#define TELEM_AUTH_RX_TRIES	3
#define TELEM_AUTH_RX_WAIT_MS	5000

static const struct cell_modem_methods* methods;
static const struct at_config *at_cfg;

static struct cellular_info cell_info;
static struct telemetry_info telemetry_info;

static const struct at_config* get_safe_at_config()
{
        /* Safe but un-optimal values for all modems */
        static const struct at_config cfg = {
                .urc_delay_ms = 250,
        };

        return &cfg;
}

size_t cellular_exec_cmd(struct serial_buffer *sb,
                         const size_t wait,
                         const char **rx_msgs,
                         const size_t rx_msgs_size)
{
        serial_buffer_append(sb, "\r\n");
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
        if (NULL == at_cfg)
                at_cfg = get_safe_at_config();

        delayMs(at_cfg->urc_delay_ms);

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
        return msgs && count && 0 == strncmp(ans, msgs[count - 1],
                                             strlen(ans));
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

enum cellular_net_status cellmodem_get_status( void )
{
        return cell_info.net_status;
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

static bool get_methods(const enum cellular_modem modem,
                        const struct cell_modem_methods **methods)
{
        const char *modem_name = NULL;
        *methods = NULL;

        switch(modem) {
        case CELLULAR_MODEM_SIM900:
                modem_name = "SIM900";
                *methods = get_sim900_methods();
                break;
        case CELLULAR_MODEM_UBLOX_SARA_U280:
                modem_name = "UBlox Sara U280";
                *methods = get_sara_u280_methods();
                break;
        default:
                break;
        }

        if (!*methods) {
                pr_warning_int_msg("[cell] Unknown modem code: ", modem);
                return false;
        }

        at_cfg = (*methods)->get_at_config();
        pr_info_str_msg("[cell] Loading driver for ", modem_name);
        return true;
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
                if (gsm_ping_modem(sb)) {
                        pr_info("[cell] auto-baud successful\r\n");
                        return true;
                }

                pr_debug("[cell] Unexpected reply. Backing off...\r\n");
                delayMs(backoff_ms);
        }

        return false;
}

enum cellular_modem probe_cellular_manuf(struct serial_buffer *sb)
{
        const char *msgs[2];
        const size_t msgs_len = ARRAY_LEN(msgs);

        serial_buffer_reset(sb);
        serial_buffer_append(sb, "AT+CGMI");
        const size_t count = cellular_exec_cmd(sb, PROBE_READ_TIMEOUT_MS,
                                               msgs, msgs_len);

        if (!is_rsp_ok(msgs, count))
                return CELLULAR_MODEM_UNKNOWN;

        pr_info_str_msg("[cell] manufacturer is ", msgs[0]);

        if (strstr(msgs[0], "u-blox"))
                return CELLULAR_MODEM_UBLOX_SARA_U280;
        else if (strstr(msgs[0], "SIMCOM_Ltd"))
                return CELLULAR_MODEM_SIM900;
        else
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

const char* cellular_get_net_status_desc()
{
        switch(cell_info.net_status) {
        case CELLULAR_NETWORK_NOT_SEARCHING:
                return "Not Searching";
        case CELLULAR_NETWORK_SEARCHING:
                return "Searching";
        case CELLULAR_NETWORK_DENIED:
                return "Denied.  Call provider.";
        case CELLULAR_NETWORK_REGISTERED:
                return "Registered";
        default:
                return "Unknown";
        }
}

int cellular_disconnect(DeviceConfig *config)
{
        /* Sane because DeviceConfig is typedef from struct serial_buffer* */
        struct serial_buffer *sb = (struct serial_buffer*) config;

        telemetry_info.status = TELEMETRY_STATUS_IDLE;
        telemetry_info.active_since = 0;

        /* Only disconnect if the socket is >= 0 */
        if (telemetry_info.socket < 0) {
                pr_info("[cell] Already disconnected\r\n");
                return 0;
        }

        return methods ? methods->close_telem_connection(
                sb, &cell_info, &telemetry_info) : 0;
}

static void reset_modem()
{
        pr_info("[cell] Resetting modem...\r\n");

        /*
         * SIM900 requres at least 1 second of pull for powerdown
         * and then 1 second of non-pull for powerup.  This however
         * can screw with Ublox-Sara, which needs to autobaud very
         * shortly (< 3sec) after powerup.  This balance seems to work
         * "Well enough"(tm).
         */
        cell_pwr_btn(true);
        delayMs(RESET_MODEM_DELAY_MS);
        cell_pwr_btn(false);
        delayMs(RESET_MODEM_DELAY_MS);

        at_cfg = get_safe_at_config();
}

/**
 * Puts the device in a state that is ready to use.
 */
static int cellular_init_modem(struct serial_buffer *sb)
{
        reset_modem();

        if (!autobaud_modem(sb, AUTOBAUD_ATTEMPTS, AUTOBAUD_BACKOFF_MS))
                return DEVICE_INIT_FAIL;

        if(!gsm_set_echo(sb, false))
                return DEVICE_INIT_FAIL;

        return DEVICE_INIT_SUCCESS;
}

static void print_registration_failure()
{
        pr_info("[cell] Network registration failed: ");
        switch(cell_info.net_status) {
        case CELLULAR_NETWORK_SEARCHING:
                pr_info("Network not found");
                break;
        case CELLULAR_NETWORK_DENIED:
                pr_info("Access denied.  Call provider.");
                break;
        default:
                pr_info("Unknown reason. ");
                pr_info_int_msg("Message code ", cell_info.net_status);
                return;
        }
        pr_info("\r\n");
}

static bool auth_telem_stream(struct serial_buffer *sb,
                              const TelemetryConfig *tc)
{
        serial_buffer_reset(sb);

        const char *deviceId = tc->telemetryDeviceId;

        struct Serial *serial = sb->serial;
        json_objStart(serial);
        json_objStartString(serial, "auth");
        json_string(serial, "deviceId", deviceId, 1);
        json_int(serial, "apiVer", API_REV, 1);
        json_string(serial, "device", DEVICE_NAME, 1);
        json_string(serial, "ver", version_full(), 1);
        json_string(serial, "sn", cpu_get_serialnumber(), 0);
        json_objEnd(serial, 0);
        json_objEnd(serial, 0);
        serial_write_c(serial, '\n');

        pr_debug_str_msg("sending auth- deviceId: ", deviceId);

        /* Parse the incoming JSON */
        jsmn_parser parser;
        jsmntok_t toks[TELEM_AUTH_JSMN_TOKENS];

        for (size_t tries = TELEM_AUTH_RX_TRIES; tries; --tries) {
                if (!serial_buffer_rx(sb, TELEM_AUTH_RX_WAIT_MS))
                        continue;

                jsmn_init(&parser);
                jsmnerr_t status = jsmn_parse(&parser, sb->buffer,
                                              toks, TELEM_AUTH_JSMN_TOKENS);

                if (JSMN_SUCCESS != status) {
                        pr_warning_int_msg("[cell] Auth JSON parsing error: ",
                                           status);
                        continue;
                }

                const jsmntok_t *status_val = jsmn_find_get_node_value_string(
                        toks, "status");
                if (status_val) {
                        /* Could be success or failure.  Handle both */
                        const char *status = status_val->data;
                        if (0 == strcmp(status, "ok")) {
                                pr_info("[cell] Server authenticated\r\n");
                                return true;
                        }

                        /* If here, then something went wrong. */
                        pr_warning_str_msg("[cell] Auth status: ", status);
                        const jsmntok_t *message_val = jsmn_find_get_node_value_string(
                                toks, "message");
                        if (message_val)
                                pr_warning_str_msg("[cell] Auth message: ",
                                                   message_val->data);

                        return false;
                }

                pr_debug_str_msg("[cell] Unexpected auth response: ",
                                 sb->buffer);
        }

        pr_info("[cell] Unable to authenticate telemetry server.\r\n");
        return false;
}

int cellular_init_connection(DeviceConfig *config)
{
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

        /* If here, there is a modem attached.  Figure out what modem it is */
        enum cellular_modem modem_type = probe_cellular_manuf(sb);
        if (!get_methods(modem_type, &methods)) {
                pr_error_int_msg("Unable to load methods for modem_type ",
                                 modem_type);
                return DEVICE_INIT_FAIL;
        }

        if (!methods->init_modem(sb, &cell_info)) {
                telemetry_info.status = TELEMETRY_STATUS_MODEM_INIT_FAILED;
                return DEVICE_INIT_FAIL;
        }

        /* Read SIM data.  Don't care if these fail */
        methods->get_sim_info(sb, &cell_info);

        /* Now register on the network */
        if (!methods->register_on_network(sb, &cell_info)) {
                telemetry_info.status =
                        TELEMETRY_STATUS_CELL_REGISTRATION_FAILED;
                return DEVICE_INIT_FAIL;
        }

        /* Read in Network data.  Don't care if this fails */
        pr_info("[cell] Network registered\r\n");
        methods->get_network_info(sb, &cell_info);

        /* Now setup a GPRS + PDP connection */
        if (!methods->setup_pdp(sb, &cell_info, cellCfg)) {
                telemetry_info.status =
                        TELEMETRY_STATUS_INTERNET_CONFIG_FAILED;
                return DEVICE_INIT_FAIL;
        }

        /* Connect to RCL */
        if(!methods->open_telem_connection(sb, &cell_info, &telemetry_info,
                                           telemetryConfig)) {
                telemetry_info.status =
                        TELEMETRY_STATUS_SERVER_CONNECTION_FAILED;
                print_registration_failure();
                return DEVICE_INIT_FAIL;
        }

        /* Auth against RCL */
        if (!auth_telem_stream(sb, telemetryConfig)){
                telemetry_info.status = TELEMETRY_STATUS_REJECTED_DEVICE_ID;
                return DEVICE_INIT_FAIL;
        }


        telemetry_info.status = TELEMETRY_STATUS_CONNECTED;
        telemetry_info.active_since = getUptime();

	return DEVICE_INIT_SUCCESS;
}

int cellular_check_connection_status(DeviceConfig *config)
{
	if (0 != strncmp(config->buffer, "DISCONNECT", 10))
                return 0;

        pr_info("[cell] socket disconnect\r\n");
        return 1;
}
