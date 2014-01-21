/*
 * luaBaseBinding.c
 *
 *  Created on: May 10, 2011
 *      Author: brent
 */
#include "mod_string.h"
#include "luaBaseBinding.h"
#include "memory.h"
#include "FreeRTOS.h"
#include "lauxlib.h"
#include "lualib.h"
#include "luaScript.h"
#include "luaCommands.h"
#include "command.h"
#include "taskUtil.h"
#include "luaTask.h"

void registerBaseLuaFunctions(lua_State *L){
	//Utility
	lua_registerlight(L,"print",Lua_Print);
	lua_registerlight(L,"println", Lua_Println);
	lua_registerlight(L,"getStackSize", Lua_GetStackSize);
	lua_registerlight(L,"setTickRate", Lua_SetTickRate);
	lua_registerlight(L,"getTickRate", Lua_GetTickRate);
}

int Lua_Println(lua_State *L){
	Serial *serial = get_command_context()->serial;
	if (lua_gettop(L) >= 1 && serial){
		serial->put_s(lua_tostring(L,1));
		put_crlf(serial);
	}
	return 0;
}

int Lua_Print(lua_State *L){
	Serial *serial = get_command_context()->serial;
	if (lua_gettop(L) >= 1 && serial){
		serial->put_s(lua_tostring(L,1));
	}
	return 0;
}

int Lua_GetStackSize(lua_State *L){
	lua_pushinteger(L,lua_gettop(L));
	return 1;
}

int Lua_SetTickRate(lua_State *L){
	if (lua_gettop(L) >= 1){
		int freq = lua_tointeger(L, 1);
		set_ontick_freq(freq);
	}
	return 0;
}

int Lua_GetTickRate(lua_State *L){
	lua_pushinteger(L, get_ontick_freq());
	return 1;
}

