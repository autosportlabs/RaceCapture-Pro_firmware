/*
 * Race Capture Pro Firmware
 *
 * Copyright (C) 2019 Autosport Labs
 *
 * This file is part of the Race Capture Pro fimrware suite
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

#include "cellular.h"
#include "cell_pwr_btn.h"
#include "gsm.h"
#include "macros.h"
#include "printk.h"
#include "serial_buffer.h"
#include "sara_r4.h"
#include "taskUtil.h"

#include <string.h>
#include <stdlib.h>

#define READ_TIMEOUT 	500
#define MEDIUM_TIMEOUT 	15000
#define CONNECT_TIMEOUT 60000

#define GPRS_ACTIVATE_PDP_ATTEMPTS	3
#define GPRS_ACTIVATE_PDP_BACKOFF_MS	3000
#define GPRS_ATTACH_ATTEMPTS	5
#define GPRS_ATTACH_BACKOFF_MS	1000
#define NET_REG_ATTEMPTS	100
#define SARA_R4_ERROR_THRESHOLD 6
#define NET_REG_BACKOFF_MS	1000
#define STOP_DM_RX_EVENTS	10
#define STOP_DM_RX_TIMEOUT_MS	1000
#define SARA_R4_SUBSCRIBER_NUMBER_RETRIES 3

static bool sara_r4_get_subscriber_number(struct serial_buffer *sb,
                struct cellular_info *ci)
{
        /* sara R4 often does not respond immediately to SIM number
         * requests; do some retrying
         */
        bool status = false;
        for (size_t i = 0; i < SARA_R4_SUBSCRIBER_NUMBER_RETRIES && !status; ++i)
                status = gsm_get_subscriber_number(sb, ci);

        if (!status)
                pr_warning("[cell] Phone number not available\r\n");
        return status;
}

static bool sara_r4_get_signal_strength(struct serial_buffer *sb,
                struct cellular_info *ci)
{
        return gsm_get_signal_strength(sb, ci);
}

static bool sara_r4_get_imei(struct serial_buffer *sb,
                               struct cellular_info *ci)
{
        return gsm_get_imei(sb, ci);
}

static enum cellular_net_status sara_r4_get_network_reg_status(
        struct serial_buffer *sb, struct cellular_info *ci)
{
        const char *cmd = "AT+CEREG?";
        const char *msgs[2];
        const size_t msgs_len = ARRAY_LEN(msgs);
        const char *answrs[] = {"+CEREG: 0,0",
                                "+CEREG: 0,1",
                                "+CEREG: 0,2",
                                "+CEREG: 0,3",
                                "+CEREG: 0,4",
                                "+CEREG: 0,5"
                               };
        const size_t answrs_len = ARRAY_LEN(answrs);

        serial_buffer_reset(sb);
        serial_buffer_append(sb, cmd);
        const int idx = cellular_exec_match(sb, READ_TIMEOUT, msgs, msgs_len,
                                            answrs, answrs_len, 0);

        switch(idx) {
        case 0:
                ci->net_status = CELLULAR_NETWORK_NOT_SEARCHING;
                break;
        case 1:
        case 5:
                ci->net_status = CELLULAR_NETWORK_REGISTERED;
                break;
        case 2:
        case 4:
                ci->net_status = CELLULAR_NETWORK_SEARCHING;
                break;
        case 3:
                ci->net_status = CELLULAR_NETWORK_DENIED;
                break;
        default:
                ci->net_status = CELLULAR_NETWORK_STATUS_UNKNOWN;
                break;
        }

        return ci->net_status;
}

static enum cellular_net_status sara_r4_get_net_reg_status(
        struct serial_buffer *sb,struct cellular_info *ci)
{
        return sara_r4_get_network_reg_status(sb, ci);
}

static bool sara_r4_get_network_reg_info(struct serial_buffer *sb,
                struct cellular_info *ci)
{
        return gsm_get_network_reg_info(sb, ci);
}

static bool sara_r4_is_gprs_attached(struct serial_buffer *sb)
{
        return gsm_is_gprs_attached(sb);
}

