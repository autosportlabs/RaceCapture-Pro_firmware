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

static int lockedApiMode = 0;

void initMessaging(){
	init_command();
	initApi();
}

void process_msg(Serial *serial, char * buffer, size_t bufferSize){
	if (lockedApiMode){
		read_line(serial, buffer, bufferSize);
	}
	else{
		interactive_read_line(serial, buffer, bufferSize);
	}

	if (strlen(buffer) == 0){
		show_welcome(serial);
		show_command_prompt(serial);
		lockedApiMode = 0;
	}
	else{
		if (buffer[0] == '{'){
			lockedApiMode = 1;
			process_api(serial, buffer, bufferSize);
		}
		else{
			int res = process_command(serial, buffer, bufferSize);
			if (res != COMMAND_OK){
				serial->put_s("Unknown Command- Press Enter for Help.");
				put_crlf(serial);
			}
			show_command_prompt(serial);
		}
	}
}


