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

#ifndef CELLMODEM_H_
#define CELLMODEM_H_

#include "cpp_guard.h"
#include "serial.h"
#include "loggerConfig.h"

#include <stdint.h>

CPP_GUARD_BEGIN

typedef enum {
    CELLMODEM_STATUS_NOT_INIT = 0,
    CELLMODEM_STATUS_PROVISIONED,
    CELLMODEM_STATUS_NO_NETWORK
} cellmodem_status_t;

cellmodem_status_t cellmodem_get_status( void );

int cell_get_signal_strength();
char * cell_get_subscriber_number();
char * cell_get_IMEI();
void setCellBuffer(char *buffer, size_t len);
int loadDefaultCellConfig(Serial *serial);
int initCellModem(Serial *serial, CellularConfig *cellCfg);
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

CPP_GUARD_END

#endif
