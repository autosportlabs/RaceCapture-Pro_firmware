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
#include "command.h"
#include "lauxlib.h"
#include "luaBaseBinding.h"
#include "luaCommands.h"
#include "luaScript.h"
#include "luaTask.h"
#include "lualib.h"
#include "lauxlib.h"
#include "memory.h"
#include "mod_string.h"
#include "printk.h"
#include "taskUtil.h"

int incorrect_arguments(lua_State *L)
{
        return luaL_error(L, "incorrect argument");
}

void registerBaseLuaFunctions(lua_State *L)
{
    lua_registerlight(L,"getStackSize", Lua_GetStackSize);
    lua_registerlight(L,"setTickRate", Lua_SetTickRate);
    lua_registerlight(L,"getTickRate", Lua_GetTickRate);
    lua_registerlight(L,"print", Lua_PrintLog);
    lua_registerlight(L,"println", Lua_PrintLogLn);
    lua_registerlight(L,"setLogLevel", Lua_SetLogLevel);
    lua_registerlight(L,"getLogLevel", Lua_GetLogLevel);
    lua_registerlight(L,"sleep", Lua_Sleep);
}

int Lua_Sleep(lua_State *L)
{
    if (lua_gettop(L) >= 1) {
        delayMs(lua_tointeger(L,1));
    }
    return 0;
}

int Lua_GetStackSize(lua_State *L)
{
    lua_pushinteger(L,lua_gettop(L));
    return 1;
}

int Lua_SetTickRate(lua_State *L)
{
        if (lua_gettop(L) != 1 || !lua_isnumber(L, 1))
                return incorrect_arguments(L);

        const size_t res = lua_task_set_callback_freq(lua_tointeger(L, 1));
        if (!res)
                return luaL_error(L, "Invalid frequency");

        lua_pushnumber(L, res);
        return 1;
}

int Lua_GetTickRate(lua_State *L)
{
    lua_pushinteger(L, lua_task_get_callback_freq());
    return 1;
}

static int printLog(lua_State *L, int addNewline)
{
        if (0 == lua_gettop(L))
                return 0;

        const char *msg = lua_tostring(L, 1);
        const int level = lua_gettop(L) >= 2 ? lua_tointeger(L, 2) : INFO;

        if (in_interactive_mode()) {
                struct Serial *serial = get_command_context()->serial;
                if (serial) {
                        serial_put_s(serial, lua_tostring(L,1));
                        if (addNewline) {
                                put_crlf(serial);
                        }
                }
        }

        printk(level, msg);

        if (addNewline) {
                printk(level, "\r\n");
        }

        return 0;
}

int Lua_PrintLogLn(lua_State *L)
{
    return printLog(L, 1);
}

int Lua_PrintLog(lua_State *L)
{
    return printLog(L, 0);
}

int Lua_SetLogLevel(lua_State *L)
{
    if (lua_gettop(L) >= 1) {
        int level = lua_tointeger(L, 1);
        set_log_level(level);
    }
    return 0;
}

int Lua_GetLogLevel(lua_State *L)
{
    lua_pushinteger(L, get_log_level());
    return 1;
}
