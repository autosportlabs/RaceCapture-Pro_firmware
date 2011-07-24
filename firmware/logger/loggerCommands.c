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

static struct cmd_t execLuaCmd = {"lua",
		"Executes the specified Lua script fragment",
		ExecLuaCommand,
		NULL};

static struct cmd_t writeScriptPageCmd = {"writeScriptPage",
		"Writes the base64 encoded script to the specified page",
		WriteScriptPage,
		NULL};

static struct cmd_t readScriptPageCmd = {"readScriptPage",
		"Returns the specified script page in base64 encoded format",
		ReadScriptPage,
		NULL};

void InitLoggerCommands(){
	AppendCommand(&execLuaCmd);
	AppendCommand(&writeScriptPageCmd);
	AppendCommand(&readScriptPageCmd);
}

void ExecLuaCommand(unsigned char argc, char **argv){

}

void WriteScriptPage(unsigned char argc, char **argv){

	if (argc < 3){
		SendString("Invalid Parameters");
		SendCrlf();
		ShowHelp();
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


}
