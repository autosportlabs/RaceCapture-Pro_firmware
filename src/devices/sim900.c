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

#include "cellular.h"
#include "gsm.h"
#include "macros.h"
#include "printk.h"
#include "serial_buffer.h"
#include "sim900.h"
#include "taskUtil.h"

#include <stdbool.h>
#include <string.h>

#define READ_TIMEOUT 	500
#define MEDIUM_TIMEOUT 	15000
#define CONNECT_TIMEOUT 60000

#define GPRS_ACTIVATE_PDP_ATTEMPTS	3
#define GPRS_ACTIVATE_PDP_BACKOFF_MS	2000
#define GPRS_ATTACH_ATTEMPTS	10
#define GPRS_ATTACH_BACKOFF_MS	2000
#define NET_REG_ATTEMPTS	10
#define NET_REG_BACKOFF_MS	3000
#define STOP_DM_RX_EVENTS	10
#define STOP_DM_RX_TIMEOUT_MS	1000

static bool sim900_set_multiple_connections(struct serial_buffer *sb,
                                            const bool enable)
{
        const char *msgs[1];
        const size_t msgs_len = ARRAY_LEN(msgs);

        serial_buffer_reset(sb);
        serial_buffer_printf_append(sb, "AT+CIPMUX=%d", enable ? 1 : 0);
        const size_t count = cellular_exec_cmd(sb, READ_TIMEOUT, msgs,
                                               msgs_len);
        return is_rsp_ok(msgs, count);
}

static bool sim900_set_transparent_mode(struct serial_buffer *sb,
                                        const bool enable)
{
        const char *msgs[1];
        const size_t msgs_len = ARRAY_LEN(msgs);

        serial_buffer_reset(sb);
        serial_buffer_printf_append(sb, "AT+CIPMODE=%d", enable ? 1 : 0);
        const size_t count = cellular_exec_cmd(sb, READ_TIMEOUT, msgs,
                                               msgs_len);
        return is_rsp_ok(msgs, count);
}

static bool sim900_set_flow_control(struct serial_buffer *sb,
                                    const int dce_by_dte,
                                    const int dte_by_dce)
{
        const char *msgs[1];
        const size_t msgs_len = ARRAY_LEN(msgs);

        serial_buffer_reset(sb);
        serial_buffer_printf_append(sb, "AT+IFC=%d,%d", dce_by_dte,
                                    dte_by_dce);
        const size_t count = cellular_exec_cmd(sb, READ_TIMEOUT, msgs,
                                               msgs_len);
        return is_rsp_ok(msgs, count);
}


static bool sim900_init_settings(struct serial_buffer *sb,
                                 struct cellular_info *ci)
{
        sim900_set_flow_control(sb, 0, 0);

        /*
         * Don't check the return status here because if the modem has
         * to restart logging then this can fail.  Silly SIM900.
         */
        sim900_set_multiple_connections(sb, false);
        sim900_set_transparent_mode(sb, true);
        return true;
}

static bool sim900_get_subscriber_number(struct serial_buffer *sb,
                                         struct cellular_info *ci)
{
        return gsm_get_subscriber_number(sb, ci);
}

static bool sim900_get_signal_strength(struct serial_buffer *sb,
                                       struct cellular_info *ci)
{
        return gsm_get_signal_strength(sb, ci);
}

static bool sim900_get_imei(struct serial_buffer *sb,
                            struct cellular_info *ci)
{
        return gsm_get_imei(sb, ci);
}

static enum cellular_net_status sim900_get_net_reg_status(
        struct serial_buffer *sb,
        struct cellular_info *ci)
{
        return gsm_get_network_reg_status(sb, ci);
}

static bool sim900_is_gprs_attached(struct serial_buffer *sb)
{
        return gsm_is_gprs_attached(sb);
}

static bool sim900_get_network_reg_info(struct serial_buffer *sb,
                                        struct cellular_info *ci)
{
        return gsm_get_network_reg_info(sb, ci);
}

