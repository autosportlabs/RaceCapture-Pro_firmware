/*
 * luaCommands.c
 *
 *  Created on: Jul 24, 2011
 *      Author: brent
 */
#include "mod_string.h"
#include "luaCommands.h"
#include "modp_numtoa.h"
#include "modp_atonum.h"
#include "luaScript.h"
#include "lua.h"
#include "luaTask.h"
#include "memory.h"
#include "FreeRTOS.h"
#include "printk.h"
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#define LINE_BUFFER_SIZE  256

static int g_interactive_mode = 0;

void ExecLuaInterpreter(Serial *serial, unsigned int argc, char **argv)
{

    g_interactive_mode = 1;
    serial->put_s("Entering Lua Interpreter. enter 'exit' to leave");
    put_crlf(serial);

    lua_State *L = getLua();

    cmd_context *cmdContext = get_command_context();
    char * luaLine = cmdContext->lineBuffer;

    int result;
    while(1) {
        serial->put_s("> ");
        interactive_read_line(serial, luaLine, cmdContext->lineBufferSize);
        if (strcmp(luaLine,"exit") == 0) break;
        lockLua();
        lua_gc(L,LUA_GCCOLLECT,0);
        result = luaL_loadbuffer(L, luaLine, strlen(luaLine), "");
        if (0 != result) {
            serial->put_s("error: (");
            serial->put_s(lua_tostring(L,-1));
            serial->put_s(")");
            put_crlf(serial);
            lua_pop(L,1);
        } else {
            lua_pushvalue(L,-1);
            result = lua_pcall(L,0,0,0);
            if (0 != result) {
                serial->put_s("error: (");
                serial->put_s(lua_tostring(L,-1));
                serial->put_s(")");
                put_crlf(serial);
                lua_pop(L,1);
            }
            lua_pop(L,1);
        }
        unlockLua();
    }
    g_interactive_mode = 0;
}


void ReloadScript(Serial *serial, unsigned int argc, char **argv)
{
    setShouldReloadScript(1);
    put_commandOK(serial);
}

int in_interactive_mode()
{
    return g_interactive_mode;
}

