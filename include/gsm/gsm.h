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

#ifndef _GSM_H_
#define _GSM_H_

#include "cellular.h"
#include "serial_buffer.h"

#include <stdbool.h>

bool gsm_ping_modem(struct serial_buffer *sb);

bool gsm_set_echo(struct serial_buffer *sb, bool on);

bool gsm_get_subscriber_number(struct serial_buffer *sb,
                               struct cellular_info *ci);

bool gsm_get_signal_strength(struct serial_buffer *sb,
                             struct cellular_info *cell_info);

bool gsm_get_imei(struct serial_buffer *sb,
                  struct cellular_info *cell_info);

enum cellular_net_status gsm_get_network_reg_status(
        struct serial_buffer *sb, struct cellular_info *ci);

bool gsm_is_gprs_attached(struct serial_buffer *sb);

bool gsm_get_network_reg_info(struct serial_buffer *sb,
                              struct cellular_info *ci);


#endif /* _GSM_H_ */
