/*
 * messaging.h
 *
 *  Created on: Jun 23, 2013
 *      Author: brent
 */

#ifndef MESSAGING_H_
#define MESSAGING_H_
#include "serial.h"
#include <stddef.h>

void initMessaging();

void process_msg_interactive(Serial *serial, char * buffer, size_t bufferSize);
void process_msg(Serial *serial, char * buffer, size_t bufferSize);



#endif /* MESSAGING_H_ */