static bool sara_r4_set_apn_config(struct serial_buffer *sb,
                                     const int pdp_id,
                                     const char *host,
                                     const char* user,
                                     const char* password)
{
        const char *msgs[1];
        const size_t msgs_len = ARRAY_LEN(msgs);

        serial_buffer_reset(sb);

        /* APN settings
         * user / password not supported for the R4
         */
        serial_buffer_printf_append(sb, "AT+CGDCONT=1,\"IP\",\"%s\"", host);

        const size_t count = cellular_exec_cmd(sb, READ_TIMEOUT, msgs,
                                               msgs_len);
        bool is_ok = is_rsp_ok(msgs, count);
        pr_info_bool_msg("[sara_r4] Set APN: ", is_ok);
        return is_ok;
}

static bool sara_r4_put_dns_config(struct serial_buffer *sb,
                                     const char* dns1,
                                     const char *dns2)
{
        /* DNS configuration not currently supported
         * Placeholder for future DNS configuration
         */
        return true;
}

static int sara_r4_create_tcp_socket(struct serial_buffer *sb)
{
        const char *cmd = "AT+USOCR=6";
        const char *msgs[2];
        const size_t msgs_len = ARRAY_LEN(msgs);

        serial_buffer_reset(sb);
        serial_buffer_append(sb, cmd);
        const size_t count = cellular_exec_cmd(sb, READ_TIMEOUT, msgs,
                                               msgs_len);
        const bool status = is_rsp_ok(msgs, count);
        if (!status) {
                pr_warning("[sara_r4] Failed to create a socket\r\n");
                return -1;
        }

        /*
         * Parse out the socket ID.  Needed for Muxing.
         * +USOCR: <socket_id>
         */
        const int socket = msgs[0][8] - '0';
        if (socket < 0 || socket > 6) {
                pr_warning("[sara_r4] Failed to parse socket ID.\r\n");
                return -2;
        }

        pr_debug_int_msg("[sara_r4] Socket ID: ", socket);
        return socket;
}

static bool sara_r4_connect_tcp_socket(struct serial_buffer *sb,
                const int socket_id,
                const char* host,
                const int port)
{
        const char *msgs[1];
        const size_t msgs_len = ARRAY_LEN(msgs);

        serial_buffer_reset(sb);
        serial_buffer_printf_append(sb, "AT+USOCO=%d,\"%s\",%d", socket_id,
                                    host, port);
        const size_t count = cellular_exec_cmd(sb, CONNECT_TIMEOUT, msgs,
                                               msgs_len);
        return is_rsp_ok(msgs, count);
}

static bool sara_r4_close_tcp_socket(struct serial_buffer *sb,
                                       const int socket_id)
{
        const char *msgs[1];
        const size_t msgs_len = ARRAY_LEN(msgs);

        serial_buffer_reset(sb);
        serial_buffer_printf_append(sb, "AT+USOCL=%d,1", socket_id);
        const size_t count = cellular_exec_cmd(sb, MEDIUM_TIMEOUT, msgs,
                                               msgs_len);
        return is_rsp_ok(msgs, count);
}

static bool sara_r4_start_direct_mode(struct serial_buffer *sb,
                                        const int socket_id)
{
        const char *msgs[1];
        const size_t msgs_len = ARRAY_LEN(msgs);

        serial_buffer_reset(sb);
        serial_buffer_printf_append(sb, "AT+USODL=%d", socket_id);
        const size_t count = cellular_exec_cmd(sb, MEDIUM_TIMEOUT, msgs,
                                               msgs_len);
        return is_rsp(msgs, count, "CONNECT");
}

static bool sara_r4_stop_direct_mode(struct serial_buffer *sb)
{
        /* Must delay min 2000ms before stopping direct mode */
        delayMs(2100);

        /*
         * Using straight serial buffer logic here instead of
         * the AT command system because we can get data intended for
         * the JSON parser while we do this. Also because we don't
         * want the \r at the end of the command.
         */
        serial_buffer_reset(sb);
        serial_buffer_append(sb, "+++");
        serial_buffer_tx(sb);
        for (size_t events = STOP_DM_RX_EVENTS; events; --events) {
                serial_buffer_reset(sb);
                if (serial_buffer_rx(sb, STOP_DM_RX_TIMEOUT_MS) &&
                    is_rsp((const char**) &(sb->buffer), 1, "DISCONNECT"))
                        return true;
        }

        return false;
}

