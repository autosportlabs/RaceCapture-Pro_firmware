/*
 * messaging.h
 *
 *  Created on: Jun 23, 2013
 *      Author: brent
 */

#ifndef MESSAGING_H_
#define MESSAGING_H_
#include <stddef.h>
#include "serial.h"
#include "command.h"
#include "api.h"


#define MESSAGE_SUCCESS(X) (X == COMMAND_OK || X == API_SUCCESS || X == API_SUCCESS_NO_RETURN )

void initMessaging();

void process_msg_interactive(Serial *serial, char * buffer, size_t bufferSize);
int process_msg(Serial *serial, char * buffer, size_t bufferSize);



#endif /* MESSAGING_H_ */
