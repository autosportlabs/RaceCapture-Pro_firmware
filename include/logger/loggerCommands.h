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

#ifndef LOGGERCOMMANDS_H_
#define LOGGERCOMMANDS_H_

#include "cpp_guard.h"
#include "command.h"

CPP_GUARD_BEGIN

#define LOGGER_COMMANDS                                                 \
        SYSTEM_COMMAND("resetConfig", "Resets All configuration Data "  \
                       "to factory default", "", ResetConfig)           \
        SYSTEM_COMMAND("testSD", "Test Write to SD card.",              \
                       "<lineWrites> <periodicFlush> <quietMode>",      \
                       TestSD)                                          \
        SYSTEM_COMMAND("startTerminal", "Starts a debugging terminal "  \
                       "session on the specified port.",                \
                       "<port> <baud> [echo 1|0]", StartTerminal)       \
        SYSTEM_COMMAND("setLogLevel", "Sets the log level", "<level>",  \
                       SetLogLevel)                                     \
        SYSTEM_COMMAND("logGpsData", "Enables logging of raw GPS data " \
                       "from the GPS Mouse", "<1|0>", LogGpsData)       \
        SYSTEM_COMMAND("viewLog", "Prints out logging messages to the " \
                       "terminal as they happen", "", ViewLog)          \
        SYSTEM_COMMAND("setSerialLog", "Enables/disables logging of  "  \
                       "serial device for debug purposes",              \
                       "<port> <0|1>", SetSerialLog)                    \
        SYSTEM_COMMAND("flashConfig", "Flashes the NVRAM with the "     \
                       "current configuration of the LoggerConfig",     \
                       "", FlashConfig)                                 \


void ResetConfig(struct Serial *serial, unsigned int argc, char **argv);
void TestSD(struct Serial *serial, unsigned int argc, char **argv);

void StartTerminal(struct Serial *serial, unsigned int argc, char **argv);
void ViewLog(struct Serial *serial, unsigned int argc, char **argv);
void SetLogLevel(struct Serial *serial, unsigned int argc, char **argv);
void LogGpsData(struct Serial *serial, unsigned int argc, char **argv);
void SetSerialLog(struct Serial *serial, unsigned int argc, char **argv);
void FlashConfig(struct Serial *serial, unsigned int argc, char **argv);

CPP_GUARD_END

#endif /* LOGGERCOMMANDS_H_ */
