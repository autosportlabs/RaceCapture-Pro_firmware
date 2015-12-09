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
#include "serial.h"

#include <stdint.h>

void setCellBuffer(char *buffer, size_t len);
int loadDefaultCellConfig(Serial *serial);
int initCellModem(Serial *serial, CellularConfig *cellCfg,
                  struct cellular_info *cell_info);
int configureNet(Serial *serial);
int connectNet(Serial *serial, const char *host, const char *port, int udpMode);
int closeNet(Serial *serial);
int isNetConnectionErrorOrClosed();
const char * readsCell(Serial *serial, size_t timeout);
void putsCell(Serial *serial, const char *data);
void putQuotedStringCell(Serial *serial, char *s);
void putUintCell(Serial *serial, uint32_t num);
void putIntCell(Serial *serial, int num);
void putFloatCell(Serial *serial, float num, int precision);

#endif /* _SIM900_H_ */