static bool sara_r4_set_profile(struct serial_buffer *sb)
{
        const char *msgs[1];
        const size_t msgs_len = ARRAY_LEN(msgs);

        serial_buffer_reset(sb);
        /* Set the default profile, 1 - pick from SIM card
         */
        serial_buffer_printf_append(sb, "AT+UMNOPROF=%d", 1);
        const size_t count = cellular_exec_cmd(sb, MEDIUM_TIMEOUT, msgs, msgs_len);
        bool is_ok = is_rsp_ok(msgs, count);
        pr_info_bool_msg("[sara_r4] Set Profile: ", is_ok);
        return is_ok;
}

static bool sara_r4_set_error_reporting(struct serial_buffer *sb)
{
        const char *msgs[1];
        const size_t msgs_len = ARRAY_LEN(msgs);

        serial_buffer_reset(sb);
        /* Turn on extended error reporting */
        serial_buffer_printf_append(sb, "AT+CMEE=%d", 2);
        const size_t count = cellular_exec_cmd(sb, MEDIUM_TIMEOUT, msgs, msgs_len);
        bool is_ok = is_rsp_ok(msgs, count);
        pr_info_bool_msg("[sara_r4] Set CMEE: ", is_ok);
        return is_ok;
}


static bool sara_r4_set_baud_rate(struct serial_buffer *sb)
{
        const char *msgs[2];
        const size_t msgs_len = ARRAY_LEN(msgs);

        serial_buffer_reset(sb);
        serial_buffer_append(sb, "AT+IPR=115200");
        const size_t count = cellular_exec_cmd(sb, READ_TIMEOUT, msgs, msgs_len);
        delayMs(100);
        serial_config(sb->serial,8,0,1,115200);
        return is_rsp_ok(msgs, count);
}

static bool sara_r4_init(struct serial_buffer *sb,
                           struct cellular_info *ci,
                           CellularConfig *cc)
{

        pr_info("[sara_r4] Initializing\r\n");
        return (sara_r4_set_baud_rate(sb) &&
                        sara_r4_set_apn_config(sb, 0, cc->apnHost, cc->apnUser, cc->apnPass) &&
                        sara_r4_set_profile(sb) &&
                        sara_r4_set_error_reporting(sb));
}

static bool sara_r4_get_sim_info(struct serial_buffer *sb,
                                   struct cellular_info *ci)
{
        bool status = false;
        status = sara_r4_get_subscriber_number(sb, ci);
        status = sara_r4_get_imei(sb, ci) && status;
        return status;
}

static bool sara_r4_register_on_network(struct serial_buffer *sb,
                struct cellular_info *ci)
{
        size_t errors = 0;
        /* Check our status on the network */
        for (size_t tries = NET_REG_ATTEMPTS; tries && errors < SARA_R4_ERROR_THRESHOLD; --tries) {
                errors += (sara_r4_get_net_reg_status(sb, ci) == CELLULAR_NETWORK_STATUS_UNKNOWN);
                errors += (sara_r4_get_signal_strength(sb, ci) == false);
                switch(ci->net_status) {
                case CELLULAR_NETWORK_DENIED:
                case CELLULAR_NETWORK_REGISTERED:
                        goto out;
                default:
                        break;
                }

                /* Wait before trying again */
                delayMs(NET_REG_BACKOFF_MS);
        }

out:
        return CELLULAR_NETWORK_REGISTERED == ci->net_status;
}

static bool sara_r4_setup_pdp(struct serial_buffer *sb,
                                struct cellular_info *ci,
                                const CellularConfig *cc)
{
        /* Check GPRS attached */
        bool gprs_attached;
        for (size_t tries = GPRS_ATTACH_ATTEMPTS; tries; --tries) {
                gprs_attached = sara_r4_is_gprs_attached(sb);

                if (gprs_attached)
                        break;

                /* Wait before trying again.  Arbitrary backoff */
                delayMs(GPRS_ATTACH_BACKOFF_MS);
        }

        pr_info_str_msg("[sara_r4] GPRS Attached: ",
                        gprs_attached ? "yes" : "no");
        if (!gprs_attached)
                return false;

        if (!sara_r4_put_dns_config(sb, cc->dns1, cc->dns2)){
                pr_info("[sara_r4] Using default DNS\r\n");
        }
        return true;
}

