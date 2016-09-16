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

#include "FreeRTOS.h"
#include "GPIO.h"
#include "capabilities.h"
#include "lauxlib.h"
#include "led.h"
#include "lua.h"
#include "luaBaseBinding.h"
#include "luaLoggerBinding.h"
#include "luaScript.h"
#include "luaTask.h"
#include "lualib.h"
#include "mem_mang.h"
#include "panic.h"
#include "portable.h"
#include "printk.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"
#include "taskUtil.h"
#include "virtual_channel.h"
#include "watchdog.h"
#include <math.h>
#include <stdbool.h>
#include <string.h>

/* Keep Stack value high as the parser can get very stack hungry.  Issue #411 */
#define LUA_BYPASS_DELAY_SEC		5
#define LUA_CONSECUTIVE_FAILURES_LIMIT	3
#define LUA_DEFAULT_ONTICK_HZ		1
#define LUA_ERR_NONE			0
#define LUA_ERR_BUG			-1
#define LUA_NO_PERIODIC_FUNCTION	-2
#define LUA_ERR_SCRIPT_LOAD_FAILED	-3
#define LUA_FLASH_DELAY_MS		250
#define LUA_LOCK_WAIT_MS		1000
#define LUA_MAXIMUM_ONTICK_HZ		1000
#define LUA_PERIODIC_FUNCTION 		"onTick"
#define LUA_STACK_SIZE 			2048
#define _LOG_PFX			"[lua] "

struct lua_run_state {
        lua_State *lua_state;
        bool script_loaded;
};

enum run_status {
	LUA_CMD_LOCK_ERROR = -2,
	LUA_CMD_GENERIC_ERROR = -1,
	LUA_CMD_FAILURE = 0,
	LUA_CMD_SUCCESS = 1,
};

static struct {
        int priority;
        xSemaphoreHandle lock;
	xSemaphoreHandle lua_signal;
        xTaskHandle task_handle;
        lua_State *lua_runtime;
        size_t callback_interval;
        size_t lua_mem_size;
	struct {
		const char* cmd; /* Command to execute */
		enum run_status status;
		xSemaphoreHandle cmd_signal;
		xSemaphoreHandle cmd_mutex;
	} interactive;
} state;

static void* myAlloc(void *ud, void *ptr, size_t osize, size_t nsize)
{
        const int delta = nsize - osize;
        const size_t new_lua_mem_size = state.lua_mem_size + delta;

        if (nsize == 0) {
                pr_trace_int_msg(_LOG_PFX "RAM Freed: ", abs(delta));
                portFree(ptr);
                state.lua_mem_size = new_lua_mem_size;
                return NULL;
        }

        if (LUA_MEM_MAX && LUA_MEM_MAX < new_lua_mem_size) {
                pr_warning(_LOG_PFX "Memory ceiling hit: ");
                pr_warning_int(new_lua_mem_size);
                pr_warning_int_msg(" > ", LUA_MEM_MAX);
                return NULL;
        }

        void *nptr = portRealloc(ptr, nsize);
        if (nptr == NULL) {
                pr_trace(_LOG_PFX "Realloc failed: ");
                pr_trace_int(state.lua_mem_size);
                pr_trace_int_msg(" -> ", new_lua_mem_size);
                return NULL;
        }

        const char *msg = delta < 0 ? _LOG_PFX "RAM released: " :
                "RAM allocated: ";
        pr_trace_int_msg(msg, abs(delta));
        state.lua_mem_size = new_lua_mem_size;

        return nptr;
}

static bool get_lock_wait(size_t time)
{
        return pdTRUE == xSemaphoreTake(state.lock, time);
}

static void get_lock(void)
{
        get_lock_wait(portMAX_DELAY);
}

static void release_lock(void)
{
        xSemaphoreGive(state.lock);
}

static bool load_script(lua_State *ls)
{
        const char *script = getScript();
        const size_t len = strlen(script);

        pr_info_int_msg(_LOG_PFX "Loading script. Length: ", len);

        if (0 != luaL_dostring(ls, script)) {
                pr_error(_LOG_PFX "Startup script error: (");
                pr_error(lua_tostring(ls, -1));
                pr_error(")\r\n");

                lua_pop(ls, 1);
                return false;
        }

        pr_info(_LOG_PFX "Successfully loaded script.\r\n");
        return true;
}

