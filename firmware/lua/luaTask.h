#ifndef LUATASK_H_
#define LUATASK_H_
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"


void startLuaTask();

int doIt(lua_State *L);

void luaTask(void *params);

#endif /*LUATASK_H_*/
