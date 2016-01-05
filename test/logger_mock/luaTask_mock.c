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


#include "luaTask.h"

void lockLua(void)
{

}

void unlockLua(void)
{

}

void startLuaTask(int priority)
{

}

void luaTask(void *params)
{

}

void *myAlloc (void *ud, void *ptr, size_t osize,size_t nsize)
{
    return NULL;
}

void * getLua(void)
{
    return NULL;
}

unsigned int getLastPointer()
{
    return 0;
}

void setAllocDebug(int enableDebug)
{

}

int getAllocDebug()
{
    return 0;
}

int getShouldReloadScript(void)
{
    return 0;
}

void setShouldReloadScript(int reload)
{

}

void set_ontick_freq(size_t freq)
{

}
size_t get_ontick_freq()
{
    return 1;
}

void terminate_lua() {}
void initialize_lua() {}
void run_lua_interactive_cmd(Serial *serial, const char* cmd) {}
