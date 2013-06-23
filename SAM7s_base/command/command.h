/*
 * command.h
 *
 *  Created on: Jul 23, 2011
 *      Author: brent
 */

#ifndef COMMAND_H_
#define COMMAND_H_

#include "serial.h"

#define COMMAND_OK "result=\"ok\";"
#define COMMAND_ERROR "result=\"error\";"
#define ERROR_CODE_INVALID_PARAM -1
#define ERROR_CODE_MISSING_PARAMS -2
#define ERROR_CODE_INVALID_COMMAND -3
#define ERROR_CODE_CRITICAL_ERROR -4

typedef struct _cmd_context{
	Serial * serial;
	char * lineBuffer;
	size_t lineBufferSize;
} cmd_context;

typedef struct _cmd_t
{
	const char *cmd;
	const char *help;
	const char *paramHelp;
	void (*func)(Serial *serial, unsigned int argc, char **argv);
} cmd_t;

#define NULL_COMMAND {NULL, NULL,NULL, NULL}

void show_welcome(Serial *serial);

void show_command_prompt(Serial *serial);

void process_interactive_command(Serial *serial, char * buffer, size_t bufferSize);

void put_commandOK(Serial * serial);

void put_commandError(Serial * serial, int result);

void put_commandParamError(Serial * serial, char *msg);

void init_command(void);

cmd_context * get_command_context();

#endif
