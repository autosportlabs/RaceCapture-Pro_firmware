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

#ifndef BASECOMMANDS_H_
#define BASECOMMANDS_H_

#include "command.h"
#include "cpp_guard.h"
#include "constants.h"
#include "serial.h"

CPP_GUARD_BEGIN

#define BASE_COMMANDS                                                   \
        SYSTEM_COMMAND("showTasks", "Show status of running tasks", "", \
                       ShowTaskInfo)                                    \
        SYSTEM_COMMAND("version", "Gets the version numbers", "",       \
                       GetVersion)                                      \
        SYSTEM_COMMAND("showStats", "Info on system statistics.","",    \
                       ShowStats)                                       \
        SYSTEM_COMMAND("sysReset", "Reset the system",                  \
                       "[bootloader 0|1]", ResetSystem)


void ShowTaskInfo(struct Serial *serial, unsigned int argc, char **argv);
void GetVersion(struct Serial *serial, unsigned int argc, char **argv);
void ShowStats(struct Serial *serial, unsigned int argc, char **argv);
void ResetSystem(struct Serial *serial, unsigned int argc, char **argv);

CPP_GUARD_END

#endif /* BASECOMMANDS_H_ */
