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

#ifndef LUATASK_H_
#define LUATASK_H_

#include "cpp_guard.h"
#include "serial.h"
#include <stdbool.h>
#include <stddef.h>

CPP_GUARD_BEGIN

struct lua_runtime_info {
        int top_index;
        size_t mem_usage_kb;
};

void lua_task_run_interactive_cmd(struct Serial *serial, const char* cmd);

struct lua_runtime_info lua_task_get_runtime_info();

size_t lua_task_get_mem_size();

size_t lua_task_set_callback_freq(const size_t freq);

size_t lua_task_get_callback_freq();

bool lua_task_stop();

bool lua_task_start();

bool lua_task_init(const int priority);

CPP_GUARD_END

#endif /*LUATASK_H_*/
