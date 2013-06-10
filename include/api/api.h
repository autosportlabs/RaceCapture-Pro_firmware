/*
 * api.h
 *
 *  Created on: Jun 9, 2013
 *      Author: brent
 */

#ifndef API_H_
#define API_H_

#include "jsmn.h"
#include "serial.h"

typedef struct _api_context{
	Serial * serial;
	char * lineBuffer;
	size_t lineBufferSize;
} api_context;


typedef struct _api_t
{
	const char *cmd;
	void (*func)(Serial *serial, const jsmntok_t *json);
} api_t;

#define NULL_API {NULL, NULL}

void process_api(Serial *serial, char * buffer, size_t bufferSize);


#endif /* API_H_ */
