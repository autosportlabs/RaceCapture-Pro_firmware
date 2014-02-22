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

#define MAJOR_REV_STR  XSTR(MAJOR_REV)
#define MINOR_REV_STR  XSTR(MINOR_REV)
#define BUGFIX_REV_STR XSTR(BUGFIX_REV)

#define WELCOME_MSG "Welcome to RaceCapture/Pro : Firmware Version " MAJOR_REV_STR "." MINOR_REV_STR "." BUGFIX_REV_STR
#define COMMAND_PROMPT "RaceCapture/Pro"

#define LUA_ENABLED

#define SYSTEM_COMMANDS {LOGGER_COMMANDS, BASE_COMMANDS, LUA_COMMANDS, NULL_COMMAND}

#define SYSTEM_APIS {LOGGER_API, NULL_API }


//USB manufacturer string descriptor
//first element = length of this entire array, in bytes
//second element = 0x03 = descriptor type string (don't change)
#define USB_MFG_STRING_DESCRIPTOR 	\
	{ \
		30, \
		0x03, \
		'A', 0x00, 'u', 0x00, 't', 0x00, 'o', 0x00, 's', 0x00, 'p', 0x00, 'o', 0x00, 'r', 0x00, 't', 0x00, ' ', 0x00, 'L', 0x00, 'a', 0x00, 'b', 0x00, 's', 0x00 \
	}

//USB product string descriptor.
//first element = length of this entire array, in bytes
//second element = 0x03 = descriptor type string (don't change)
#define USB_PRODUCT_STRING_DESCRIPTOR \
	{ \
		32, \
		0x03, \
		'R', 0x00, 'a', 0x00, 'c', 0x00, 'e', 0x00, 'C', 0x00, 'a', 0x00, 'p', 0x00, 't', 0x00, 'u', 0x00, 'r', 0x00, 'e', 0x00, \
		'/', 0x00, 'P', 0x00, 'r', 0x00, 'o', 0x00 \
	}

#define WATCHDOG_TIMEOUT_MS 3000


#endif /* CONSTANTS_H_ */
