/*
 * messaging.c
 *
 *  Created on: Jun 23, 2013
 *      Author: brent
 */
#include "messaging.h"
#include "serial.h"
#include "command.h"
#include "api.h"
#include "mod_string.h"

void initMessaging(){
	init_command();
	initApi();
}

void process_msg(Serial *serial, char * buffer, size_t bufferSize){
	if (buffer[0] == '{'){
		process_api(serial, buffer, bufferSize);
	}
	else{
		process_interactive_command(serial, buffer, bufferSize);
	}
}

void process_msg_interactive(Serial *serial, char * buffer, size_t bufferSize){
	interactive_read_line(serial, buffer, bufferSize);

	if (strlen(buffer) == 0){
		show_welcome(serial);
		show_command_prompt(serial);
	}
	else{
		process_msg(serial, buffer, bufferSize);
	}
}


