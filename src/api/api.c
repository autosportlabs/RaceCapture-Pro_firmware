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

#include "api.h"
#include "constants.h"
#include "printk.h"
#include "mod_string.h"

#define JSON_TOKENS 200

static jsmn_parser g_jsonParser;
static jsmntok_t g_json_tok[JSON_TOKENS];

const api_t apis[] = SYSTEM_APIS;

void initApi()
{
    jsmn_init(&g_jsonParser);
}

static void putQuotedStr(const Serial *serial, const char *str)
{
    serial->put_c('"');
    serial->put_s(str);
    serial->put_c('"');
}

static void putKeyAndColon(const Serial *serial, const char *key)
{
    putQuotedStr(serial, key);
    serial->put_c(':');
}

static void putNull(Serial *serial)
{
    serial->put_s("null");
}

static void putCommaIfNecessary(const Serial *serial, int necessary)
{
    if (necessary)
        serial->put_c(',');
}

void json_valueStart(Serial *serial, const char *name)
{
    putKeyAndColon(serial, name);
}

void json_null(Serial *serial, const char *name, int more)
{
    putKeyAndColon(serial, name);
    putNull(serial);
    putCommaIfNecessary(serial, more);
}

void json_int(Serial *serial, const char *name, int value, int more)
{
    putKeyAndColon(serial, name);
    put_int(serial, value);
    putCommaIfNecessary(serial, more);
}

void json_uint(Serial *serial, const char *name, unsigned int value, int more)
{
    putKeyAndColon(serial, name);
    put_uint(serial, value);
    putCommaIfNecessary(serial, more);
}

void json_escapedString(Serial *serial, const char *name, const char *value, int more)
{
    putKeyAndColon(serial, name);
    serial->put_c('"');
    put_escapedString(serial, value, strlen(value));
    serial->put_c('"');
    putCommaIfNecessary(serial, more);
}

void json_string(Serial *serial, const char *name, const char *value, int more)
{
    putKeyAndColon(serial, name);

    if (value) {
        putQuotedStr(serial, value);
    } else {
        putNull(serial);
    }

    putCommaIfNecessary(serial, more);
}

void json_float(Serial *serial, const char *name, float value, int precision, int more)
{
    putKeyAndColon(serial, name);
    put_float(serial, value, precision);
    putCommaIfNecessary(serial, more);
}

void json_objStartString(Serial *serial, const char *label)
{
    putKeyAndColon(serial, label);
    serial->put_c('{');
}

// Call itoa here and use above?
void json_objStartInt(Serial *serial, int label)
{
    serial->put_c('"');
    put_int(serial, label);
    serial->put_s("\":{");
}

void json_objStart(Serial *serial)
{
    serial->put_c('{');
}

void json_objEnd(Serial *serial, int more)
{
    serial->put_c('}');
    putCommaIfNecessary(serial, more);
}

void json_arrayStart(Serial *serial, const char * name)
{
    if (name != NULL)
        putKeyAndColon(serial, name);

    serial->put_c('[');
}

void json_arrayElementString(Serial *serial, const char *value, int more)
{
    putQuotedStr(serial, value);
    putCommaIfNecessary(serial, more);
}

void json_arrayElementInt(Serial *serial, int value, int more)
{
    put_int(serial, value);
    putCommaIfNecessary(serial, more);
}

void json_arrayElementFloat(Serial *serial, float value, int precision, int more)
{
    put_float(serial, value, precision);
    putCommaIfNecessary(serial, more);
}

void json_arrayEnd(Serial *serial, int more)
{
    serial->put_c(']');
    putCommaIfNecessary(serial, more);
}

void json_sendResult(Serial *serial, const char *messageName, int resultCode)
{
    json_objStart(serial);
    json_objStartString(serial, messageName);
    json_int(serial, "rc", resultCode, 0);
    json_objEnd(serial, 0);
    json_objEnd(serial, 0);
}

static int dispatch_api(Serial *serial, const char * apiMsgName, const jsmntok_t *apiPayload)
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

static int execute_api(Serial * serial, const jsmntok_t *json)
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

int process_api(Serial *serial, char *buffer, size_t bufferSize)
{
    jsmn_init(&g_jsonParser);
    memset(g_json_tok, 0, sizeof(g_json_tok));

    int r = jsmn_parse(&g_jsonParser, buffer, g_json_tok, JSON_TOKENS);
    if (r == JSMN_SUCCESS) {
        return execute_api(serial, g_json_tok);
    } else {
        pr_warning_int_msg("API Error: ", r);
        return API_ERROR_MALFORMED;
    }
}
