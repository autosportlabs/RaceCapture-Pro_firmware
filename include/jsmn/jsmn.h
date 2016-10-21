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

#ifndef __JSMN_H_
#define __JSMN_H_

#include "cpp_guard.h"
#include "serial.h"
#include <stdbool.h>
#include <stddef.h>

CPP_GUARD_BEGIN

/**
 * JSON type identifier. Basic types are:
 * 	o Object
 * 	o Array
 * 	o String
 * 	o Other primitive: number, boolean (true/false) or null
 */
typedef enum {
    JSMN_PRIMITIVE = 0,
    JSMN_OBJECT = 1,
    JSMN_ARRAY = 2,
    JSMN_STRING = 3
} jsmntype_t;

typedef enum {
    /* Not enough tokens were provided */
    JSMN_ERROR_NOMEM = -1,
    /* Invalid character inside JSON string */
    JSMN_ERROR_INVAL = -2,
    /* The string is not a full JSON packet, more bytes expected */
    JSMN_ERROR_PART = -3,
    /* Everything was fine */
    JSMN_SUCCESS = 0
} jsmnerr_t;

/**
 * JSON token description.
 * @param		type	type (object, array, string etc.)
 * @param		start	start position in JSON data string
 * @param		end		end position in JSON data string
 */
typedef struct {
    jsmntype_t type;
    char * data;
    int start;
    int end;
    int size;
#ifdef JSMN_PARENT_LINKS
    int parent;
#endif
} jsmntok_t;

/**
 * JSON parser. Contains an array of token blocks available. Also stores
 * the string being parsed now and current position in that string
 */
typedef struct {
    unsigned int pos; /* offset in the JSON string */
    unsigned int toknext; /* next token to allocate */
    int toksuper; /* superior token node, e.g parent object or array */
} jsmn_parser;

/**
 * Create JSON parser over an array of tokens
 */
void jsmn_init(jsmn_parser *parser);

/**
 * Run JSON parser. It parses a JSON data string into and array of tokens, each describing
 * a single JSON object.
 */
jsmnerr_t jsmn_parse(jsmn_parser *parser, const char *js,
                     jsmntok_t *tokens, unsigned int num_tokens);

/**
 * null terminate the string at the current token for convenience
 */
const jsmntok_t * jsmn_trimData(const jsmntok_t *tok);

/*
 * returns 1 if the token value is a JSON null, 0 if not
 */
int jsmn_isNull(const jsmntok_t *tok);

/**
 * Finds the node with the given name.
 */
const jsmntok_t * jsmn_find_node(const jsmntok_t *node, const char * name);

/**
 * Finds the value node of the node with the given name and value type.
 */
const jsmntok_t * jsmn_find_get_node_value(const jsmntok_t *node,
                                           const char *name,
                                           const jsmntype_t val_type);

/**
 * Finds the string value node of a Node with the given name.
 */
const jsmntok_t * jsmn_find_get_node_value_string(const jsmntok_t *node,
                                                  const char *name);

/**
 * Finds the primitive value node of a Node with a given name.
 */
const jsmntok_t * jsmn_find_get_node_value_prim(const jsmntok_t *node, const char *name);

bool jsmn_exists_set_val_int(const jsmntok_t* root, const char* field,
                             void* val);

bool jsmn_exists_set_val_float(const jsmntok_t* root, const char* field,
                               void* val);

bool jsmn_exists_set_val_bool(const jsmntok_t* root, const char* field,
                              void* val);

bool jsmn_exists_set_val_string(const jsmntok_t* root, const char* field,
				void* val, const size_t max_len,
				const bool strip);

void jsmn_decode_string(char* dst, const char* src, size_t len);

void jsmn_encode_write_string(struct Serial* serial, const char* str);


CPP_GUARD_END

#endif /* __JSMN_H_ */
