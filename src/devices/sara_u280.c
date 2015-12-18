/*
 * Race Capture Pro Firmware
 *
 * Copyright (C) 2015 Autosport Labs
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

#include "array_utils.h"
#include "cellular.h"
#include "gsm.h"
#include "printk.h"
#include "serial_buffer.h"
#include "sara_u280.h"
#include "sim900_device.h"
#include "taskUtil.h"

#include <string.h>
#include <stdlib.h>

bool sara_u280_get_subscriber_number(struct serial_buffer *sb,
                                     struct cellular_info *ci)
{
        return gsm_get_subscriber_number(sb, ci);
}

bool sara_u280_get_signal_strength(struct serial_buffer *sb,
                                   struct cellular_info *ci)
{
        return gsm_get_subscriber_number(sb, ci);
}

bool sara_u280_get_imei(struct serial_buffer *sb,
                        struct cellular_info *ci)
{
        return gsm_get_imei(sb, ci);
}

enum cellular_net_status sara_u280_get_net_reg_status(
        struct serial_buffer *sb,struct cellular_info *ci)
{
        return gsm_get_network_reg_status(sb, ci);
}

bool sara_u280_get_network_reg_info(struct serial_buffer *sb,
                                    struct cellular_info *ci)
{
        return gsm_get_network_reg_info(sb, ci);
}

bool sara_u280_is_gprs_attached(struct serial_buffer *sb)
{
        return gsm_is_gprs_attached(sb);
}

bool sara_u280_get_ip_address(struct serial_buffer *sb)
{
        const char *cmd = "AT+UPSND=0,0";
        const char *msgs[2];
        const size_t msgs_len = ARRAY_LEN(msgs);

        serial_buffer_reset(sb);
        serial_buffer_append(sb, cmd);
        const size_t count = cellular_exec_cmd(sb, READ_TIMEOUT, msgs, msgs_len);
        const bool status = is_rsp_ok(msgs, count);
        if (!status) {
                pr_warning("[cell] Failed to read IP\r\n");
                return false;
        }

        /* +UPSND: 0,0,"93.68.225.175" */
        const char* ipaddr = msgs[0];
        for (size_t i = 2; i && ipaddr; --i)
                ipaddr = strchr(ipaddr, ',');

        if (ipaddr) {
                pr_info_str_msg("[cell] IP address: ", ipaddr);
        } else {
                pr_warning_str_msg("[cell] Unable to parse IP address: ", msgs[0]);
        }

        return ipaddr != NULL;
}

void sara_u280_power_cycle(bool force_hard)
{
        sim900_device_power_button(1);
        delayMs(2000);
        sim900_device_power_button(0);
        delayMs(3000);
}

bool sara_u280_put_pdp_config(struct serial_buffer *sb, const int pdp_id,
                              const char *host, const char* user,
                              const char* password)
{
        const char *msgs[1];
        const size_t msgs_len = ARRAY_LEN(msgs);

        serial_buffer_reset(sb);

        /* APN settings */
        serial_buffer_printf_append(sb, "AT+UPSD=0,1,\"%s\"", host);
        serial_buffer_printf_append(sb, ";+UPSD=0,2,\"%s\"", user);
        serial_buffer_printf_append(sb, ";+UPSD=0,3,\"%s\"", password);

        /* Dynamic IP */
        serial_buffer_append(sb, ";+UPSD=0,7,\"0.0.0.0\"");


        const size_t count = cellular_exec_cmd(sb, READ_TIMEOUT, msgs, msgs_len);
        return is_rsp_ok(msgs, count);
}

bool sara_u280_put_dns_config(struct serial_buffer *sb, const char* dns1,
                              const char *dns2)
{
        const char *msgs[1];
        const size_t msgs_len = ARRAY_LEN(msgs);

        serial_buffer_reset(sb);
        serial_buffer_printf_append(sb, "AT+UPSD=0,4,\"%s\"", dns1);
        if (dns2) {
                serial_buffer_append(sb, ";");
                serial_buffer_printf_append(sb, "+UPSD=0,5,\"%s\"", dns2);
        }

        const size_t count = cellular_exec_cmd(sb, READ_TIMEOUT, msgs, msgs_len);
        return is_rsp_ok(msgs, count);
}

static bool sara_u280_gprs_psd_action(struct serial_buffer *sb, const int pid,
                                      const int aid)
{
        const char *msgs[1];
        const size_t msgs_len = ARRAY_LEN(msgs);

        serial_buffer_reset(sb);
        serial_buffer_printf_append(sb, "AT+UPSDA=%d,%d", pid, aid);
        const size_t count = cellular_exec_cmd(sb, MEDIUM_TIMEOUT, msgs, msgs_len);
        const bool status = is_rsp_ok(msgs, count);

        return status;
}

bool sara_u280_activate_pdp(struct serial_buffer *sb, const int pdp_id)
{
        return sara_u280_gprs_psd_action(sb, pdp_id, 3);
}

bool sara_u280_deactivate_pdp(struct serial_buffer *sb, const int pdp_id)
{
        return sara_u280_gprs_psd_action(sb, pdp_id, 4);
}

int sara_u280_create_tcp_socket(struct serial_buffer *sb)
{
        const char *cmd = "AT+USOCR=6";
        const char *msgs[2];
        const size_t msgs_len = ARRAY_LEN(msgs);

        serial_buffer_reset(sb);
        serial_buffer_append(sb, cmd);
        const size_t count = cellular_exec_cmd(sb, READ_TIMEOUT, msgs, msgs_len);
        const bool status = is_rsp_ok(msgs, count);
        if (!status) {
                pr_warning("[sara_u280] Failed to create a socket\r\n");
                return -1;
        }

        /*
         * Parse out the socket ID.  Needed for Muxing.
         * +USOCR: <socket_id>
         */
        const int socket = msgs[0][8] - '0';
        if (socket < 0 || socket > 6) {
                pr_warning("[sara_u280] Failed to parse socket ID.\r\n");
                return -2;
        }

        pr_debug_int_msg("[sara_u280] Socket ID: ", socket);
        return socket;
}

bool sara_u280_connect_tcp_socket(struct serial_buffer *sb,
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

bool sara_u280_close_tcp_socket(struct serial_buffer *sb,
                                const int socket_id)
{
        const char *msgs[1];
        const size_t msgs_len = ARRAY_LEN(msgs);

        serial_buffer_reset(sb);
        serial_buffer_printf_append(sb, "AT+USOCL=%d", socket_id);
        const size_t count = cellular_exec_cmd(sb, MEDIUM_TIMEOUT, msgs,
                                               msgs_len);
        return is_rsp_ok(msgs, count);
}

bool sara_u280_start_direct_mode(struct serial_buffer *sb,
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

bool sara_u280_stop_direct_mode(struct serial_buffer *sb)
{
        /* Must delay min 2000ms before stopping direct mode */
	delayMs(2100);

        serial_buffer_reset(sb);
        serial_buffer_append(sb, "+++");
        cellular_exec_cmd(sb, 0, NULL, 0);

	delayMs(500);

        size_t tries = 10;
        for (; tries; --tries) {
                if (gsm_ping_modem(sb))
                        break;
        }
        return tries > 0;
}
