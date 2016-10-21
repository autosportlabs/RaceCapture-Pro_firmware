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

#include "cellular.h"
#include "gsm.h"
#include "macros.h"
#include "printk.h"
#include "serial_buffer.h"
#include "str_util.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define READ_TIMEOUT 	500

bool gsm_ping_modem(struct serial_buffer *sb)
{
        const char *msgs[2];
        const size_t msgs_len = ARRAY_LEN(msgs);

        serial_buffer_reset(sb);
        serial_buffer_append(sb, "AT");
        const size_t count = cellular_exec_cmd(sb, READ_TIMEOUT, msgs,
                                               msgs_len);
        return is_rsp_ok(msgs, count);
}

/**
 * Turns on/off echo from the modem.
 */
bool gsm_set_echo(struct serial_buffer *sb, bool on)
{
        const char *cmd = on ? "ATE1" : "ATE0";
        const char *msgs[2];
        const size_t msgs_len = ARRAY_LEN(msgs);

        serial_buffer_reset(sb);
        serial_buffer_append(sb, cmd);
        const size_t count = cellular_exec_cmd(sb, READ_TIMEOUT, msgs, msgs_len);
        const bool status = is_rsp_ok(msgs, count);

        if (!status)
                pr_warning("[gsm] Failed to alter echoing.\r\n");

        return status;
}


bool gsm_get_subscriber_number(struct serial_buffer *sb,
                               struct cellular_info *ci)
{
        const char *cmd = "AT+CNUM";
        const char *msgs[2];
        const size_t msgs_len = ARRAY_LEN(msgs);

        serial_buffer_reset(sb);
        serial_buffer_append(sb, cmd);
        const size_t count = cellular_exec_cmd(sb, READ_TIMEOUT, msgs,
                                               msgs_len);
        const bool status = is_rsp_ok(msgs, count);

        if (!status) {
                pr_warning("[gsm] Failed to read phone number\r\n");
                return false;
        }

        char *num_start = strstr(msgs[0], ",\"");
        if (!num_start)
                goto parse_fail;

        num_start += 2;
        char *num_end = strstr(num_start, "\"");
        if (!num_end)
                goto parse_fail;

        *num_end = '\0';
        strntcpy(ci->number, num_start, sizeof(ci->number));
        return true;

parse_fail:
        pr_warning("[gsm] Failed to parse phone number\r\n");
        return false;
}

bool gsm_get_signal_strength(struct serial_buffer *sb,
                             struct cellular_info *ci)
{
        const char *cmd = "AT+CSQ";
        char *msgs[2];
        const char **const_msgs = (const char **) msgs; /* Stupid Compiler */
        const size_t msgs_len = ARRAY_LEN(msgs);

        serial_buffer_reset(sb);
        serial_buffer_append(sb, cmd);
        const size_t count = cellular_exec_cmd(
                sb, READ_TIMEOUT, const_msgs, msgs_len);
        const bool status = is_rsp_ok(const_msgs, count);

        if (!status) {
                pr_warning("[gsm] Failed to read signal strength\r\n");
                return false;
        }

        char *rssi_string = strstr(msgs[0], ": ") + 2;
        if (rssi_string != NULL) {
                char *comma = strchr(rssi_string, ',');
                *comma = '\0';
                const int signal = atoi(rssi_string);
                ci->signal = signal;
        }

        /* TODO: Interpret cellular value per page in ublox AT CMD Sheet */
        return true;
}

bool gsm_get_imei(struct serial_buffer *sb,
                  struct cellular_info *cell_info)
{
        const char *cmd = "AT+CGSN";
        const char *msgs[2];
        const size_t msgs_len = ARRAY_LEN(msgs);

        serial_buffer_reset(sb);
        serial_buffer_append(sb, cmd);
        const size_t count = cellular_exec_cmd(sb, READ_TIMEOUT, msgs, msgs_len);
        const bool status = is_rsp_ok(msgs, count);

        if (!status) {
                pr_warning("[sara_u280] Failed to read IMEI\r\n");
                return false;
        }

	strntcpy(cell_info->imei, msgs[0], sizeof(cell_info->imei));
	rstrip_inline(cell_info->imei);

        return 1;
}

enum cellular_net_status gsm_get_network_reg_status(
        struct serial_buffer *sb, struct cellular_info *ci)
{
        const char *cmd = "AT+CGREG?";
        const char *msgs[2];
        const size_t msgs_len = ARRAY_LEN(msgs);
        const char *answrs[] = {"+CGREG: 0,0",
                                "+CGREG: 0,1",
                                "+CGREG: 0,2",
                                "+CGREG: 0,3",
                                "+CGREG: 0,4",
                                "+CGREG: 0,5"};
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
        }

        return ci->net_status;
}

bool gsm_is_gprs_attached(struct serial_buffer *sb)
{
        const char *cmd = "AT+CGATT?";
        const char *msgs[2];
        const size_t msgs_len = ARRAY_LEN(msgs);
        const char *answrs[] = {"+CGATT: 0",
                                "+CGATT: 1",};
        const size_t answrs_len = ARRAY_LEN(answrs);

        serial_buffer_reset(sb);
        serial_buffer_append(sb, cmd);
        return cellular_exec_match(sb, READ_TIMEOUT, msgs, msgs_len,
                                   answrs, answrs_len, 0) == 1;
}

bool gsm_get_network_reg_info(struct serial_buffer *sb,
                              struct cellular_info *ci)
{
        const char *cmd = "AT+COPS?";
        const char *msgs[2];
        const size_t msgs_len = ARRAY_LEN(msgs);

        serial_buffer_reset(sb);
        serial_buffer_append(sb, cmd);
        const size_t count = cellular_exec_cmd(sb, READ_TIMEOUT, msgs, msgs_len);
        const bool status = is_rsp_ok(msgs, count);
        if (!status) {
                pr_warning("[gsm] Failed to get network info\r\n");
                return false;
        }

        /*
         * Network provider may be there.  May not be.
         * +COPS: <mode>[,<format>,<oper>[,<AcT>]]
         */
       const char *str_beg = strchr(msgs[0], '"');
        if (NULL != str_beg) {
                ++str_beg;
                char *str_end = strchr(str_beg, '"');
                *str_end = '\0';
        } else {
                str_beg = "UNKNOWN";
        }

        strntcpy(ci->op, str_beg, sizeof(ci->op));
        return status;
}
