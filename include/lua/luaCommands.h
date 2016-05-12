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

#ifndef LUACOMMANDS_H_
#define LUACOMMANDS_H_

#include "cpp_guard.h"
#include "command.h"
#include "serial.h"

CPP_GUARD_BEGIN

#if LUA_SUPPORT
#define LUA_COMMANDS                                                    \
        SYSTEM_COMMAND("lua", "Enter the lua script interpreter.", "",  \
                       ExecLuaInterpreter)                              \
        SYSTEM_COMMAND("reloadScript", "Reloads the current script.",   \
                       "", ReloadScript)
#else
#define LUA_COMMANDS
#endif /* LUA_SUPPORT */

struct Serial *getLuaSerialContext();
void ExecLuaInterpreter(struct Serial *serial, unsigned int argc, char **argv);
void ReloadScript(struct Serial *serial, unsigned int argc, char **argv);
void ShowMemInfo(struct Serial *serial, unsigned int argc, char **argv);

int in_interactive_mode();

CPP_GUARD_END

#endif /* LUACOMMANDS_H_ */