static int lua_invocation(struct lua_run_state *rs)
{
        int status = LUA_ERR_BUG;
        const char function[] = LUA_PERIODIC_FUNCTION;
        get_lock();

        /* First load our script if needed */
        if (!rs->script_loaded) {
                /*
                 * Reset virt channels before we load the script as these are
                 * often defined as part of the script load process.
                 */
                reset_virtual_channels();

                if (!load_script(rs->lua_state)) {
                        status = LUA_ERR_SCRIPT_LOAD_FAILED;
                        goto done;
                }

                rs->script_loaded = true;
        }

        /* Now run the callback */
        lua_getglobal(rs->lua_state, function);
        if (lua_isnil(rs->lua_state, -1)) {
                /* No longer a failure per Issue #707 */
                lua_pop(rs->lua_state, 1);
                status = LUA_NO_PERIODIC_FUNCTION;
                goto done;
        }

        status = lua_pcall(rs->lua_state, 0, 0, 0);
        if (0 != status) {
                pr_error_str_msg(_LOG_PFX "Script error: ",
                                 lua_tostring(rs->lua_state, -1));
                lua_pop(rs->lua_state, 1);
        }

done:
	lua_settop(rs->lua_state, 0);
        release_lock();
        return status;
}

static const char* get_failure_msg(const int cause)
{
        switch (cause) {
        case LUA_YIELD:
                return "Yielded. ASL BUG?";
        case LUA_ERRRUN:
                return "Runtime Error";
        case LUA_ERRSYNTAX:
                return "Syntax Error";
        case LUA_ERRMEM:
                return "Out of Memory";
        case LUA_ERRERR:
                return "Unknown Lua Error";
        case LUA_ERR_BUG:
                return "ASL BUG";
        case LUA_ERR_SCRIPT_LOAD_FAILED:
                return "Failed to load script";
        default:
                return "Unknown";
        }
}

static void lua_failure_state(const int cause)
{
        const char *msg = get_failure_msg(cause);
        pr_warning_str_msg(_LOG_PFX "Failure: ", msg);

        while(true) {
                delayMs(LUA_FLASH_DELAY_MS);
                led_toggle(LED_ERROR);

		/* Give to unblock any waiting interactive commands */
		xSemaphoreGive(state.interactive.cmd_signal);
        }
}

static bool lua_interactive(struct lua_run_state *rs)
{
	/* Check if there is a command */
	if (NULL == state.interactive.cmd)
		return false;

	/* If here then there is a command.  Lets get it rolling */
	lua_State *ls = rs->lua_state;
        const bool got_lock = get_lock_wait(msToTicks(LUA_LOCK_WAIT_MS));
        if (!got_lock) {
                state.interactive.status = LUA_CMD_LOCK_ERROR;
		goto done;
        }

	state.interactive.status = luaL_dostring(ls, state.interactive.cmd) ?
		LUA_CMD_FAILURE : LUA_CMD_SUCCESS;

done:
	state.interactive.cmd = NULL;

	/*
	 * Give the command_signal to unblock the command process and then
	 * re-take the lua_signal to block until the command is done printing
	 * the message we have included.
	 */
	xSemaphoreGive(state.interactive.cmd_signal);
	xSemaphoreTake(state.lua_signal, portMAX_DELAY);

	 /* Clear the stack before we return */
	lua_settop(ls, 0);
	release_lock();
	return true;
}

static void lua_task(void *params)
{
        struct lua_run_state rs = {
                .lua_state = params,
                .script_loaded = false,
        };

        int consecutive_failures = 0;
	portTickType wake_tick = 0;
        for(;;) {
		/* Handle the interactive commands if there are any */
		if (lua_interactive(&rs))
			continue;

		portTickType curr_tick = xTaskGetTickCount();
		if (curr_tick < wake_tick)
			/* If signal is given, restart loop, else normal op */
			if (xSemaphoreTake(state.lua_signal, wake_tick - curr_tick))
				continue;

                wake_tick = xTaskGetTickCount() + state.callback_interval;
                const int rc = lua_invocation(&rs);

                /* If its a known unrecoverable, fail fast */
                switch (rc) {
		case LUA_ERR_NONE:
		case LUA_NO_PERIODIC_FUNCTION:
			consecutive_failures = 0;
			/*
			 * We no longer stop the task because we always want to
			 * have a Lua interactive session available.
			 */
			continue;
                case LUA_ERR_BUG:
                case LUA_ERR_SCRIPT_LOAD_FAILED:
                        lua_failure_state(rc);
		default:
			/* If here then there was a failure. */
			++consecutive_failures;

                }

                /* If here, perhaps we can recover.  */
                get_lock();
                lua_gc(rs.lua_state, LUA_GCCOLLECT, 0);
                release_lock();
                if (consecutive_failures < LUA_CONSECUTIVE_FAILURES_LIMIT)
                        continue;

                /* If here, then we failed to recover */
                lua_failure_state(rc);
        }
}

static bool is_init(const bool quiet)
{
        const bool init = NULL != state.lock;

        if (!init && !quiet)
                pr_warning(_LOG_PFX "Not initialized\r\n");

        return init;
}

