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

#ifndef _SIM900_H_
#define _SIM900_H_

#include "cellular.h"
#include "serial_buffer.h"

#include <stdbool.h>

bool sim900_get_subscriber_number(struct serial_buffer *sb,
                                  struct cellular_info *ci);

bool sim900_get_signal_strength(struct serial_buffer *sb,
                                struct cellular_info *ci);

bool sim900_get_imei(struct serial_buffer *sb,
                     struct cellular_info *ci);

enum cellular_net_status sim900_get_net_reg_status(struct serial_buffer *sb,
                                                   struct cellular_info *ci);

int sim900_get_network_reg_info(struct serial_buffer *sb,
                                struct cellular_info *ci);

bool sim900_is_gprs_attached(struct serial_buffer *sb);

bool sim900_get_ip_address(struct serial_buffer *sb);

bool sim900_put_dns_config(struct serial_buffer *sb, const char* dns1,
                           const char *dns2);

bool sim900_put_pdp_config(struct serial_buffer *sb, const int pdp_id,
                           const char *apn_host, const char* apn_user,
                           const char* apn_password);

bool sim900_activate_pdp(struct serial_buffer *sb, const int pdp_id);

bool sim900_deactivate_pdp(struct serial_buffer *sb, const int pdp_id);

int sim900_create_tcp_socket(struct serial_buffer *sb);

bool sim900_connect_tcp_socket(struct serial_buffer *sb,
                               const int socket_id,
                               const char* host,
                               const int port);

bool sim900_close_tcp_socket(struct serial_buffer *sb,
                             const int socket_id);

bool sim900_start_direct_mode(struct serial_buffer *sb,
                              const int socket_id);

bool sim900_stop_direct_mode(struct serial_buffer *sb);

#endif /* _SIM900_H_ */
