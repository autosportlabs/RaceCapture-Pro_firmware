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
#include "jsmn.h"
#include "macros.h"
#include "serial.h"
#include "str_util.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

/**
 * Allocates a fresh unused token from the token pull.
 */
static jsmntok_t *jsmn_alloc_token(jsmn_parser *parser,
                                   jsmntok_t *tokens, size_t num_tokens)
{
    jsmntok_t *tok;
    if (parser->toknext >= num_tokens) {
        return NULL;
    }
    tok = &tokens[parser->toknext++];
    tok->data = NULL;
    tok->start = tok->end = -1;
    tok->size = 0;
#ifdef JSMN_PARENT_LINKS
    tok->parent = -1;
#endif
    return tok;
}

/**
 * Fills token type and boundaries.
 */
static void jsmn_fill_token(jsmntok_t *token, jsmntype_t type,
                            const char *js, int start, int end)
{
    token->type = type;
    token->data = (char *)js + start;
    token->start = start;
    token->end = end;
    token->size = 0;
}

/**
 * Fills next available token with JSON primitive.
 */
static jsmnerr_t jsmn_parse_primitive(jsmn_parser *parser, const char *js,
                                      jsmntok_t *tokens, size_t num_tokens)
{
    jsmntok_t *token;
    int start;

    start = parser->pos;

    for (; js[parser->pos] != '\0'; parser->pos++) {
        switch (js[parser->pos]) {
#ifndef JSMN_STRICT
            /* In strict mode primitive must be followed by "," or "}" or "]" */
        case ':':
#endif
        case '\t' :
        case '\r' :
        case '\n' :
        case ' ' :
        case ','  :
        case ']'  :
        case '}' :
            goto found;
        }
        if (js[parser->pos] < 32 || js[parser->pos] >= 127) {
            parser->pos = start;
            return JSMN_ERROR_INVAL;
        }
    }
#ifdef JSMN_STRICT
    /* In strict mode primitive must be followed by a comma/object/array */
    parser->pos = start;
    return JSMN_ERROR_PART;
#endif

found:
    token = jsmn_alloc_token(parser, tokens, num_tokens);
    if (token == NULL) {
        parser->pos = start;
        return JSMN_ERROR_NOMEM;
    }
    jsmn_fill_token(token, JSMN_PRIMITIVE, js, start, parser->pos);
#ifdef JSMN_PARENT_LINKS
    token->parent = parser->toksuper;
#endif
    parser->pos--;
    return JSMN_SUCCESS;
}

/**
 * Filsl next token with JSON string.
 */
static jsmnerr_t jsmn_parse_string(jsmn_parser *parser, const char *js,
                                   jsmntok_t *tokens, size_t num_tokens)
{
    jsmntok_t *token;

    int start = parser->pos;

    parser->pos++;

    /* Skip starting quote */
    for (; js[parser->pos] != '\0'; parser->pos++) {
        char c = js[parser->pos];

        /* Quote: end of string */
        if (c == '\"') {
            token = jsmn_alloc_token(parser, tokens, num_tokens);
            if (token == NULL) {
                parser->pos = start;
                return JSMN_ERROR_NOMEM;
            }
            jsmn_fill_token(token, JSMN_STRING, js, start+1, parser->pos);
#ifdef JSMN_PARENT_LINKS
            token->parent = parser->toksuper;
#endif
            return JSMN_SUCCESS;
        }

        /* Backslash: Quoted symbol expected */
        if (c == '\\') {
            parser->pos++;
            switch (js[parser->pos]) {
                /* Allowed escaped symbols */
            case '\"':
            case '/' :
            case '\\' :
            case 'b' :
            case 'f' :
            case 'r' :
            case 'n'  :
            case 't' :
                break;
                /* Allows escaped symbol \uXXXX */
            case 'u':
                /* TODO */
                break;
                /* Unexpected symbol */
            default:
                parser->pos = start;
                return JSMN_ERROR_INVAL;
            }
        }
    }
    parser->pos = start;
    return JSMN_ERROR_PART;
}

/**
 * Parse JSON string and fill tokens.
 */
