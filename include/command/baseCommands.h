/**
 * AutoSport Labs - Race Capture Firmware
 *
 * Copyright (C) 2014 AutoSport Labs
 *
 * This file is part of the Race Capture firmware suite
 *
 * This is free software: you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details. You should have received a copy of the GNU
 * General Public License along with this code. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef BASECOMMANDS_H_
#define BASECOMMANDS_H_

#include "constants.h"
#include "serial.h"

#define BASE_COMMANDS \
		{"showTasks", "Show status of running tasks", "", ShowTaskInfo}, \
		{"version", "Gets the version numbers", "", GetVersion}, \
		{"showStats", "Info on system statistics.","", ShowStats}, \
		{"sysReset", "Reset the system", "", ResetSystem}

void ShowTaskInfo(Serial *serial, unsigned int argc, char **argv);
void GetVersion(Serial *serial, unsigned int argc, char **argv);
void ShowStats(Serial *serial, unsigned int argc, char **argv);
void ResetSystem(Serial *serial, unsigned int argc, char **argv);

#endif /* BASECOMMANDS_H_ */
