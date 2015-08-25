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

#ifndef LUACOMMANDS_H_
#define LUACOMMANDS_H_

#include "command.h"
#include "serial.h"

#if LUA_SUPPORT > 0
#define LUA_COMMANDS                                                    \
        SYSTEM_COMMAND("lua", "Enter the lua script interpreter.", "",  \
                       ExecLuaInterpreter)                              \
        SYSTEM_COMMAND("reloadScript", "Reloads the current script.",   \
                       "", ReloadScript)
#else
#define LUA_COMMANDS
#endif

Serial *getLuaSerialContext();
void ExecLuaInterpreter(Serial *serial, unsigned int argc, char **argv);
void ReloadScript(Serial *serial, unsigned int argc, char **argv);
void ShowMemInfo(Serial *serial, unsigned int argc, char **argv);

int in_interactive_mode();
#endif /* LUACOMMANDS_H_ */
