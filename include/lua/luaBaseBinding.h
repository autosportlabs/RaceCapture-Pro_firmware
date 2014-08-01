/*
 * luaBaseBindings.h
 *
 *  Created on: May 10, 2011
 *      Author: brent
 */

#ifndef LUABASEBINDINGS_H_
#define LUABASEBINDINGS_H_
#include "lua.h"

int f_impl(lua_State *L);

void registerBaseLuaFunctions(lua_State *L);

// utility functions
int Lua_Print(lua_State *L);
int Lua_Println(lua_State *L);
int Lua_GetStackSize(lua_State *L);
int Lua_SetTickRate(lua_State *L);
int Lua_GetTickRate(lua_State *L);
int Lua_PrintLog(lua_State *L);
int Lua_PrintLogLn(lua_State *L);
int Lua_SetLogLevel(lua_State *L);
int Lua_GetLogLevel(lua_State *L);

#endif /* LUABASEBINDINGS_H_ */
