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

#ifndef COMMAND_H_
#define COMMAND_H_

#include "cpp_guard.h"
#include "serial.h"

CPP_GUARD_BEGIN

#define COMMAND_OK_MSG "result=\"ok\";"
#define COMMAND_ERROR_MSG "result=\"error\";"
#define COMMAND_OK 1
#define COMMAND_UNKNOWN 0
#define ERROR_CODE_INVALID_PARAM -1
#define ERROR_CODE_MISSING_PARAMS -2
#define ERROR_CODE_INVALID_COMMAND -3
#define ERROR_CODE_CRITICAL_ERROR -4

typedef struct _cmd_context {
    struct Serial * serial;
    char * lineBuffer;
    size_t lineBufferSize;
} cmd_context;

typedef struct _cmd_t {
    const char *cmd;
    const char *help;
    const char *paramHelp;
    void (*func)(struct Serial *serial, unsigned int argc, char **argv);
} cmd_t;

#define SYSTEM_COMMAND(_CMD, _DESC, _ARG_DESC, _METHOD) \
        {(_CMD),(_DESC),(_ARG_DESC),(_METHOD)},

#define NULL_COMMAND {NULL, NULL, NULL, NULL}

void show_welcome(struct Serial *serial);

void show_command_prompt(struct Serial *serial);

int process_command(struct Serial *serial, char * buffer, size_t bufferSize);

void put_commandOK(struct Serial * serial);

void put_commandError(struct Serial * serial, int result);

void put_commandParamError(struct Serial * serial, char *msg);

void init_command(void);

cmd_context * get_command_context();

CPP_GUARD_END

#endif
