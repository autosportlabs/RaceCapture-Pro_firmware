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

void registerBaseLuaFunctions(lua_State *L){
	//Utility
	lua_registerlight(L,"print",Lua_Print);
	lua_registerlight(L,"println", Lua_Println);
	lua_registerlight(L,"getStackSize", Lua_GetStackSize);
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
