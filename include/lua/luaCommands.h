/*
 * luaCommands.h
 *
 *  Created on: Jul 24, 2011
 *      Author: brent
 */

#ifndef LUACOMMANDS_H_
#define LUACOMMANDS_H_
#include "command.h"
#include "serial.h"

#define LUA_COMMANDS \
		{"lua", "Enter the lua script interpreter.","", ExecLuaInterpreter}, \
		{"reloadScript", "Reloads the current script.","",ReloadScript}

Serial *getLuaSerialContext();
void ExecLuaInterpreter(Serial *serial, unsigned int argc, char **argv);
void ReloadScript(Serial *serial, unsigned int argc, char **argv);
void ShowMemInfo(Serial *serial, unsigned int argc, char **argv);

int in_interactive_mode();
#endif /* LUACOMMANDS_H_ */
