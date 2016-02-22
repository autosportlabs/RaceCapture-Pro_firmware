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

#ifndef LUABASEBINDINGS_H_
#define LUABASEBINDINGS_H_

#include "cpp_guard.h"
#include "lua.h"

CPP_GUARD_BEGIN

int f_impl(lua_State *L);

void registerBaseLuaFunctions(lua_State *L);

// utility functions
int Lua_Print(lua_State *L);
int Lua_Println(lua_State *L);
int Lua_GetStackSize(lua_State *L);
int Lua_SetTickRate(lua_State *L);
int Lua_GetTickRate(lua_State *L);
int Lua_PrintLog(lua_State *L);
int Lua_PrintLogLn(lua_State *L);
int Lua_SetLogLevel(lua_State *L);
int Lua_GetLogLevel(lua_State *L);

/**
 * Returns an error to the Lua runtime with a descriptive string
 * IMPORTANT: This methods never returns.  The `lua_error` method.
 *            does a long jump internally.  The return statements here
 *            are really only here to make things look pretty and logical.
 */
int rcp_lua_error(lua_State *L, const char *msg);
int incorrect_arguments(lua_State *L);



CPP_GUARD_END

#endif /* LUABASEBINDINGS_H_ */