static bool sara_r1_configure_tcp_socket_character_trigger(struct serial_buffer *sb,
                                                int socket_id)
{
        const char *msgs[1];
        const size_t msgs_len = ARRAY_LEN(msgs);

        serial_buffer_reset(sb);
        serial_buffer_printf_append(sb, "AT+UDCONF=7,%d,10", socket_id);
        const size_t count = cellular_exec_cmd(sb, CONNECT_TIMEOUT, msgs,
                                               msgs_len);
        bool is_ok = is_rsp_ok(msgs, count);
        pr_info_bool_msg("[sara_r4] Configure TCP socket character trigger: ", is_ok);
        return is_ok;
}

static bool sara_r1_configure_tcp_socket_nodelay(struct serial_buffer *sb,
                                         int socket_id)
{
        const char *msgs[1];
        const size_t msgs_len = ARRAY_LEN(msgs);

        serial_buffer_reset(sb);
        serial_buffer_printf_append(sb, "AT+USOSO=%d,6,1,1", socket_id);
        const size_t count = cellular_exec_cmd(sb, CONNECT_TIMEOUT, msgs,
                                               msgs_len);
        bool is_ok = is_rsp_ok(msgs, count);
        pr_info_bool_msg("[sara_r4] Configure TCP socket nodelay: ", is_ok);
        return is_ok;
}

static bool sara_r1_configure_tcp_socket(struct serial_buffer *sb,
                                         int socket_id)
{
        return sara_r1_configure_tcp_socket_character_trigger(sb, socket_id) &&
                        sara_r1_configure_tcp_socket_nodelay(sb, socket_id);
}

static bool sara_r4_connect_rcl_telem(struct serial_buffer *sb,
                                        struct cellular_info *ci,
                                        struct telemetry_info *ti,
                                        const TelemetryConfig *tc)
{
        ti->socket = sara_r4_create_tcp_socket(sb);
        if (ti->socket < 0) {
                pr_warning("[sara_r4] Failed to create a socket\r\n");
                return false;
        }

        if (!sara_r4_connect_tcp_socket(sb, ti->socket,
                                          tc->telemetryServerHost,
                                          tc->telemetry_port)) {
                pr_warning("[sara_r4] Failed to connect to ");
                pr_warning(tc->telemetryServerHost);
                pr_warning_int_msg(":", tc->telemetry_port);

                return false;
        }

        if (!sara_r1_configure_tcp_socket(sb, ti->socket)) {
                        pr_error("[SARA-R1] Failed to configure socket\r\n");
                        return false;
        }

        return sara_r4_start_direct_mode(sb, ti->socket);
}

static bool sara_r4_disconnect(struct serial_buffer *sb,
                                 struct cellular_info *ci,
                                 struct telemetry_info *ti)
{
        if (!sara_r4_stop_direct_mode(sb)) {
                /* Then we don't know if can issue commands */
                pr_warning("[sara_r4] Failed to escape Direct Mode\r\n");
        }

        if (!sara_r4_close_tcp_socket(sb, ti->socket)) {
                pr_warning_int_msg("[sara_r4] Failed to close socket ",
                                   ti->socket);
                return false;
        }

        ti->socket = -1;
        return true;
}

static const struct at_config* sara_r4_get_at_config()
{
        /* Optimized AT config values for U-blox sara r4 */
        static const struct at_config cfg = {
                .urc_delay_ms = 100,
        };

        return &cfg;
}


static const struct cell_modem_methods sara_r4_methods = {
        .get_at_config = sara_r4_get_at_config,
        .init_modem = sara_r4_init,
        .get_sim_info = sara_r4_get_sim_info,
        .register_on_network = sara_r4_register_on_network,
        .get_network_info = sara_r4_get_network_reg_info,
        .setup_pdp = sara_r4_setup_pdp,
        .open_telem_connection = sara_r4_connect_rcl_telem,
        .close_telem_connection = sara_r4_disconnect,
};

const struct cell_modem_methods* get_sara_r4_methods()
{
        return &sara_r4_methods;
}
