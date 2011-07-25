/*
 * loggerCommands.c
 *
 *  Created on: Jul 24, 2011
 *      Author: brent
 */
#include <string.h>
#include "loggerCommands.h"
#include "usb_comm.h"
#include "modp_numtoa.h"
#include "modp_atonum.h"
#include "base64.h"
#include "luaScript.h"
#include "lua.h"
#include "luaTask.h"
#include "memory.h"

static struct cmd_t execLuaCmd = {"lua",
		"Executes the specified Lua script fragment. Usage: lua <script>",
		ExecLuaCommand,
		NULL};

static struct cmd_t writeScriptPageCmd = {"writeScriptPage",
		"Writes the base64 encoded script to the specified 256 byte page. Usage: writeScriptPage <page> <base64encodedScript>",
		WriteScriptPage,
		NULL};

static struct cmd_t readScriptPageCmd = {"readScriptPage",
		"Returns the specified script page in base64 encoded format. Usage: readScriptPage <page>",
		ReadScriptPage,
		NULL};

void InitLoggerCommands(){
	AppendCommand(&execLuaCmd);
	AppendCommand(&writeScriptPageCmd);
	AppendCommand(&readScriptPageCmd);
}

void ExecLuaCommand(unsigned char argc, char **argv){

	if (argc < 2){
		SendCommandParamError("No Lua Script Specified");
		return;
	}

	lockLua();
	lua_State *L = getLua();

	int result = luaL_loadstring(L,argv[1]);
	if (0 != result){
		SendString("result=\"error:(");
		SendString(lua_tostring(L,-1));
		lua_pop(L,1);
		SendString("\");");
		SendCrlf();
	}else{
		lua_pushvalue(L,-1);
		lua_call(L,0,0);
		lua_pop(L,1);
		SendString("result=\"ok\";");
		SendCrlf();
	}
	unlockLua();

}

void WriteScriptPage(unsigned char argc, char **argv){
	if (argc < 3){
		SendCommandParamError("Invalid Parameters");
		return;
	}

	unsigned int page = modp_atoi(argv[1]);
	char * scriptPage = argv[2];

	if (scriptPage != NULL){
		vPortEnterCritical();
		char *decodedScript = base64decode(scriptPage, strlen(scriptPage));
		int result = flashScriptPage(page,decodedScript);
		vPortFree(decodedScript);
	   	vPortExitCritical();
		if (result == 0){
			SendCommandOK();
		}
		else{
			SendCommandError(result);
		}
		SendCrlf();
	}
}

void ReadScriptPage(unsigned char argc, char **argv){

	if (argc < 2){
		SendCommandParamError("Invalid Parameters");
		return;
	}

	int page = modp_atoi(argv[0]);

	if (page >=0 && page < SCRIPT_PAGES){

		const char * script = getScript();
		//forward to the requested page
		script += (MEMORY_PAGE_SIZE * page);
		//check for truncated page
		size_t scriptLen = strlen(script);
		if (scriptLen > MEMORY_PAGE_SIZE) scriptLen = MEMORY_PAGE_SIZE;
		char *encoded = base64encode(script,scriptLen);
		if (NULL != encoded){
			SendString(encoded);
			SendCrlf();
			vPortFree(encoded);
		}
	}
	else{
		SendCommandParamError("Invalid Script Page specified");
	}
}
