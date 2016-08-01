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
#include <string.h>
#include "printk.h"
#include "taskUtil.h"
#include <string.h>

/**
 * Called whenever we hit a panic state.  Errors and prints a note
 * to get in contact with us.  I opted to use this here since it will
 * be easier to debug in the field instead of using the standard panic.
 */
int lua_panic(lua_State *l)
{
        return luaL_error(l, "BUG. Please inform AutosportLabs");
}

/**
 * Validates the number of arguments passed.  If either bound is exceeded,
 * then this method will throw a lua error and will long jump back into
 * the interpreter, effectively bypassing the remaining C code.
 */
void lua_validate_args_count(lua_State *l, const int min, const int max)
{
        const int args = lua_gettop(l);
        if (min <= args && args <= max)
                return;

        /* If here, then failure. */
        char buff[64];
        if (min == max) {
                sprintf(buff, "Incorrect number of arguments. Expected %d",
                        min);
        } else {
                sprintf(buff, "Incorrect number of arguments. Expected "
                        "between %d to %d", min, max);
        }

        luaL_error(l, buff);
}

/**
 * Validates that the argument at idx is of boolean type.  If not
 * then this method will throw a lua error and will long jump back into
 * the interpreter, effectively bypassing the remaining C code.
 */
void lua_validate_arg_boolean(lua_State *l, const int idx)
{
        if (lua_isboolean(l, idx))
                return;

        char buff[48];
        sprintf(buff, "Expected boolean argument at position %d", idx);
        luaL_error(l, buff);
}

/**
 * Validates that the argument at idx is of number (int/float) type.
 * This includes strings that are all digits (like "123.45"). If not
 * then this method will throw a lua error and will long jump back into
 * the interpreter, effectively bypassing the remaining C code.
 */
void lua_validate_arg_number(lua_State *l, const int idx)
{
        if (lua_isnumber(l, idx))
                return;

        char buff[48];
        sprintf(buff, "Expected number argument at position %d", idx);
        luaL_error(l, buff);
}

/**
 * Validates that the argument at idx is a string. If not
 * then this method will throw a lua error and will long jump back into
 * the interpreter, effectively bypassing the remaining C code.
 */
void lua_validate_arg_string(lua_State *l, const int idx)
{
        if (lua_isstring(l, idx))
                return;

        char buff[48];
        sprintf(buff, "Expected string argument at position %d", idx);
        luaL_error(l, buff);
}

/**
 * Validates that the argument at idx is a number or a string. If not
 * then this method will throw a lua error and will long jump back into
 * the interpreter, effectively bypassing the remaining C code.
 */
void lua_validate_arg_number_or_string(lua_State *l, const int idx)
{
        if (lua_isnumber(l, idx) || lua_isstring(l, idx))
                return;

        char buff[64];
        sprintf(buff, "Expected number or string argument at position %d",
                idx);
        luaL_error(l, buff);
}

/**
 * Validates that the argument at idx is a table. If not
 * then this method will throw a lua error and will long jump back into
 * the interpreter, effectively bypassing the remaining C code.
 */
void lua_validate_arg_table(lua_State *l, const int idx)
{
        if (lua_istable(l, idx))
                return;

        char buff[48];
        sprintf(buff, "Expected table argument at position %d", idx);
        luaL_error(l, buff);
}

/**
 * Validates that the argument at idx is of boolean type or int type.  The
 * intention here is that the value will be converted using lua_toboolean_flex.
 * This method is a form of a stop gap until we can implement strong typing
 * in our bindings to/from lua since we have used 0 and 1 in places where
 * we should have been using false/true respectively.
 */
void lua_validate_arg_boolean_flex(lua_State *l, const int idx)
{
        if (lua_isboolean(l, idx) || lua_isnumber(l, idx))
                return;

        char buff[48];
        sprintf(buff, "Expected boolean/int argument at position %d", idx);
        luaL_error(l, buff);
}

/**
 * Similar to the lua_toboolean method except this method will treat a value
 * of 0 as false instead of true.  This is for legacy purposese since we have
 * used 1 and 0 in place of true and false in our API respectively.
 * @return false if false, nil, 0 or string that represent 0; true otherwise.
 */
bool lua_toboolean_flex(lua_State *l, const int idx)
{
	if (lua_isnumber(l, idx))
		return 0 != lua_tonumber(l, idx);

	return lua_toboolean(l, idx);
}


static int lua_sleep(lua_State *L)
{
        lua_validate_args_count(L, 1, 1);

        delayMs(lua_tointeger(L, 1));
        return 0;
}

static int lua_get_stack_size(lua_State *L)
{
        lua_pushinteger(L, lua_gettop(L));
        return 1;
}

static int lua_set_tick_rate(lua_State *L)
{
        lua_validate_args_count(L, 1, 1);
        lua_validate_arg_number(L, 1);

        const size_t res = lua_task_set_callback_freq(lua_tointeger(L, 1));
        if (!res)
                return luaL_error(L, "Invalid frequency");

        lua_pushnumber(L, res);
        return 1;
}

static int lua_get_tick_rate(lua_State *L)
{
        lua_pushinteger(L, lua_task_get_callback_freq());
        return 1;
}

static int log_print(lua_State *L, bool addNewline)
{
        lua_validate_args_count(L, 0, 2);

        int level = INFO;
        const char *msg;

        switch(lua_gettop(L)) {
        default:
                /* Should never get here */
                return lua_panic(L);
        case 0:
                return 0;
        case 2:
                lua_validate_arg_number(L, 2);
                level = lua_tointeger(L, 2);
                /* Break intentionally missing here */
        case 1:
                lua_validate_arg_string(L, 1);
                msg = lua_tostring(L, 1);
        }

        if (in_interactive_mode()) {
                struct Serial *serial = get_command_context()->serial;
                if (serial) {
                        serial_write_s(serial, lua_tostring(L,1));
                        if (addNewline)
                                put_crlf(serial);
                }
        }

        printk(level, msg);
        if (addNewline)
                printk(level, "\r\n");

        return 0;
}

static int lua_log_println(lua_State *L)
{
        return log_print(L, true);
}

static int lua_log_print(lua_State *L)
{
        return log_print(L, false);
}

static int lua_log_set_level(lua_State *L)
{
        lua_validate_args_count(L, 1, 1);

        set_log_level(lua_tointeger(L, 1));
        return 0;
}

static int lua_log_get_level(lua_State *L)
{
        lua_pushinteger(L, get_log_level());
        return 1;
}

void registerBaseLuaFunctions(lua_State *L)
{
        lua_registerlight(L, "getStackSize", lua_get_stack_size);
        lua_registerlight(L, "setTickRate", lua_set_tick_rate);
        lua_registerlight(L, "getTickRate", lua_get_tick_rate);
        lua_registerlight(L, "print", lua_log_print);
        lua_registerlight(L, "println", lua_log_println);
        lua_registerlight(L, "setLogLevel", lua_log_set_level);
        lua_registerlight(L, "getLogLevel", lua_log_get_level);
        lua_registerlight(L, "sleep", lua_sleep);
}
