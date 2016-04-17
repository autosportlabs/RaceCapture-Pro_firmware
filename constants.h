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

#ifndef CONSTANTS_H_
#define CONSTANTS_H_

/* Include all of the command definitions here */
#include "baseCommands.h"
#include "loggerCommands.h"
#include "loggerApi.h"
#include "api.h"
#include "command.h"
#include "luaCommands.h"

#define XSTR(s) STR(s)
#define STR(s) #s

#define MAJOR_REV_STR  XSTR(MAJOR_REV)
#define MINOR_REV_STR  XSTR(MINOR_REV)
#define BUGFIX_REV_STR XSTR(BUGFIX_REV)

#define WATCHDOG_TIMEOUT_MS 2000

#define RCP_SERVICE_PORT	7223

#endif /* CONSTANTS_H_ */
