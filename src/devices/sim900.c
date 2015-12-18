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
#include "sim900.h"
#include "taskUtil.h"

#include <stdbool.h>
#include <string.h>

bool sim900_get_subscriber_number(struct serial_buffer *sb,
                                  struct cellular_info *ci)
{
        return gsm_get_subscriber_number(sb, ci);
}

bool sim900_get_signal_strength(struct serial_buffer *sb,
                                struct cellular_info *ci)
{
        return gsm_get_signal_strength(sb, ci);
}

bool sim900_get_imei(struct serial_buffer *sb,
                     struct cellular_info *ci)
{
        return gsm_get_imei(sb, ci);
}

enum cellular_net_status sim900_get_net_reg_status(struct serial_buffer *sb,
                                                   struct cellular_info *ci)
{
        return gsm_get_network_reg_info(sb, ci);
}

bool sim900_is_gprs_attached(struct serial_buffer *sb)
{
        return gsm_is_gprs_attached(sb);
}

int sim900_get_network_reg_info(struct serial_buffer *sb,
                                struct cellular_info *ci)
{
        return gsm_get_network_reg_info(sb, ci);
}

bool sim900_get_ip_address(struct serial_buffer *sb)
{
        const char *msgs[1];
        const size_t msgs_len = ARRAY_LEN(msgs);

        serial_buffer_reset(sb);
        serial_buffer_append(sb, "AT+CIFSR");

        const size_t count = cellular_exec_cmd(sb, READ_TIMEOUT, msgs, msgs_len);
        const bool status = is_rsp(msgs, count, "ERROR");
        if (!status) {
                pr_warning("[sim900] Failed to read IP\r\n");
                return false;
        }

        pr_info_str_msg("[sim900] IP address: ", msgs[0]);
        return true;
}

bool sim900_put_pdp_config(struct serial_buffer *sb, const int pdp_id,
                           const char *apn_host, const char* apn_user,
                           const char* apn_password)
{
        const char *msgs[1];
        const size_t msgs_len = ARRAY_LEN(msgs);

        serial_buffer_reset(sb);
        serial_buffer_printf_append(sb, "AT+CSTT=\"%s\",\"%s\",\"%s\"",
                                    apn_host, apn_user, apn_password);
        const size_t count = cellular_exec_cmd(sb, READ_TIMEOUT, msgs, msgs_len);
        return is_rsp_ok(msgs, count);
}

bool sim900_put_dns_config(struct serial_buffer *sb, const char* dns1,
                           const char *dns2)
{
        const char *msgs[1];
        const size_t msgs_len = ARRAY_LEN(msgs);

        serial_buffer_reset(sb);
        serial_buffer_printf_append(sb, "AT+CDNSCFG=\"%s\"", dns1);
        if (dns2)
                serial_buffer_printf_append(sb, ",\"%s\"", dns2);

        const size_t count = cellular_exec_cmd(sb, READ_TIMEOUT, msgs, msgs_len);
        return is_rsp_ok(msgs, count);
}

bool sim900_activate_pdp(struct serial_buffer *sb, const int pdp_id)
{
        const char *msgs[1];
        const size_t msgs_len = ARRAY_LEN(msgs);

        serial_buffer_reset(sb);
        serial_buffer_append(sb, "AT+CIICR");
        const size_t count = cellular_exec_cmd(sb, READ_TIMEOUT, msgs, msgs_len);
        return is_rsp_ok(msgs, count);
}

bool sim900_deactivate_pdp(struct serial_buffer *sb, const int pdp_id)
{
        const char *msgs[1];
        const size_t msgs_len = ARRAY_LEN(msgs);

        serial_buffer_reset(sb);
        serial_buffer_append(sb, "AT+CIPSHUT");
        const size_t count = cellular_exec_cmd(sb, READ_TIMEOUT, msgs, msgs_len);
        return is_rsp(msgs, count, "SHUT OK");

        return false;
}

static bool sim900_disable_mux(struct serial_buffer *sb)
{
        const char *msgs[1];
        const size_t msgs_len = ARRAY_LEN(msgs);

        serial_buffer_reset(sb);
        serial_buffer_append(sb, "AT+CIPMUX=0");
        const size_t count = cellular_exec_cmd(sb, READ_TIMEOUT, msgs,
                                               msgs_len);
        return is_rsp_ok(msgs, count);
}

int sim900_create_tcp_socket(struct serial_buffer *sb)
{
        /* Single connection only (for now) */
        return sim900_disable_mux(sb);
}

bool sim900_connect_tcp_socket(struct serial_buffer *sb,
                               const int socket_id,
                               const char *host,
                               const int port)
{
        const char *msgs[1];
        const size_t msgs_len = ARRAY_LEN(msgs);

        serial_buffer_reset(sb);
        serial_buffer_printf_append(sb, "AT+CIPSTART=\"%s\",\"%s\",\"%d\"",
                                    "TCP", host, port);
        const size_t count = cellular_exec_cmd(sb, READ_TIMEOUT, msgs,
                                               msgs_len);
        return is_rsp_ok(msgs, count);
}

bool sim900_close_tcp_socket(struct serial_buffer *sb,
                             const int socket_id)
{
        const char *msgs[1];
        const size_t msgs_len = ARRAY_LEN(msgs);

        serial_buffer_reset(sb);
        serial_buffer_append(sb, "AT+CIPCLOSE");
        const size_t count = cellular_exec_cmd(sb, READ_TIMEOUT, msgs,
                                               msgs_len);
        return is_rsp_ok(msgs, count);

}

bool sim900_start_direct_mode(struct serial_buffer *sb,
                              const int socket_id)
{
        /* NO-OP as already in direct mode when conneciton open */
        /* Just clear the serial buffer */
        serial_buffer_reset(sb);
        return true;
}

bool sim900_stop_direct_mode(struct serial_buffer *sb)
{
        /* Must delay min 1000ms before stopping direct mode */
	delayMs(1100);

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
