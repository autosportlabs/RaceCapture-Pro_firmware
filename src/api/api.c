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


#include "api.h"
#include "constants.h"
#include "loggerApi.h"
#include "panic.h"
#include "printk.h"
#include <string.h>

#define JSON_TOKENS 200

static jsmn_parser g_jsonParser;
static jsmntok_t* g_json_tok;
static const api_t apis[] = {API_METHODS NULL_API};

void initApi()
{
	if (NULL == g_json_tok)
		g_json_tok = calloc(sizeof(jsmntok_t), JSON_TOKENS);

	if (NULL == g_json_tok)
		panic(PANIC_CAUSE_MALLOC);

	jsmn_init(&g_jsonParser);
}

static void putQuotedStr(struct Serial *serial, const char *str)
{
	serial_write_c(serial, '"');
	jsmn_encode_write_string(serial, str);
	serial_write_c(serial, '"');
}

static void putKeyAndColon(struct Serial *serial, const char *key)
{
    putQuotedStr(serial, key);
    serial_write_c(serial, ':');
}

static void putNull(struct Serial *serial)
{
    serial_write_s(serial, "null");
}

static void putCommaIfNecessary(struct Serial *serial, int necessary)
{
    if (necessary)
        serial_write_c(serial, ',');
}

void json_valueStart(struct Serial *serial, const char *name)
{
    putKeyAndColon(serial, name);
}

void json_null(struct Serial *serial, const char *name, int more)
{
    putKeyAndColon(serial, name);
    putNull(serial);
    putCommaIfNecessary(serial, more);
}

void json_int(struct Serial *serial, const char *name, int value, int more)
{
    putKeyAndColon(serial, name);
    put_int(serial, value);
    putCommaIfNecessary(serial, more);
}

void json_uint(struct Serial *serial, const char *name, unsigned int value, int more)
{
    putKeyAndColon(serial, name);
    put_uint(serial, value);
    putCommaIfNecessary(serial, more);
}

void json_escapedString(struct Serial *serial, const char *name, const char *value, int more)
{
	putKeyAndColon(serial, name);
	serial_write_c(serial, '"');
	jsmn_encode_write_string(serial, value);
	serial_write_c(serial, '"');
	putCommaIfNecessary(serial, more);
}

void json_string(struct Serial *serial, const char *name, const char *value, int more)
{
    putKeyAndColon(serial, name);

    if (value) {
        putQuotedStr(serial, value);
    } else {
        putNull(serial);
    }

    putCommaIfNecessary(serial, more);
}

void json_float(struct Serial *serial, const char *name, float value, int precision, int more)
{
    putKeyAndColon(serial, name);
    put_float(serial, value, precision);
    putCommaIfNecessary(serial, more);
}

void json_bool(struct Serial *serial, const char *name,
               const bool value, bool more)
{
    putKeyAndColon(serial, name);
    serial_write_s(serial, value ? "true" : "false");
    putCommaIfNecessary(serial, more);
}

void json_objStartString(struct Serial *serial, const char *label)
{
    putKeyAndColon(serial, label);
    serial_write_c(serial, '{');
}

// Call itoa here and use above?
void json_objStartInt(struct Serial *serial, int label)
{
    serial_write_c(serial, '"');
    put_int(serial, label);
    serial_write_s(serial, "\":{");
}

void json_objStart(struct Serial *serial)
{
    serial_write_c(serial, '{');
}

void json_objEnd(struct Serial *serial, int more)
{
    serial_write_c(serial, '}');
    putCommaIfNecessary(serial, more);
}

void json_arrayStart(struct Serial *serial, const char * name)
{
    if (name != NULL)
        putKeyAndColon(serial, name);

    serial_write_c(serial, '[');
}

void json_arrayElementString(struct Serial *serial, const char *value, int more)
{
    putQuotedStr(serial, value);
    putCommaIfNecessary(serial, more);
}

void json_arrayElementInt(struct Serial *serial, int value, int more)
{
    put_int(serial, value);
    putCommaIfNecessary(serial, more);
}

void json_arrayElementFloat(struct Serial *serial, float value, int precision, int more)
{
    put_float(serial, value, precision);
    putCommaIfNecessary(serial, more);
}

void json_arrayEnd(struct Serial *serial, int more)
{
    serial_write_c(serial, ']');
    putCommaIfNecessary(serial, more);
}

void json_sendResult(struct Serial *serial, const char *messageName, int resultCode)
{
    json_objStart(serial);
    json_objStartString(serial, messageName);
    json_int(serial, "rc", resultCode, 0);
    json_objEnd(serial, 0);
    json_objEnd(serial, 0);
}

static int dispatch_api(struct Serial *serial, const char * apiMsgName, const jsmntok_t *apiPayload)
{

    const api_t * api = apis;
    int res = API_ERROR_UNSPECIFIED;
    while (api->cmd != NULL) {
        if (strcmp(api->cmd, apiMsgName) == 0) {
            res = api->func(serial, apiPayload);
            if (res != API_SUCCESS_NO_RETURN)
                json_sendResult(serial, apiMsgName, res);
            break;
        }
        api++;
    }
    if (NULL == api->cmd) {
        res = API_ERROR_UNKNOWN_MSG;
        json_sendResult(serial, apiMsgName, res);
    }
    put_crlf(serial);
    return res;
}

static int execute_api(struct Serial * serial, const jsmntok_t *json)
{
    const jsmntok_t *root = &json[0];
    if (root->type == JSMN_OBJECT && root->size == 2) {
        const jsmntok_t *apiMsgName = &json[1];
        const jsmntok_t *payload = &json[2];
        if (apiMsgName->type == JSMN_STRING) {
            jsmn_trimData(apiMsgName);
            return dispatch_api(serial, apiMsgName->data, payload);
        } else {
            return API_ERROR_MALFORMED;
        }
    } else {
        return API_ERROR_MALFORMED;
    }
}

int process_api(struct Serial *serial, char *buffer, size_t bufferSize)
{
	jsmn_init(&g_jsonParser);
	memset(g_json_tok, 0, sizeof(jsmntok_t) * JSON_TOKENS);

	const int r = jsmn_parse(&g_jsonParser, buffer, g_json_tok, JSON_TOKENS);
	if (JSMN_SUCCESS == r)
		return execute_api(serial, g_json_tok);

	pr_warning("API Parsing Error: \"");
	pr_warning(buffer);
	pr_warning_int_msg("\"\r\n failed with code ", r);
	return API_ERROR_MALFORMED;
}

const char* unknown_api_key()
{
        return "unknown";
}
