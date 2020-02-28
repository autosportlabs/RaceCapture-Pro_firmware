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

#include "luaScript.h"
#include "luaTask.h"
#include "mem_mang.h"
#include "printk.h"
#include "str_util.h"
#include <string.h>

#ifndef RCP_TESTING
static const volatile ScriptConfig g_scriptConfig  __attribute__((section(".script\n\t#")));
#else
static ScriptConfig g_scriptConfig = {MAGIC_NUMBER_SCRIPT_INIT,
                                      DEFAULT_SCRIPT
                                     };
#endif

#define _LOG_PFX   "[luaScript] "

void initialize_script()
{
        if (g_scriptConfig.magicInit != MAGIC_NUMBER_SCRIPT_INIT) {
                flash_default_script();
        }
}

int flash_default_script()
{
        int result = -1;
        pr_info(_LOG_PFX "Flashing default script...");

        lua_task_stop();

        /* Define a minimal script, enough
         * to flash the lua script memory region */
        typedef struct {
                uint32_t script_init_magic_number;
                char script[sizeof(DEFAULT_SCRIPT) + 1];
        } minimal_script_t;

        minimal_script_t minimal_script = {MAGIC_NUMBER_SCRIPT_INIT, DEFAULT_SCRIPT};

        result = memory_flash_region((void *)&g_scriptConfig,
                                     (void *)&minimal_script,
                                     sizeof (minimal_script_t));

        pr_info(result == 0 ? "win\r\n" : "fail\r\n");
        return result;
}

const char * getScript()
{
        return (const char *)g_scriptConfig.script;
}

//unescapes a string in place
void unescapeScript(char *data)
{
        char *result = data;
        while (*data) {
                if (*data == '\\') {
                        switch(*(data + 1)) {
                        case '_':
                                *result = ' ';
                                break;
                        case 'n':
                                *result = '\n';
                                break;
                        case 'r':
                                *result = '\r';
                                break;
                        case '\\':
                                *result = '\\';
                                break;
                        case '"':
                                *result = '\"';
                                break;
                        case '\0': //this should *NOT* happen
                                *result = '\0';
                                return;
                                break;
                        default: // unknown escape char?
                                *result = ' ';
                                break;
                        }
                        result++;
                        data+=2;
                } else {
                        *result++ = *data++;
                }
        }
        *result='\0';
}

enum script_add_result flashScriptPage(const unsigned int page,
                                       const char *data,
                                       const enum script_add_mode mode)
{
        if (page >= MAX_SCRIPT_PAGES) {
                pr_error(_LOG_PFX "invalid script index\r\n");
                return SCRIPT_ADD_RESULT_FAIL;
        }

        switch (mode) {
        case SCRIPT_ADD_MODE_IN_PROGRESS:
        case SCRIPT_ADD_MODE_COMPLETE:
                /* Valid cases.  Carry on */
                break;
        default:
                pr_error_int_msg(_LOG_PFX "Unknown script_add_mode: ", mode);
                return SCRIPT_ADD_RESULT_FAIL;
        }

        static ScriptConfig *g_scriptBuffer;
        if (NULL == g_scriptBuffer) {
                lua_task_stop();

                pr_debug(_LOG_PFX "Allocating new script buffer\r\n");
                g_scriptBuffer =
                        (ScriptConfig *) portMalloc(sizeof(ScriptConfig));
                memcpy((void *)g_scriptBuffer, (void *)&g_scriptConfig,
                       sizeof(ScriptConfig));
        }

        if (NULL == g_scriptBuffer) {
                pr_error(_LOG_PFX "Failed to allocate memory for script "
                         "buffer.\r\n");
                return SCRIPT_ADD_RESULT_FAIL;
        }

        char *pageToAdd = g_scriptBuffer->script + page * SCRIPT_PAGE_SIZE;
        strncpy(pageToAdd, data, SCRIPT_PAGE_SIZE);

        if (SCRIPT_ADD_MODE_IN_PROGRESS == mode)
                return SCRIPT_ADD_RESULT_OK;

        pr_info(_LOG_PFX "Completed updating LUA. Flashing... ");
        const int rc = memory_flash_region((void*) &g_scriptConfig,
                                           (void*) g_scriptBuffer,
                                           sizeof(ScriptConfig));
        portFree(g_scriptBuffer);
        g_scriptBuffer = NULL;

        if (0 != rc) {
                pr_info_int_msg(_LOG_PFX "Failed with code ", rc);
                return SCRIPT_ADD_RESULT_FAIL;
        }

        pr_info("win!\r\n");
        lua_task_start();
        return SCRIPT_ADD_RESULT_OK;
}