static bool is_runtime_active()
{
        return NULL != state.lua_runtime;
}

static lua_State* setup_lua_state()
{
        pr_info(_LOG_PFX "Initializing Lua state\r\n");

        lua_State *ls = lua_newstate(myAlloc, NULL);
        if (!ls) {
                pr_error(_LOG_PFX "LUA runtime alloc failure.\r\n");
                return NULL;
        }

        /* Open optional libraries */
        luaopen_base(ls);
        registerBaseLuaFunctions(ls);
        registerLuaLoggerBindings(ls);

        if (LUA_REGISTER_EXTERNAL_LIBS) {
                luaopen_table(ls);
                luaopen_string(ls);
                luaopen_math(ls);
                luaopen_bit(ls);
        }

        /* Now do an agressive GC cycle to cleanup as much as possible */
        lua_gc(ls, LUA_GCCOLLECT, 0);
        pr_info(_LOG_PFX "memory usage: ");
        pr_info_int(lua_gc(ls, LUA_GCCOUNT, 0));
        pr_info("KB\r\n");

        /* Set garbage collection settings */
        lua_gc(ls, LUA_GCSETPAUSE, LUA_GC_PAUSE_PCT);
        lua_gc(ls, LUA_GCSETSTEPMUL, LUA_GC_STEP_MULT_PCT);

        return ls;
}

void lua_task_run_interactive_cmd(struct Serial *serial, const char* cmd)
{
        if (!is_init(false) || !is_runtime_active()) {
                serial_write_s(serial, "Internal Error: Lua not active.");
                put_crlf(serial);
                return;
        }

	/* Take the mutex to prevent two commands at once */
	xSemaphoreTake(state.interactive.cmd_mutex, portMAX_DELAY);

	/*
	 * To preserve stack space we let the luaTask run the command.  This
	 * ensures that we can keep stacks from other processes to a minimum
	 * at the expense of some complexity (the locking here).
	 *
	 * Notice we give the lua_signal twice here. The first time will
	 * unblock the luaTask so that it may perform our command.  The
	 * second call completes the interactive command.  This is done this
	 * way so we may pop all elements/error messages as needed. Therefore
	 * it is safe to manipulate the Lua state since the lua_interactive
	 * method will keep the Lua lock until we give the lua_signal the
	 * second time.
	 *
	 * Once this task has completed reading the results of the command
	 * and has given the lua_signa, the lua_interactive task will
	 * re-activate and will clean up and continue as normal.
	 */
	state.interactive.status = LUA_CMD_GENERIC_ERROR;
	state.interactive.cmd = cmd;
	xSemaphoreGive(state.lua_signal);
	xSemaphoreTake(state.interactive.cmd_signal, portMAX_DELAY);

	/*
	 * Once we get here the command has completed since we got the
	 * cmd_signal. Print the values on the stack out for the interactive
	 * user.
	 */
	switch (state.interactive.status) {
	case LUA_CMD_GENERIC_ERROR:
		serial_write_s(serial, "Internal Error.  Sorry :(");
		put_crlf(serial);
		break;

	case LUA_CMD_LOCK_ERROR:
		serial_write_s(serial, "Internal Error: Failed to acquire "
			"Lua lock. Check script.");
		put_crlf(serial);
		break;
	case LUA_CMD_FAILURE:
		serial_write_s(serial, "Error: ");
	case LUA_CMD_SUCCESS:
	{
		lua_State *ls = state.lua_runtime;
		const int items = lua_gettop(ls);
		for (int i = 1; i <= items; ++i) {
			if (1 < i)
				serial_write_s(serial, ", ");

			serial_write_s(serial, lua_tostring(ls, i));
		}

		if (items)
			put_crlf(serial);
        }
	}

	/* Completes the task */
	xSemaphoreGive(state.lua_signal);
	xSemaphoreGive(state.interactive.cmd_mutex);
}

struct lua_runtime_info lua_task_get_runtime_info()
{
        struct lua_runtime_info ri;
        memset(&ri, 0, sizeof(ri));

        if (!is_init(false) || !is_runtime_active())
                return ri;

        lua_State *ls = state.lua_runtime;
        ri.top_index = lua_gettop(ls);
        ri.mem_usage_kb = lua_gc(ls, LUA_GCCOUNT, 0);

        return ri;
}

size_t lua_task_get_mem_size()
{
        return state.lua_mem_size;
}

size_t lua_task_set_callback_freq(const size_t freq)
{
        if (LUA_MAXIMUM_ONTICK_HZ < freq || 0 == freq)
                return 0;

        return state.callback_interval = msToTicks(TICK_RATE_HZ / freq);
}