jsmnerr_t jsmn_parse(jsmn_parser *parser, const char *js, jsmntok_t *tokens,
                     unsigned int num_tokens)
{
    jsmnerr_t r;
    int i;
    jsmntok_t *token;

    for (; js[parser->pos] != '\0'; parser->pos++) {
        char c;
        jsmntype_t type;

        c = js[parser->pos];
        switch (c) {
        case '{':
        case '[':
            token = jsmn_alloc_token(parser, tokens, num_tokens);
            if (token == NULL)
                return JSMN_ERROR_NOMEM;
            if (parser->toksuper != -1) {
                tokens[parser->toksuper].size++;
#ifdef JSMN_PARENT_LINKS
                token->parent = parser->toksuper;
#endif
            }
            token->type = (c == '{' ? JSMN_OBJECT : JSMN_ARRAY);
            token->start = parser->pos;
            token->data = (char *)js + parser->pos;
            parser->toksuper = parser->toknext - 1;
            break;
        case '}':
        case ']':
            type = (c == '}' ? JSMN_OBJECT : JSMN_ARRAY);
#ifdef JSMN_PARENT_LINKS
            if (parser->toknext < 1) {
                return JSMN_ERROR_INVAL;
            }
            token = &tokens[parser->toknext - 1];
            for (;;) {
                if (token->start != -1 && token->end == -1) {
                    if (token->type != type) {
                        return JSMN_ERROR_INVAL;
                    }
                    token->end = parser->pos + 1;
                    parser->toksuper = token->parent;
                    break;
                }
                if (token->parent == -1) {
                    break;
                }
                token = &tokens[token->parent];
            }
#else
            for (i = parser->toknext - 1; i >= 0; i--) {
                token = &tokens[i];
                if (token->start != -1 && token->end == -1) {
                    if (token->type != type) {
                        return JSMN_ERROR_INVAL;
                    }
                    parser->toksuper = -1;
                    token->end = parser->pos + 1;
                    break;
                }
            }
            /* Error if unmatched closing bracket */
            if (i == -1) return JSMN_ERROR_INVAL;
            for (; i >= 0; i--) {
                token = &tokens[i];
                if (token->start != -1 && token->end == -1) {
                    parser->toksuper = i;
                    break;
                }
            }
#endif
            break;
        case '\"':
            r = jsmn_parse_string(parser, js, tokens, num_tokens);
            if (r < 0) return r;
            if (parser->toksuper != -1)
                tokens[parser->toksuper].size++;
            break;
        case '\t' :
        case '\r' :
        case '\n' :
        case ':' :
        case ',':
        case ' ':
            break;
#ifdef JSMN_STRICT
            /* In strict mode primitives are: numbers and booleans */
        case '-':
        case '0':
        case '1' :
        case '2':
        case '3' :
        case '4':
        case '5':
        case '6':
        case '7' :
        case '8':
        case '9':
        case 't':
        case 'f':
        case 'n' :
#else
            /* In non-strict mode every unquoted value is a primitive */
        default:
#endif
            r = jsmn_parse_primitive(parser, js, tokens, num_tokens);
            if (r < 0) return r;
            if (parser->toksuper != -1)
                tokens[parser->toksuper].size++;
            break;

#ifdef JSMN_STRICT
            /* Unexpected char in strict mode */
        default:
            return JSMN_ERROR_INVAL;
#endif

        }
    }

    for (i = parser->toknext - 1; i >= 0; i--) {
        /* Unmatched opened object or array */
        if (tokens[i].start != -1 && tokens[i].end == -1) {
            return JSMN_ERROR_PART;
        }
    }

    return JSMN_SUCCESS;
}

/**
 * Creates a new parser based over a given  buffer with an array of tokens
 * available.
 */
void jsmn_init(jsmn_parser *parser)
{
    parser->pos = 0;
    parser->toknext = 0;
    parser->toksuper = -1;
}

const jsmntok_t * jsmn_trimData(const jsmntok_t *tok)
{
    tok->data[tok->end - tok->start] = '\0';
    return tok;
}

int jsmn_isNull(const jsmntok_t *tok)
{
    return strncmp("null", tok->data, 3) == 0;
}

const jsmntok_t * jsmn_find_node(const jsmntok_t *node, const char * name)
{
        if (NULL == node)
                return NULL;

        for (; node->start || node->end; ++node)
                if (0 == strcmp(name, jsmn_trimData(node)->data))
                        return node;

        return NULL;
}

