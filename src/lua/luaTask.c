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
#include "GPIO.h"
#include "LED.h"
#include "lauxlib.h"
#include "lua.h"
#include "luaBaseBinding.h"
#include "luaLoggerBinding.h"
#include "luaScript.h"
#include "luaTask.h"
#include "lualib.h"
#include "mem_mang.h"
#include "mod_string.h"
#include "portable.h"
#include "printk.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"
#include "taskUtil.h"
#include "watchdog.h"

#include <math.h>
#include <stdbool.h>

#define DEFAULT_ONTICK_HZ 1
#define MAX_ONTICK_HZ 30
#define LUA_STACK_SIZE 1000
#define LUA_PERIODIC_FUNCTION "onTick"

#define LUA_BYPASS_FLASH_DELAY 250
#define LUA_BYPASS_DELAY_SEC 5
#define LUA_BYPASS_FLASH_COUNT (1000 / LUA_BYPASS_FLASH_DELAY) * LUA_BYPASS_DELAY_SEC

static enum {
        LUA_DISABLED = 0,
        LUA_ENABLED,
} lua_run_state;

static lua_State *g_lua;
static xSemaphoreHandle xLuaLock;
static size_t onTickSleepInterval;
static size_t lua_mem_size;

static void* myAlloc(void *ud, void *ptr, size_t osize, size_t nsize)
{
        const int delta = nsize - osize;
        const size_t new_lua_mem_size = lua_mem_size + delta;

        if (nsize == 0) {
                pr_debug_int_msg("[lua] RAM Freed: ", abs(delta));
                portFree(ptr);
                lua_mem_size = new_lua_mem_size;
                return NULL;
        }

        if (LUA_MEM_MAX && LUA_MEM_MAX < new_lua_mem_size) {
                pr_info("[lua] Memory ceiling hit: ");
                pr_info_int(new_lua_mem_size);
                pr_info_int_msg(" > ", LUA_MEM_MAX);
                return NULL;
        }

        void *nptr = portRealloc(ptr, nsize);
        if (nptr == NULL) {
                pr_debug("[lua] Realloc failed: ");
                pr_debug_int(lua_mem_size);
                pr_debug_int_msg(" -> ", new_lua_mem_size);
                return NULL;
        }

        const char *msg = delta < 0 ? "[lua] RAM released: " :
                "[lua] RAM allocated: ";
        pr_trace_int_msg(msg, abs(delta));
        lua_mem_size = new_lua_mem_size;

        return nptr;
}

size_t get_lua_mem_size()
{
        return lua_mem_size;
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

        pr_info("lua: Loading lua script (len = ");
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

        return true;
}

static bool user_bypass_requested(void)
{
        pr_info("lua: Checking for Lua runtime bypass request\r\n");
        bool bypass = false;
        LED_disable(LED_ERROR);
        for (size_t i = 0; i < LUA_BYPASS_FLASH_COUNT; i++) {
            LED_toggle(LED_ERROR);
            if (GPIO_is_button_pressed()) {
                bypass = true;
                break;
            }
            delayMs(LUA_BYPASS_FLASH_DELAY);
        }
        LED_disable(LED_ERROR);
        return bypass;
}

static int lua_gc_config(const int what, const int data)
{
        return 0 == data ? 0 : lua_gc(g_lua, what, data);
}

void initialize_lua()
{
        lockLua();

        pr_info("lua: Initializing...\r\n");

        if (LUA_ENABLED == lua_run_state)
                goto cleanup;

        g_lua = lua_newstate(myAlloc, NULL);
        if (!g_lua) {
                pr_error("lua: Can't allocate memory for LUA state.\r\n");
                goto cleanup;
        }

        //open optional libraries
        luaopen_base(g_lua);
        registerBaseLuaFunctions(g_lua);
        registerLuaLoggerBindings(g_lua);

        if (LUA_REGISTER_EXTERNAL_LIBS) {
                luaopen_table(g_lua);
                luaopen_string(g_lua);
                luaopen_math(g_lua);
        }

        if (!_load_script()) {
                _terminate_lua();
                goto cleanup;
        }

        /* Now do an agressive GC cycle to cleanup as much as possible */
        lua_gc(g_lua, LUA_GCCOLLECT, 0);
        pr_info("lua: memory usage: ");
        pr_info_int(lua_gc(g_lua, LUA_GCCOUNT, 0));
        pr_info("KB\r\n");

        /* Set garbage collection settings */
        lua_gc_config(LUA_GCSETPAUSE, LUA_GC_PAUSE_PCT);
        lua_gc_config(LUA_GCSETSTEPMUL, LUA_GC_STEP_MULT_PCT);


        /* If here, then init was successful.  Enable runtime */
        lua_run_state = LUA_ENABLED;

cleanup:
        unlockLua();
}

static int run_lua_method(const char *method)
{
        int status = -1;

        lockLua();

        lua_getglobal(g_lua, method);
        if (lua_isnil(g_lua, -1)) {
                pr_error_str_msg("lua: Method not found: ", method);
                lua_pop(g_lua, 1);
                goto cleanup;
        }

        status = lua_pcall(g_lua, 0, 0, 0);
        if (0 != status) {
                pr_error_str_msg("lua: Script error: ", lua_tostring(g_lua, -1));
                lua_pop(g_lua, 1);
                goto cleanup;
        }

cleanup:
        unlockLua();
        return status;
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


static void luaTask(void *params)
{
        set_ontick_freq(DEFAULT_ONTICK_HZ);
        initialize_script();

        const bool should_bypass_lua = (watchdog_is_watchdog_reset() && user_bypass_requested());
        if (should_bypass_lua) {
                pr_error("lua: Bypassing Lua Runtime\r\n");
                LED_enable(LED_ERROR);
        } else {
                initialize_lua();
        }

        int fales = 0;
        for(;;) {
                portTickType xLastWakeTime = xTaskGetTickCount();
                vTaskDelayUntil(&xLastWakeTime, onTickSleepInterval);

                if (LUA_ENABLED != lua_run_state)
                        continue;

                const int rc = run_lua_method(LUA_PERIODIC_FUNCTION);
                if (LUA_ERRMEM != rc) {
                        fales = 0;
                        continue;
                }

                /* If here then LUA failed to alloc RAM. */
                if (++fales < 3)
                        continue;

                pr_warning("[lua] LUA Memory Failure.  Resetting LUA\r\n");
                terminate_lua();
                initialize_lua();
                fales = 0;
        }
}

void startLuaTask(int priority)
{
        vSemaphoreCreateBinary(xLuaLock);
        lua_run_state = LUA_DISABLED;

        xTaskCreate(luaTask, (signed portCHAR *) "luaTask",
                    LUA_STACK_SIZE, NULL, priority, NULL);
}