size_t lua_task_get_callback_freq()
{
        return 1000 / ticksToMs(state.callback_interval);
}

bool lua_task_stop()
{
        if (!is_init(false) || !is_runtime_active())
                return false;

        const size_t ticks = msToTicks(LUA_LOCK_WAIT_MS);
        const bool got_lock = get_lock_wait(ticks);

        /* Its possible to have a runtime but no task */
        if (state.task_handle) {
                if (!got_lock) {
                        pr_warning(_LOG_PFX "Killing Lua task because "
                                   "it is unresponsive\r\n");
                } else {
                        pr_info(_LOG_PFX "Gracefully stopping Lua Task\r\n");
                }

                vTaskDelete(state.task_handle);
                state.task_handle = NULL;
        }

        pr_info(_LOG_PFX "Destroying Lua State\r\n");
        lua_close(state.lua_runtime);
        state.lua_runtime = NULL;

        led_disable(LED_ERROR);

        /*
         * If we didn't get the semaphore, then the task we just
         * killed was the likely owner of it (better be).  Since
         * its dead, then its safe to give the semaphore back since
         * the task can't do it anyways.  And if we did get the
         * semaphore, then we need to give it back.
         */
        release_lock();
        return true;
}

bool lua_task_start()
{
        if (!is_init(false) || is_runtime_active())
                return false;

        get_lock();
        bool ok = false;

        /* Initialize the Lua runtime here */
        state.lua_runtime = setup_lua_state();
        if (!is_runtime_active()) {
                pr_warning(_LOG_PFX "Failed to create lua runtime\r\n");
                goto done;
        }

        pr_info(_LOG_PFX "Starting Lua Task\r\n");

        /* Make all task names 16 chars including NULL char */
        static const signed portCHAR task_name[] = "Lua Exec Task  ";
        ok = pdPASS == xTaskCreate(lua_task, task_name, LUA_STACK_SIZE,
                                   state.lua_runtime, state.priority,
                                   &state.task_handle);

        if (!ok) {
                pr_warning(_LOG_PFX "Failed to start Lua Task\r\n");
                state.task_handle = NULL; /* Just to be sure */
        }
done:
        release_lock();
        return ok;
}

/**
 * Checks if we need to bypass the Lua script at the user's request.
 * @return True if asked to do so, false otherwise.
 */
static bool user_bypass_check()
{
        /* Only check if reset cause was a watchdog event */
        if (!watchdog_is_watchdog_reset())
                return false;

        pr_info(_LOG_PFX "Runtime bypass check triggered...\r\n");
        bool bypass = false;
        led_disable(LED_ERROR);

	const int flash_count =
                LUA_BYPASS_DELAY_SEC * 1000 / LUA_FLASH_DELAY_MS;
        for (size_t i = 0; i < flash_count; ++i) {
                delayMs(LUA_FLASH_DELAY_MS);
                led_toggle(LED_ERROR);
                if (GPIO_is_button_pressed()) {
                        bypass = true;
                        break;
                }
        }

        led_disable(LED_ERROR);
        pr_info_bool_msg(_LOG_PFX "Bypass requested: ", bypass);

        return bypass;
}

/**
 * Called when we first setup the system.  Should only ever be called
 * once.
 */
bool lua_task_init(const int priority)
{
        /* Only init once */
        if (is_init(true)) {
                pr_error(_LOG_PFX "Already initialized\r\n");
                return false;
        }

        /*
         * Always set initial state here.  Needed because we
         * may kill this task later.  Setting the lock is what we use
         * to determine that we are initialized.
         */
        vSemaphoreCreateBinary(state.lock);
        state.priority = priority;

        if (!is_init(false)) {
                pr_error(_LOG_PFX "Failed to alloc lock semaphore\r\n");
                return false;
        }

        if (user_bypass_check()) {
                pr_info(_LOG_PFX "User bypassed Lua runtime start.\r\n");
                return false;
        }

	/* Now init the rest of Lua since we are running it */
	state.lua_signal = xSemaphoreCreateBinary();
	state.interactive.cmd_signal = xSemaphoreCreateBinary();
	state.interactive.cmd_mutex = xSemaphoreCreateMutex();
	if (!state.lua_signal || !state.interactive.cmd_signal ||
	    !state.interactive.cmd_mutex) {
		pr_error(_LOG_PFX "Failed to alloc remaining semaphores\r\n");
		return false;
	}

	/*
	 * Give the command mutex so we may issue interactive commands to
	 * Lua Runtime.
	 */
	xSemaphoreGive(state.interactive.cmd_mutex);

        /* XXX: This method name sucks.  It resests script if needed */
        initialize_script();

        lua_task_set_callback_freq(LUA_DEFAULT_ONTICK_HZ);
        return lua_task_start();
}
