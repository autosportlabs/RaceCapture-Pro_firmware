/*
 * Race Capture Firmware
 *
 * Copyright (C) 2016 Autosport Labs
 *
 * This file is part of the Race Capture firmware suite
 *
 * This is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details. You should
 * have received a copy of the GNU General Public License along with
 * this code. If not, see <http://www.gnu.org/licenses/>.
 */

#include "baseCommands.h"
#include "command.h"
#include "constants.h"
#include "loggerCommands.h"
#include "luaCommands.h"
#include "macros.h"
#include "serial.h"
#include "versionInfo.h"
#include <stdio.h>
#include <string.h>

#define SYSTEM_COMMANDS { LOGGER_COMMANDS       \
                          BASE_COMMANDS         \
                          LUA_COMMANDS          \
                          NULL_COMMAND          \
        }

#define MAX_ARGS	10
#define MAX_HEADER_WIDTH	80

static const char device_name[] = FRIENDLY_DEVICE_NAME;
static const cmd_t commands[] = SYSTEM_COMMANDS;
static int menuPadding = 0;

cmd_context commandContext;

static void set_command_context(struct Serial *serial, char *buffer, size_t bufferSize)
{
    commandContext.lineBuffer = buffer;
    commandContext.serial = serial;
    commandContext.lineBufferSize = bufferSize;
}

static void clear_command_context()
{
    commandContext.lineBuffer = NULL;
    commandContext.serial = NULL;
    commandContext.lineBufferSize = 0;
}

static void show_help(struct Serial *serial)
{
        serial_write_s(serial, "Available Commands:");
        put_crlf(serial);
        put_crlf(serial);

    const cmd_t * cmd = commands;
    while (cmd->cmd != NULL) {
            serial_write_s(serial, cmd->cmd);
        int padding = menuPadding - strlen(cmd->cmd);

        while (padding-- > 0)
                serial_write_c(serial, ' ');

        serial_write_s(serial, ": ");
        serial_write_s(serial, cmd->help);
        serial_write_s(serial, " Usage: ");
        serial_write_s(serial, cmd->cmd);
        serial_write_c(serial, ' ');
        serial_write_s(serial, cmd->paramHelp);

        put_crlf(serial);
        cmd++;
    }
}

static void calculateMenuPadding()
{
    const cmd_t * cmd = commands;

    while (cmd->cmd != NULL) {
        int len = strlen(cmd->cmd);
        if (len > menuPadding)
            menuPadding = len;
        cmd++;
    }

    menuPadding++;
}

static void send_header(struct Serial *serial, unsigned int len)
{
    while (len-- > 0) {
        serial_write_c(serial, '=');
    }
    put_crlf(serial);
}

void show_welcome(struct Serial *serial)
{
	static size_t len = 0;
	static char msg[MAX_HEADER_WIDTH];

	if (!len) {
		snprintf(msg, MAX_HEADER_WIDTH, "Welcome to %s version %s",
			 device_name, version_full());
		msg[MAX_HEADER_WIDTH - 1] = 0; // Terminate it as a precaution.
		len = strlen(msg);
	}

	put_crlf(serial);
	send_header(serial, len);
	serial_write_s(serial, msg);
	put_crlf(serial);
	send_header(serial, len);
	put_crlf(serial);
	show_help(serial);
}

void show_command_prompt(struct Serial *serial)
{
    serial_write_s(serial, device_name);
    serial_write_s(serial, " > ");
}

static int execute_command(struct Serial *serial, char *buffer)
{
    unsigned char argc = 0;
    char *argv[MAX_ARGS];

    argv[argc] = strtok(buffer, " ");

    do {
        argv[++argc] = strtok(NULL, " ");
    } while ((argc < 30) && (argv[argc] != NULL));

    const cmd_t * cmd = commands;

    while (cmd->cmd != NULL) {
        if (!strcmp(argv[0], cmd->cmd)) {
            cmd->func(serial, argc, argv);
            put_crlf(serial);
            break;
        }
        cmd++;
    }

    return (NULL != cmd->cmd);
}

int process_command(struct Serial *serial, char * buffer, size_t bufferSize)
{
    //this is not thread safe. need to throw a mutex around here
    set_command_context(serial, buffer, bufferSize);

    int res = execute_command(serial, buffer);
    clear_command_context();
    return res;
}

void put_commandOK(struct Serial *serial)
{
    serial_write_s(serial, COMMAND_OK_MSG);
}

void put_commandParamError(struct Serial *serial, char *msg)
{
    serial_write_s(serial, COMMAND_ERROR_MSG);
    serial_write_s(serial, "extended=\"");
    serial_write_s(serial, msg);
    serial_write_s(serial, "\";");
}

void put_commandError(struct Serial *serial, int result)
{
    serial_write_s(serial, COMMAND_ERROR_MSG);
    serial_write_s(serial, "code=");
    put_int(serial, result);
    serial_write_s(serial, ";");
}

void init_command(void)
{
    calculateMenuPadding();
}

cmd_context * get_command_context()
{
    return &commandContext;
}
