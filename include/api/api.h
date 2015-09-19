/**
 * AutoSport Labs - Race Capture Firmware
 *
 * Copyright (C) 2014 AutoSport Labs
 *
 * This file is part of the Race Capture firmware suite
 *
 * This is free software: you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details. You should have received a copy of the GNU
 * General Public License along with this code. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef API_H_
#define API_H_

#include "jsmn.h"
#include "serial.h"

#define API_SUCCESS_NO_RETURN 	2
#define API_SUCCESS 			1

#define API_ERROR_UNKNOWN_MSG	0
#define API_ERROR_PARAMETER 	-1
#define API_ERROR_MALFORMED 	-2
#define API_ERROR_SEVERE		-99
#define API_ERROR_UNSPECIFIED 	-100

typedef struct _api_context {
    Serial * serial;
    char * lineBuffer;
    size_t lineBufferSize;
} api_context;

typedef struct _api_t {
    const char *cmd;
    int (*func)(Serial *serial, const jsmntok_t *json);
} api_t;

#define NULL_API {NULL, NULL}

void initApi();

void json_valueStart(Serial *serial, const char *name);
void json_null(Serial *serial, const char *name, int more);
void json_int(Serial *serial, const char *name, int value, int more);
void json_uint(Serial *serial, const char *name, unsigned int value, int more);
void json_escapedString(Serial *serial, const char *name, const char *value, int more);
void json_string(Serial *serial, const char *name, const char *value, int more);
void json_float(Serial *serial, const char *name, float value, int precision, int more);
void json_objStartString(Serial *serial, const char * label);
void json_objStartInt(Serial *serial, int label);
void json_objStart(Serial *serial);
void json_objEnd(Serial *serial, int more);
void json_arrayStart(Serial *serial, const char * name);
void json_arrayElementString(Serial *serial, const char *value, int more);
void json_arrayElementInt(Serial *serial, int value, int more);
void json_arrayElementFloat(Serial *serial, float value, int precision, int more);
void json_arrayEnd(Serial *serial, int more);
void json_sendResult(Serial *serial, const char *messageName, int resultCode);

int process_api(Serial *serial, char * buffer, size_t bufferSize);

#endif /* API_H_ */
