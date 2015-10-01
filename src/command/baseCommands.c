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

#include "FreeRTOS.h"
#include "baseCommands.h"
#include "cpu.h"
#include "loggerConfig.h"
#include "lua.h"
#include "luaScript.h"
#include "luaTask.h"
#include "mem_mang.h"
#include "memory.h"
#include "task.h"

extern unsigned int _CONFIG_HEAP_SIZE;

static void putHeader(const Serial *serial, const char *str)
{
    put_crlf(serial);
    serial->put_s("- - - ");
    serial->put_s(str);
    serial->put_s(" - - -");
    put_crlf(serial);
}

static void putDataRowHeader(const Serial *serial, const char *str)
{
    serial->put_s(str);
    serial->put_s(" : ");
}

void ShowStats(Serial *serial, unsigned int argc, char **argv)
{
    // Memory Info
    putHeader(serial, "Memory Info");

    putDataRowHeader(serial, "Total Memory");
    put_uint(serial, (unsigned int) &_CONFIG_HEAP_SIZE);
    put_crlf(serial);

    putDataRowHeader(serial, "Free Memory");
    put_uint(serial, portGetFreeHeapSize());
    put_crlf(serial);

    // LUA Info
    putHeader(serial, "Lua Info");

    //TODO: this was done for unit testing. fix when Lua runtime is part of unit testing framework
    lua_State *L = (lua_State *) getLua();
    lua_gc(L, LUA_GCCOLLECT, 0);

    putDataRowHeader(serial, "Lua Top");
    put_int(serial, lua_gettop(L));
    put_crlf(serial);

    putDataRowHeader(serial, "Lua Memory Usage (KB)");
    put_int(serial, lua_gc(L, LUA_GCCOUNT, 0));
    put_crlf(serial);

    // Misc Info
    putHeader(serial, "Misc");

    putDataRowHeader(serial, "Size of LoggerConfig");
    put_int(serial, sizeof(LoggerConfig));
    put_crlf(serial);

    putDataRowHeader(serial, "Size of ChannelSample");
    put_int(serial, sizeof(ChannelSample));
    put_crlf(serial);
}

void ShowTaskInfo(Serial *serial, unsigned int argc, char **argv)
{
    putHeader(serial, "Task Info");

    serial->put_s("Status\tPri\tStackHR\tTask#\tName");
    put_crlf(serial);

    /*
     * Memory info from vTaskList can be misleading.  See
     * http://www.freertos.org/uxTaskGetSystemState.html for
     * more detail about how it works and value meanings.
     */
    char *taskList = (char *) portMalloc(1024);
    if (NULL != taskList) {
        vTaskList(taskList);
        serial->put_s(taskList);
        portFree(taskList);
    } else {
        serial->put_s("Out of Memory!");
    }
    put_crlf(serial);

    put_crlf(serial);
    serial->put_s("[Note] StackHR: If StackHR < 0; then stack smash");
    put_crlf(serial);
}

void GetVersion(Serial *serial, unsigned int argc, char **argv)
{
    putHeader(serial, "Version Info");
    put_nameString(serial, "major", MAJOR_REV_STR);
    put_nameString(serial, "minor", MINOR_REV_STR);
    put_nameString(serial, "bugfix", BUGFIX_REV_STR);
    put_nameString(serial, "serial", cpu_get_serialnumber());
    put_crlf(serial);
}

void ResetSystem(Serial *serial, unsigned int argc, char **argv)
{
    cpu_reset(0);
}
