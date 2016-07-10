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

int lua_panic(lua_State *l);
void lua_validate_args_count(lua_State *l, const int min, const int max);
void lua_validate_arg_boolean(lua_State *l, const int idx);
void lua_validate_arg_number(lua_State *l, const int idx);
void lua_validate_arg_string(lua_State *l, const int idx);
void lua_validate_arg_table(lua_State *l, const int idx);
void lua_validate_arg_number_or_string(lua_State *l, const int idx);
void lua_validate_arg_boolean_flex(lua_State *l, const int idx);
bool lua_toboolean_flex(lua_State *l, const int idx);
void registerBaseLuaFunctions(lua_State *L);

CPP_GUARD_END

#endif /* LUABASEBINDINGS_H_ */
