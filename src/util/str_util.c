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

#include "str_util.h"

#include <ctype.h>

/**
 * Like strlen, except it ignores serial control characters.
 * @return length of the message, not including any serial control
 * characters.
 */
size_t serial_msg_strlen(const char *data)
{
        size_t len = 0;

        for (; *data; ++data, ++len) {
                switch (*data) {
                case '\r':
                case '\n':
                        return len;
                }
        }

        return len;
}

/**
 * Left or leading strip.  Strips all whitespace characters from the string
 * by effectively returning a pointer to the first non-whitespace character
 * that it encounters.  Will not modify the string, but may not return the
 * pointer that was initially given.
 * @return Pointer to the first non-whitespace character in the string.
 */
char* lstrip_inline(char *str)
{
        /* Trim leading space */
        for(; isspace((int) *str); ++str);
        return str;
}

/**
 * Right or reverse strip.  Strips all whitespace characters starting from
 * the right side of the string by placing a null terminator when we detect
 * a non white-space character.  May modify the string.
 * @return Pointer to the string passed in.
 */
char* rstrip_inline(char *str)
{
        /* Trim trailing space */
        char *end = str + strlen(str) - 1;
        for(; end > str && isspace((int) *end); --end);
        *++end = '\0';
        return str;
}

/**
 * Convenience method that calls both rstrip_inline then lstrip_inline to
 * strip both sides of the string.  Note that string passed in may modified
 * and that pointer returned might not equal pointer given.
 */
char* strip_inline(char *str)
{
        return lstrip_inline(rstrip_inline(str));
}
