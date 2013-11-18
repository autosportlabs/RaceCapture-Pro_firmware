/*
 * messaging.c
 *
 *  Created on: Jun 23, 2013
 *      Author: brent
 */
#include "messaging.h"
#include "mod_string.h"
#include "serial.h"
#include "printk.h"

void initMessaging(){
	init_command();
	initApi();
}

int process_msg(Serial *serial, char * buffer, size_t bufferSize){
	if (buffer[0] == '{'){
		return process_api(serial, buffer, bufferSize);
	}
	else{
		return process_command(serial, buffer, bufferSize);
	}
}

void process_msg_interactive(Serial *serial, char * buffer, size_t bufferSize){
	interactive_read_line(serial, buffer, bufferSize);

	if (strlen(buffer) == 0){
		show_welcome(serial);
		show_command_prompt(serial);
	}
	else{
		int res = process_msg(serial, buffer, bufferSize);
		if (! MESSAGE_SUCCESS(res)){
			serial->put_s("Unknown Command- Press Enter for Help.");
			put_crlf(serial);
		}
		show_command_prompt(serial);
	}
}


