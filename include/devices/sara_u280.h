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

#ifndef _SARA_U280_H_
#define _SARA_U280_H_

#include "cellular.h"
#include "loggerConfig.h"
#include "serial_buffer.h"

#include <stdint.h>

bool sara_u280_get_subscriber_number(struct serial_buffer *sb,
                                     struct cellular_info *ci);

bool sara_u280_get_signal_strength(struct serial_buffer *sb,
                                   struct cellular_info *ci);

bool sara_u280_get_imei(struct serial_buffer *sb,
                        struct cellular_info *ci);

enum cellular_net_status get_net_reg_status(struct serial_buffer *sb,
                                            struct cellular_info *ci);

bool sara_u280_is_gprs_attached(struct serial_buffer *sb);

bool sara_u280_set_dynamic_ip(struct serial_buffer *sb);

bool sara_u280_get_ip_address(struct serial_buffer *sb);

bool sara_u280_put_apn_config(struct serial_buffer *sb, const char *host,
                              const char* user, const char* password);

bool sara_u280_put_dns_config(struct serial_buffer *sb, const char* dns1,
                              const char *dns2);

int get_network_reg_info(struct serial_buffer *sb,
                         struct cellular_info *ci);

bool sara_u280_activate_gprs(struct serial_buffer *sb);

bool deactivate_gprs_connection(struct serial_buffer *sb);

int sara_u280_create_tcp_socket(struct serial_buffer *sb);

bool sara_u280_connect_tcp_socket(struct serial_buffer *sb,
                                  const int socket_id,
                                  const char* host,
                                  const int port);

bool sara_u280_close_tcp_socket(struct serial_buffer *sb,
                                const int socket_id);

bool sara_u280_start_direct_mode(struct serial_buffer *sb,
                                 const int socket_id);

bool sara_u280_stop_direct_mode(struct serial_buffer *sb);

#endif /* _SARA_U280_H_ */