static bool sim900_get_ip_address(struct serial_buffer *sb)
{
        const char *msgs[1];
        const size_t msgs_len = ARRAY_LEN(msgs);

        serial_buffer_reset(sb);
        serial_buffer_append(sb, "AT+CIFSR");

        const size_t count = cellular_exec_cmd(sb, READ_TIMEOUT, msgs,
                                               msgs_len);
        const bool status = is_rsp(msgs, count, "ERROR");
        if (status) {
                pr_warning("[sim900] Failed to read IP\r\n");
                return false;
        }

        pr_info_str_msg("[sim900] IP address: ", msgs[0]);
        return true;
}

static bool sim900_put_pdp_config(struct serial_buffer *sb, const int pdp_id,
                                  const char *apn_host, const char* apn_user,
                                  const char* apn_password)
{
        const char *msgs[1];
        const size_t msgs_len = ARRAY_LEN(msgs);

        serial_buffer_reset(sb);
        serial_buffer_printf_append(sb, "AT+CSTT=\"%s\",\"%s\",\"%s\"",
                                    apn_host, apn_user, apn_password);
        const size_t count = cellular_exec_cmd(sb, READ_TIMEOUT, msgs,
                                               msgs_len);
        return is_rsp_ok(msgs, count);
}

static bool sim900_put_dns_config(struct serial_buffer *sb, const char* dns1,
                                  const char *dns2)
{
        const char *msgs[1];
        const size_t msgs_len = ARRAY_LEN(msgs);

        serial_buffer_reset(sb);
        serial_buffer_printf_append(sb, "AT+CDNSCFG=\"%s\"", dns1);
        if (dns2)
                serial_buffer_printf_append(sb, ",\"%s\"", dns2);

        const size_t count = cellular_exec_cmd(sb, READ_TIMEOUT, msgs,
                                               msgs_len);
        return is_rsp_ok(msgs, count);
}

static bool sim900_activate_pdp(struct serial_buffer *sb, const int pdp_id)
{
        const char *msgs[1];
        const size_t msgs_len = ARRAY_LEN(msgs);

        serial_buffer_reset(sb);
        serial_buffer_append(sb, "AT+CIICR");
        const size_t count = cellular_exec_cmd(sb, MEDIUM_TIMEOUT, msgs,
                                               msgs_len);
        const bool status = is_rsp_ok(msgs, count);

        /*
         * Delay if successful here b/c sim900 throws lots of garbage onto
         * the serial bus.
         */
        if (status)
                delayMs(GPRS_ACTIVATE_PDP_BACKOFF_MS);

        return status;
}

static bool sim900_connect_tcp_socket(struct serial_buffer *sb,
                                      const char *host,
                                      const int port)
{
        const char *msgs[2];
        const size_t msgs_len = ARRAY_LEN(msgs);

        serial_buffer_reset(sb);
        serial_buffer_printf_append(sb, "AT+CIPSTART=\"%s\",\"%s\",\"%d\"",
                                    "TCP", host, port);
        const size_t count = cellular_exec_cmd(sb, CONNECT_TIMEOUT, msgs,
                                               msgs_len);
        return is_rsp_ok(msgs, 1) && (
                0 == strcmp("CONNECT OK\r\n", msgs[count - 1]) ||
                0 == strcmp("CONNECT\r\n", msgs[count - 1]) ||
                0 == strcmp("ALREADY CONNECT\r\n", msgs[count - 1]));
}

static bool sim900_close_tcp_socket(struct serial_buffer *sb,
                                    const int socket_id)
{
        const char *msgs[1];
        const size_t msgs_len = ARRAY_LEN(msgs);

        serial_buffer_reset(sb);
        serial_buffer_append(sb, "AT+CIPCLOSE");
        const size_t count = cellular_exec_cmd(sb, MEDIUM_TIMEOUT, msgs,
                                               msgs_len);
        return is_rsp(msgs, count, "CLOSE OK");
}

static bool sim900_stop_direct_mode(struct serial_buffer *sb)
{
        /* Must delay min 1000ms before stopping direct mode */
	delayMs(2000);

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
                    is_rsp_ok((const char**) &(sb->buffer), 1))
                        return true;
        }

        return false;
}

static bool sim900_get_sim_info(struct serial_buffer *sb,
                                struct cellular_info *ci)
{
        return sim900_get_imei(sb, ci) &&
                sim900_get_signal_strength(sb, ci) &&
                sim900_get_subscriber_number(sb, ci);
}

