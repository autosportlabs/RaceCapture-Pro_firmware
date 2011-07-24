/*
 * loggerCommands.c
 *
 *  Created on: Jul 24, 2011
 *      Author: brent
 */
#include "loggerCommands.h"
#include "usb_comm.h"

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


}

void ReadScriptPage(unsigned char argc, char **argv){


}
