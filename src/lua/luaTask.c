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

#include "luaTask.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "portable.h"
#include "luaScript.h"
#include "luaBaseBinding.h"
#include "luaLoggerBinding.h"
#include "mem_mang.h"
#include "taskUtil.h"
#include "printk.h"
#include "mod_string.h"
#include "watchdog.h"
#include "LED.h"
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#include <stdbool.h>

#define DEFAULT_ONTICK_HZ 1
#define MAX_ONTICK_HZ 30
#define LUA_STACK_SIZE 1000
#define LUA_PERIODIC_FUNCTION "onTick"

static enum {
        LUA_DISABLED = 0,
        LUA_ENABLED,
} lua_run_state;

static lua_State *g_lua;
static xSemaphoreHandle xLuaLock;
static unsigned int lastPointer;
static size_t onTickSleepInterval;

#ifdef ALLOC_DEBUG
static int g_allocDebug = 0;
#endif
void * myAlloc (void *ud, void *ptr, size_t osize,size_t nsize)
{

#ifdef ALLOC_DEBUG
    if (g_allocDebug) {
        SendString("myAlloc- ptr:");
        SendUint((unsigned int)ptr);
        SendString(" osize:");
        SendInt(osize);
        SendString(" nsize:");
        SendInt(nsize);
    }
#endif

    if (nsize == 0) {
        portFree(ptr);
#ifdef ALLOC_DEBUG
        if (g_allocDebug) {
            SendString(" (free)");
            SendCrlf();
        }
#endif
        return NULL;
    } else {
        void *newPtr;
        if (osize != nsize) {
            newPtr = portRealloc(ptr, nsize);
        } else {
            newPtr = ptr;
        }
#ifdef ALLOC_DEBUG
        if (g_allocDebug) {
            if (ptr != newPtr) {
                SendString(" newPtr:");
                SendUint((unsigned int)newPtr);
            } else {
                SendString(" (same)");
            }
            SendCrlf();
        }
#endif
        lastPointer = (unsigned int)newPtr;
        return newPtr;
    }
}

unsigned int getLastPointer()
{
    return lastPointer;
}

void setAllocDebug(int enableDebug)
{
#ifdef ALLOC_DEBUG
    g_allocDebug = enableDebug;
#endif

}

int getAllocDebug()
{
#ifdef ALLOC_DEBUG
    return g_allocDebug;
#else
    return 0;
#endif
}

static void lockLua(void)
{
    xSemaphoreTake(xLuaLock, portMAX_DELAY);
}

static void unlockLua(void)
{
    xSemaphoreGive(xLuaLock);
}

void set_ontick_freq(size_t freq)
{
    if (freq <= MAX_ONTICK_HZ) onTickSleepInterval = msToTicks(1000 / freq);
}

size_t get_ontick_freq()
{
    return 1000 / ticksToMs(onTickSleepInterval);
}

void* getLua()
{
    return g_lua;
}

static void _terminate_lua()
{
        lua_close(g_lua);
        g_lua = NULL;

        lua_run_state = LUA_DISABLED;
}

void terminate_lua()
{
        lockLua();

        pr_info("lua: Stopping...\r\n");

        if (LUA_DISABLED == lua_run_state)
                goto cleanup;

        _terminate_lua();

cleanup:
        unlockLua();
}

static bool _load_script(void)
{
        const char *script = getScript();
        const size_t len = strlen(script);

        pr_info("Loading lua script (len = ");
        pr_info_int(len);
        pr_info("): ");

        lua_gc(g_lua, LUA_GCCOLLECT,0);

        if (0 != luaL_dostring(g_lua, script)) {
                pr_info("ERROR!\r\n");

                pr_error("lua: startup script error: (");
                pr_error(lua_tostring(g_lua,-1));
                pr_error(")\r\n");

                lua_pop(g_lua,1);
                return false;
        }

        pr_info("SUCCESS!\r\n");
        /* Empty the stack? --> lua_settop (g_lua, 0); */

        lua_gc(g_lua, LUA_GCCOLLECT,0);
        pr_info("lua: memory usage: ");
        pr_info_int(lua_gc(g_lua, LUA_GCCOUNT,0));
        pr_info("Kb\r\n");

        return true;
}

