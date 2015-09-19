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
#include "memory.h"
#include "mod_string.h"
#include "modp_atonum.h"
#include "modp_numtoa.h"
#include "printk.h"

static int g_interactive_mode = 0;

void ExecLuaInterpreter(Serial *serial, unsigned int argc, char **argv)
{
        serial->put_s("Entering Lua Interpreter. enter 'exit' to leave");
        put_crlf(serial);

        cmd_context *cmdContext = get_command_context();
        char *luaLine = cmdContext->lineBuffer;

        g_interactive_mode = 1;

        for(;;) {
                serial->put_s("> ");
                interactive_read_line(serial, luaLine, cmdContext->lineBufferSize);

                if (0 == strcmp(luaLine, "exit"))
                        break;

                run_lua_interactive_cmd(serial, luaLine);
        }

        g_interactive_mode = 0;
}


void ReloadScript(Serial *serial, unsigned int argc, char **argv)
{
        terminate_lua();
        initialize_lua();
        put_commandOK(serial);
}

int in_interactive_mode()
{
        return g_interactive_mode;
}
