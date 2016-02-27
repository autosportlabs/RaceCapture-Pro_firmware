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

#include "mod_string.h"

#include <stdbool.h>

void * memcpy(void * __restrict s1, const void * __restrict s2, size_t n)
{
    register char * __restrict r1 = (char *)s1;
    register const char * __restrict r2 = (const char *)s2;

    while (n) {
        *r1++ = *r2++;
        --n;
    }
    return s1;
}

void *memset(void *s, int c, size_t n)
{
    register unsigned char *p = (unsigned char *) s;

    while (n) {
        *p++ = (unsigned char)c;
        --n;
    }
    return s;
}


int strncmp(register const char *s1, register const char *s2, size_t n)
{
    int r = 0;

    while (n--
           && ((r = ((int)(*((unsigned char *)s1))) - *((unsigned char *)s2++))
               == 0)
           && *s1++);

    return r;
}

char *strcpy(char * __restrict s1, const char * __restrict s2)
{
    register char *s = s1;

    while ( (*s++ = *s2++) != 0 );

    return s1;
}

size_t strlen(const char *s)
{
    register const char *p;

    for (p=s ; *p ; p++);

    return p - s;
}


size_t strlcpy(char * __restrict dst, register const char * __restrict src, size_t siz)
{
    register char *d = dst;
    register const char *s = src;
    register size_t n = siz;

    /* Copy as many bytes as will fit */
    if (n != 0 && --n != 0) {
        do {
            if ((*d++ = *s++) == 0)
                break;
        } while (--n != 0);
    }

    /* Not enough room in dst, add NUL and traverse rest of src */
    if (n == 0) {
        if (siz != 0)
            *d = '\0';		/* NUL-terminate dst */
        while (*s++)
            ;
    }

    return(s - src - 1);	/* count does not include NUL */
}

char *strncpy(char * __restrict s1, register const char * __restrict s2, size_t n)
{
    register char *s = s1;

    while (n) {
        if ((*s = *s2) != 0) s2++; /* Need to fill tail with 0s. */
        ++s;
        --n;
    }

    return s1;
}

int strcmp(const char *s1, const char *s2)
{
    int r;

    while (((r = ((int)(*((unsigned char *)s1))) - *((unsigned char *)s2++)) == 0) && *s1++);

    return r;
}

#define TOLOWER(x) ((x) | 0x20)

int strcasecmp(const char *s1, const char *s2)
{
    const char * p1 = s1;
    const char * p2 = s2;
    char c1, c2;

    if (p1 == p2)
        return 0;

    do {
        c1 = TOLOWER(*p1++);
        c2 = TOLOWER(*p2++);
        if (c1 == '\0')
            break;
    } while (c1 == c2);
    return (int)(c1 - c2);
}


char *strtok(char * __restrict s1, const char * __restrict s2)
{
    static char *next_start;	/* Initialized to 0 since in bss. */
    return strtok_r(s1, s2, &next_start);
}

char *strtok_r(char * __restrict s1, const char * __restrict s2, char ** __restrict next_start)
{
    register char *s;
    register char *p;

    if (((s = s1) != NULL) || ((s = *next_start) != NULL)) {
        if (*(s += strspn(s, s2))) {
            if ((p = strpbrk(s, s2)) != NULL) {
                *p++ = 0;
            }
        } else {
            s = NULL;
            p = NULL;
        }
        *next_start = p;
    }
    return s;
}

size_t strspn(const char *s1, const char *s2)
{
    register const char *s = s1;
    register const char *p = s2;

    while (*p) {
        if (*p++ == *s) {
            ++s;
            p = s2;
        }
    }
    return s - s1;
}

char *strpbrk(const char *s1, const char *s2)
{
    register const char *s;
    register const char *p;

    for ( s=s1 ; *s ; s++ ) {
        for ( p=s2 ; *p ; p++ ) {
            if (*p == *s) return (char *) s; /* silence the warning */
        }
    }
    return NULL;
}

char *strcat(char * s1, const char * s2)
{
    register char *s = s1;

    while (*s++);
    --s;
    while ((*s++ = *s2++) != 0);

    return s1;
}

char *strstr (const char *phaystack, const char *pneedle)
{
    const unsigned char *haystack, *needle;
    char b;
    const unsigned char *rneedle;

    haystack = (const unsigned char *) phaystack;

    if ((b = *(needle = (const unsigned char *) pneedle))) {
        char c;
        haystack--;		/* possible ANSI violation */

        {
            char a;
            do
                if (!(a = *++haystack))
                    goto ret0;
            while (a != b);
        }

        if (!(c = *++needle))
            goto foundneedle;
        ++needle;
        goto jin;

        for (;;) {
            {
                char a;
                if (0)
jin: {
                    if ((a = *++haystack) == c)
                        goto crest;
                } else
                    a = *++haystack;
                do {
                    for (; a != b; a = *++haystack) {
                        if (!a)
                            goto ret0;
                        if ((a = *++haystack) == b)
                            break;
                        if (!a)
                            goto ret0;
                    }
                } while ((a = *++haystack) != c);
            }
crest: {
                char a;
                {
                    const unsigned char *rhaystack;
                    if (*(rhaystack = haystack-- + 1) == (a = *(rneedle = needle)))
                        do {
                            if (!a)
                                goto foundneedle;
                            if (*++rhaystack != (a = *++needle))
                                break;
                            if (!a)
                                goto foundneedle;
                        } while (*++rhaystack == (a = *++needle));
                    needle = rneedle;	/* took the register-poor aproach */
                }
                if (!a)
                    break;
            }
        }
    }
foundneedle:
    return (char *) haystack;
ret0:
    return 0;
}


char * strchr(const char *s, const int i)
{
    const char c = (char) i;

    do {
        if (*s == c) return (char *) s;
    } while (*s++);

    return NULL;
}