static bool sim900_register_on_network(struct serial_buffer *sb,
                                       struct cellular_info *ci)
{
        /* Check our status on the network */
        for (size_t tries = NET_REG_ATTEMPTS; tries; --tries) {
                sim900_get_net_reg_status(sb, ci);

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

static bool sim900_setup_pdp(struct serial_buffer *sb,
                             struct cellular_info *ci,
                             const CellularConfig *cc)
{
        /* Check GPRS attached */
        bool gprs_attached;
        for (size_t tries = GPRS_ATTACH_ATTEMPTS; tries; --tries) {
                gprs_attached = sim900_is_gprs_attached(sb);

                if (gprs_attached)
                        break;

                /* Wait before trying again.  Arbitrary backoff */
                delayMs(GPRS_ATTACH_BACKOFF_MS);
        }

        pr_info_str_msg("[sim900] GPRS Attached: ",
                        gprs_attached ? "yes" : "no");
        if (!gprs_attached)
                return false;

        /* Setup APN */
        const bool status =
                sim900_put_pdp_config(sb, 0, cc->apnHost, cc->apnUser,
                                      cc->apnPass);
        if (!status) {
                pr_warning("[sim900] APN config failed\r\n");
                return false;
        }

        bool gprs_active;
        for (size_t tries = GPRS_ACTIVATE_PDP_ATTEMPTS; tries; --tries) {
                gprs_active = sim900_activate_pdp(sb, 0);

                if (gprs_active)
                        break;

                /* Wait before trying again.  Arbitrary backoff */
                delayMs(GPRS_ACTIVATE_PDP_BACKOFF_MS);
        }

        if (!gprs_active) {
                pr_warning("[sim900] Failed connect GPRS\r\n");
                return false;
        }

        pr_debug("[sim900] GPRS connected\r\n");

        /* Wait to get the IP */
        bool has_ip = false;
        for (size_t tries = 5; tries && !has_ip; --tries) {
                has_ip = sim900_get_ip_address(sb);
                delayMs(1000);
        }
        if (!has_ip)
                return false;

        pr_debug("[sim900] IP acquired\r\n");

        if (!sim900_put_dns_config(sb, cc->dns1, cc->dns2)) {
                /* NON-Fatal */
                pr_warning("[sim900] DNS config failed\r\n");
        }

        return true;
}

static bool sim900_connect_rcl_telem(struct serial_buffer *sb,
                              struct cellular_info *ci,
                              struct telemetry_info *ti,
                              const TelemetryConfig *tc)
{

        if (!sim900_connect_tcp_socket(sb, tc->telemetryServerHost,
                                       tc->telemetry_port)) {
                pr_warning("[sim900] Failed to connect to ");
                pr_warning(tc->telemetryServerHost);
                pr_warning_int_msg(":", tc->telemetry_port);

                return false;
        }
        /*
         * If here, then set a fake socket value because this
         * is how we know if we have created a socket or not.
         */
        ti->socket = 0;
        serial_buffer_reset(sb);
        return true;
}

static bool sim900_disconnect(struct serial_buffer *sb,
                              struct cellular_info *ci,
                              struct telemetry_info *ti)
{
        if (!sim900_stop_direct_mode(sb)) {
                /* Then we don't know if can issue commands */
                pr_warning("[sim900] Failed to escape Direct Mode\r\n");
                return false;
        }

        if (!sim900_close_tcp_socket(sb, ti->socket)) {
                pr_warning_int_msg("[sim900] Failed to close socket ",
                                   ti->socket);
                return false;
        }

        ti->socket = -1;
        return true;
}

static const struct at_config* sim900_get_at_config()
{
        /* Optimized AT config values for SIM900 */
        static const struct at_config cfg = {
                .urc_delay_ms = 250,
        };

        return &cfg;
}

static const struct cell_modem_methods sim900_methods = {
        .get_at_config = sim900_get_at_config,
        .init_modem = sim900_init_settings,
        .get_sim_info = sim900_get_sim_info,
        .register_on_network = sim900_register_on_network,
        .get_network_info = sim900_get_network_reg_info,
        .setup_pdp = sim900_setup_pdp,
        .open_telem_connection = sim900_connect_rcl_telem,
        .close_telem_connection = sim900_disconnect,
};

const struct cell_modem_methods* get_sim900_methods()
{
        return &sim900_methods;
}
