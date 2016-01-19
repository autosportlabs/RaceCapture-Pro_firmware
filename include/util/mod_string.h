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

#ifndef MOD_STRING_H_
#define MOD_STRING_H_

#include "cpp_guard.h"

#include <stddef.h>

CPP_GUARD_BEGIN

void *memcpy(void *__restrict __dest, __const void *__restrict __src, size_t __n);

void *memset(void *s, int c, size_t n);

int strncmp(register const char *s1, register const char *s2, size_t n);

char *strcpy(char * __restrict s1, const char * __restrict s2);

size_t strlen(const char *s);

size_t strlcpy(char * __restrict dst, register const char * __restrict src, size_t siz);

char *strncpy(char * __restrict s1, register const char * __restrict s2, size_t n);

int strcmp(const char *s1, const char *s2);

int strcasecmp(const char *s1, const char *s2);

char *strtok(char * __restrict s1, const char * __restrict s2);

char *strtok_r(char * __restrict s1, const char * __restrict s2, char ** __restrict next_start);

size_t strspn(const char *s1, const char *s2);

char *strpbrk(const char *s1, const char *s2);

char *strcat(char * s1, const char * s2);

char *strstr (const char *phaystack, const char *pneedle);

char * strchr(const char *s, const int c);

CPP_GUARD_END

#endif /* MOD_STRING_H_ */
