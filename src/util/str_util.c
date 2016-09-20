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
#include <stddef.h>
#include <string.h>

/**
 * Like strlen, except it ignores serial control characters. We consider
 * it the end of a serial message if the message ends with "\r\n" or if it
 * ends with '\0'.
 * @return length of the message, not including any serial control
 * characters.
 */
size_t serial_msg_strlen(const char *data)
{
        size_t len = 0;

	/* The for loop check handles the "\0" case */
        for (; *data; ++data, ++len) {
                switch (*data) {
                case '\r':
			/* Check for the "\r\n" case */
			if ('\n' != data[1])
				continue;

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
        for(; end >= str && isspace((int) *end); --end);
        *++end = 0;

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

/**
 * Left or leading strip.  Strips all leading '0' characters from the string
 * by effectively returning a pointer to the first non-zero character
 * that it encounters. It will always give you at least one character to
 * print before a decimal or end of string.
 * @return Pointer to the first non-zero character in the string exclusing
 * leading +/- symbols.
 */
char* str_util_lstrip_zeros_inline(char *str)
{
	char* const begin = str;

	char lead_sym = 0;
	switch(*str) {
	case '-':
	case '+':
		lead_sym = *str++;
	}

        for(; '0' == *str; ++str);

        switch (*str) {
        case '\0':
        case '.':
                if (str != begin)
			--str;
        }

	if (lead_sym)
		*--str = lead_sym;

	return str;
}

/**
 * Right or reverse strip.  Strips all trailing '0' characters from the right
 * side of the string by placing a null terminator where we detect the
 * last 0 character to be.  This may modify the string provided.
 * @return Pointer to the string passed in.
 */
char* str_util_rstrip_zeros_inline(char *str)
{
        /* Save the beginning val since we need it */
        char* const begin = str;

        for(; '\0' != *str && '.' != *str; ++str);

        if ('\0' == *str || '\0' == *++str)
                return begin;

        char *ld = str;
        for(++str; *str; ++str)
                if (*str != '0')
                        ld = str;

        *++ld = 0;
        return begin;
}

/**
 * Convenience method that calls both rstrip_inline then lstrip_inline to
 * strip both sides of the string.  Note that string passed in may modified
 * and that pointer returned might not equal pointer given.
 */
char* str_util_strip_zeros_inline(char *str)
{
        str = str_util_lstrip_zeros_inline(str);
        return str_util_rstrip_zeros_inline(str);
}

/**
 * Just like strncpy except this method always ensures the resulting string
 * is null ternimated.  Think of this as safe strncpy.
 * @param dest The destination pointer
 * @param src The source pointer
 * @param n The maximum length of the string.
 */
char* strntcpy(char* dest, const char* src, size_t n)
{
	if (!n)
		return dest;

	/* Copy and NULL terminate the string always to avoid overflow */
	strncpy(dest, src, --n);
	dest[n] = 0;

	return dest;
}