void initialize_lua()
{
        lockLua();

        pr_info("lua: Starting...\r\n");

        if (LUA_ENABLED == lua_run_state)
                goto cleanup;

        g_lua = lua_newstate(myAlloc, NULL);
        if (!g_lua) {
                pr_error("LUA: Can't allocate memory for LUA state.\r\n");
                goto cleanup;
        }

        //open optional libraries
        luaopen_base(g_lua);
        luaopen_table(g_lua);
        luaopen_string(g_lua);
        luaopen_math(g_lua);
        registerBaseLuaFunctions(g_lua);
        registerLuaLoggerBindings(g_lua);

        if (!_load_script()) {
                _terminate_lua();
                goto cleanup;
        }

        /* If here, then init was successful.  Enable runtime */
        lua_run_state = LUA_ENABLED;

cleanup:
        unlockLua();
}

static void run_lua_method(const char *method)
{
        lockLua();

        lua_getglobal(g_lua, method);
        if (lua_isnil(g_lua, -1)) {
                pr_error_str_msg("Method not found: ", method);
                lua_pop(g_lua, 1);
                goto cleanup;
        }


        if (0 != lua_pcall(g_lua, 0, 0, 0)) {
                pr_error_str_msg("Lua script error: ", lua_tostring(g_lua, -1));
                lua_pop(g_lua, 1);
                goto cleanup;
        }

cleanup:
        unlockLua();
}

void run_lua_interactive_cmd(Serial *serial, const char* cmd)
{
        lockLua();

        if (!g_lua) {
                serial->put_s("error: LUA not initialized.");
                put_crlf(serial);
                goto cleanup;
        }

        lua_gc(g_lua, LUA_GCCOLLECT, 0);

        /*
         * We use a combination of loadstring + pcall instead of using
         * dostring because we want to ensure that, after the call has
         * completed, there is nothing left on the stack.  Since these
         * commands come from the interactive console, we must take care
         * to ensure that we don't leave anything on the stack at the end.
         * dostring calls pcall with MULTRET, which could leave items on
         * the stack.
         */
        int result = luaL_loadstring(g_lua, cmd) || lua_pcall(g_lua, 0, 0, 0);
        if (0 != result) {
                serial->put_s("error: (");
                serial->put_s(lua_tostring(g_lua, -1));
                serial->put_s(")");
                put_crlf(serial);
                lua_pop(g_lua, 1);
                goto cleanup;
        }

cleanup:
        unlockLua();
}

static bool is_watchdog_reset(void)
{
        if (!watchdog_is_watchdog_reset())
                return false;

        pr_error("lua: Crash detected! Bypassing Lua script as a "
                 "recovery mode\r\n");
        LED_enable(3);

        return true;
}

static void luaTask(void *params)
{
        set_ontick_freq(DEFAULT_ONTICK_HZ);
        initialize_script();

        /* If we are starting up after watchdog, don't init LUA */
        if (!is_watchdog_reset())
                initialize_lua();

        for(;;) {
                portTickType xLastWakeTime = xTaskGetTickCount();
                vTaskDelayUntil(&xLastWakeTime, onTickSleepInterval);

                if (LUA_ENABLED != lua_run_state)
                        continue;

                run_lua_method(LUA_PERIODIC_FUNCTION);
        }
}

void startLuaTask(int priority)
{
        vSemaphoreCreateBinary(xLuaLock);
        xTaskCreate(luaTask, (signed portCHAR *) "luaTask",
                    LUA_STACK_SIZE, NULL, priority, NULL);
}
