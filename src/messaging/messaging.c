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

void process_message(Serial *serial, char * buffer, size_t bufferSize){

	interactive_read_line(serial, buffer, bufferSize);

	if (strlen(buffer) == 0){
		show_welcome(serial);
		show_command_prompt(serial);
	}
	else{
		if (buffer[0] == '{'){
			process_api(serial, buffer, bufferSize);
		}
		else{
			process_interactive_command(serial, buffer, bufferSize);
		}
	}
}


