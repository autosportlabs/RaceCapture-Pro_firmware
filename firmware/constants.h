/*
 * constants.h
 *
 *  Created on: Jul 23, 2011
 *      Author: brent
 */

#ifndef CONSTANTS_H_
#define CONSTANTS_H_

//Include all of the command definitions here
#include "baseCommands.h"
#include "luaCommands.h"
#include "loggerCommands.h"

#include "command.h"

#define FIRMWARE_VERSION "1.0.0.0"
#define WELCOME_MSG "Welcome to RaceCapture/Pro : Firmware Version " FIRMWARE_VERSION
#define COMMAND_PROMPT "RaceCapture/Pro"

#define LUA_ENABLED

#define SYSTEM_COMMANDS {BASE_COMMANDS, LUA_COMMANDS, LOGGER_COMMANDS, NULL_COMMAND}

#endif /* CONSTANTS_H_ */
