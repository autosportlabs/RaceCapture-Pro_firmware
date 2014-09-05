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
#include "loggerApi.h"
#include "api.h"
#include "command.h"


#define XSTR(s) STR(s)
#define STR(s) #s

#define DEVICE_NAME    "RCP"
#define MAJOR_REV_STR  XSTR(MAJOR_REV)
#define MINOR_REV_STR  XSTR(MINOR_REV)
#define BUGFIX_REV_STR XSTR(BUGFIX_REV)

#define WELCOME_MSG "Welcome to RaceCapture/Pro : Firmware Version " MAJOR_REV_STR "." MINOR_REV_STR "." BUGFIX_REV_STR
#define COMMAND_PROMPT "RaceCapture/Pro"

#define SYSTEM_COMMANDS {LOGGER_COMMANDS, BASE_COMMANDS, LUA_COMMANDS, NULL_COMMAND}

#define SYSTEM_APIS {LOGGER_API, NULL_API }

#define WATCHDOG_TIMEOUT_MS 2000


#endif /* CONSTANTS_H_ */
