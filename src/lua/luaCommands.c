/*
 * Race Capture Firmware
 *
 * Copyright (C) 2015 Autosport Labs
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

#include "FreeRTOS.h"
#include "lauxlib.h"
#include "lua.h"
#include "luaCommands.h"
#include "luaScript.h"
#include "luaTask.h"
#include "lualib.h"
#include "macros.h"
#include "memory.h"
#include "modp_numtoa.h"
#include "printk.h"
#include <string.h>

static int g_interactive_mode = 0;

void ExecLuaInterpreter(struct Serial *serial, unsigned int argc, char **argv)
{
        serial_write_s(serial, "Entering Lua Interpreter. enter 'exit' "
                     "to leave");
        put_crlf(serial);

        static char luaLine[256];

        g_interactive_mode = 1;

        for(;;) {
                serial_write_s(serial, "> ");
                interactive_read_line(serial, luaLine, ARRAY_LEN(luaLine));

                if (0 == strcmp(luaLine, "exit"))
                        break;

                lua_task_run_interactive_cmd(serial, luaLine);
        }

        g_interactive_mode = 0;
}


void ReloadScript(struct Serial *serial, unsigned int argc, char **argv)
{
        lua_task_stop();
        lua_task_start();
        put_commandOK(serial);
}

int in_interactive_mode()
{
        return g_interactive_mode;
}
