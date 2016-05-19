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

#ifndef LUASCRIPT_H_
#define LUASCRIPT_H_

#include "cpp_guard.h"
#include "capabilities.h"
#include "memory.h"

#include <stdint.h>

CPP_GUARD_BEGIN

#if LUA_SUPPORT

enum script_add_result {
        SCRIPT_ADD_RESULT_FAIL = 0,
        SCRIPT_ADD_RESULT_OK = 1,
};

enum script_add_mode {
        SCRIPT_ADD_MODE_IN_PROGRESS = 1,
        SCRIPT_ADD_MODE_COMPLETE = 2,
};

#define MAGIC_NUMBER_SCRIPT_INIT	0xBADDECAF
#define SCRIPT_PAGE_SIZE	256
#define MAX_SCRIPT_PAGES	(SCRIPT_MEMORY_LENGTH / SCRIPT_PAGE_SIZE)

typedef struct _ScriptConfig {
        uint32_t magicInit;
        char script[SCRIPT_MEMORY_LENGTH - 4];
} ScriptConfig;

void initialize_script();

int flash_default_script();

const char * getScript();

enum script_add_result flashScriptPage(unsigned int page, const char *data,
                                       enum script_add_mode mode);

void unescapeScript(char *data);

#define DEFAULT_SCRIPT "function onTick() end"

#endif /* LUA_SUPPORT */

CPP_GUARD_END

#endif /*LUASCRIPT_H_*/
