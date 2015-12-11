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
#include "loggerConfig.h"
#include "serial_buffer.h"

#include <stdint.h>

int initCellModem(struct serial_buffer *sb, CellularConfig *cellCfg,
                  struct cellular_info *cell_info);
int configureNet(struct serial_buffer *sb);
int connectNet(struct serial_buffer *sb, const char *host, const char *port, int udpMode);
int closeNet(struct serial_buffer *sb);
int isNetConnectionErrorOrClosed(struct serial_buffer *sb);

#endif /* _SIM900_H_ */