const jsmntok_t * jsmn_find_get_node_value(const jsmntok_t *node,
                                           const char *name,
                                           const jsmntype_t val_type)
{
        const jsmntok_t *field = jsmn_find_node(node, name);

        if (!field)
                return NULL;

        /* Move to the value node */
        ++field;
        return val_type != field->type ? NULL : jsmn_trimData(field);
}

const jsmntok_t * jsmn_find_get_node_value_string(const jsmntok_t *node,
                                                  const char *name)
{
        return jsmn_find_get_node_value(node, name, JSMN_STRING);
}

const jsmntok_t * jsmn_find_get_node_value_prim(const jsmntok_t *node, const char *name)
{
        return jsmn_find_get_node_value(node, name, JSMN_PRIMITIVE);
}

bool jsmn_exists_set_val_int(const jsmntok_t* root, const char* field,
                             void* val)
{
	const jsmntok_t *node = jsmn_find_get_node_value_prim(root, field);

	if (!node)
		return false;

	int* value = (int*) val;
	*value = atoi(node->data);
	return true;
}

bool jsmn_exists_set_val_float(const jsmntok_t* root, const char* field,
                               void* val)
{
	const jsmntok_t *node = jsmn_find_get_node_value_prim(root, field);

	if (!node)
		return false;

	float* value = (float*) val;
	*value = atof(node->data);
	return true;
}

bool jsmn_exists_set_val_bool(const jsmntok_t* root, const char* field,
                              void* val)
{
	const jsmntok_t *node = jsmn_find_get_node_value_prim(root, field);

	if (!node)
		return false;

	bool* value = (bool*) val;
	*value = STR_EQ("true", node->data);
	return true;
}


bool jsmn_exists_set_val_string(const jsmntok_t* root, const char* field,
				void* val, const size_t max_len,
				const bool strip)
{
	const jsmntok_t *node = jsmn_find_get_node_value_string(root, field);

	if (!node)
		return false;

	jsmn_decode_string((char*) val, node->data, max_len);

	if (strip) {
		const char* data = strip_inline(node->data);
		if (node->data != data)
			memmove(node->data, data, strlen(data) + 1);
	}

	return true;
}

/**
 * Reads a raw JSON string and writes out the string represented in JSON.
 * This handles unescaping most unescaped characters that would come across
 * the wire.  Follows the safe practices of strntcpy and will ensure that
 * the string in dest is always NULL terminated, even if the length of the
 * JSON string was longer.
 * @param dst The destination array for the string output.
 * @param src The JSON string that we are reading in.
 * @param len The maximum length allowed in the destination string area
 * including NULL termination character.
 */
void jsmn_decode_string(char* dst, const char* src, size_t len)
{
	while(*src && len--) {
		switch(*src) {
		case '\\':
			switch(*++src) {
			case 'b':
				*dst = '\b';
				break;
			case 'f':
				*dst = '\f';
				break;
			case 'n':
				*dst = '\n';
				break;
			case 'r':
				*dst = '\r';
				break;
			case 't':
				*dst = '\t';
				break;
			case 'u':
				/* Handles \uXXXX. Not supported */
				for(int i = 0; i < 4 && *src; ++i, ++src);
				*dst = '?';
				break;
			default:
				/* No special handling required for \,/,"*/
				*dst = *src;
				break;
			}
			break;
		default:
			*dst = *src;
			break;
		}
		++src;
		++dst;
	}

	/* Ensure end of destination is always NULL'd out */
	*dst = 0;
}

/**
 * Encodes and writes a JSON string out to the given serial pipe, ensuring
 * that all special characters that need escaping are escaped.
 * @param serial The serial object to write out to
 * @param str The JSON string to write out.
 */
void jsmn_encode_write_string(struct Serial* serial, const char* str)
{
	while(*str) {
		switch(*str) {
		case '\b':
			serial_write_s(serial, "\\b");
			break;
		case '\f':
			serial_write_s(serial, "\\f");
			break;
		case '\n':
			serial_write_s(serial, "\\n");
			break;
		case '\r':
			serial_write_s(serial, "\\r");
			break;
		case '\t':
			serial_write_s(serial, "\\t");
			break;
		case '"':
			serial_write_s(serial, "\\\"");
			break;
		case '\\':
			serial_write_s(serial, "\\\\");
			break;
		default:
			serial_write_c(serial, *str);
			break;
		}
		++str;
	}
}
