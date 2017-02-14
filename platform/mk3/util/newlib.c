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

#ifdef USE_ITM
#include <platform_debug.h>
#else
#include <usart.h>
#endif

#include "mem_mang.h"
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/unistd.h>

#undef errno
extern int errno;

char *__env[1] = { 0 };
char **environ = __env;

int _write(int file, char *ptr, int len);

void _fini()
{
}

void _exit(int status)
{
    (void)status;
    while (1);
}

int _open(const char *filename, int flags, int mode )
{
    return -1;
}

int _close(int file)
{
    (void)file;
    return -1;
}

int _execve(char *name, char **argv, char **env)
{
    (void)name;
    (void)argv;
    (void)env;
    errno = ENOMEM;
    return -1;
}

int _fork()
{
    errno = EAGAIN;
    return -1;
}

int _fstat(int file, struct stat *st)
{
    (void)file;
    (void)st;
    st->st_mode = S_IFCHR;
    return 0;
}

int _getpid()
{
    return 1;
}

int _gettimeofday(struct timeval *tv, struct timezone *tz)
{
    (void)tv;
    (void)tz;
    return 0;
}

int _isatty(int file)
{
    switch (file) {
    case STDOUT_FILENO:
    case STDERR_FILENO:
    case STDIN_FILENO:
        return 1;
    default:
        errno = EBADF;
        return 0;
    }
}

int _kill(int pid, int sig)
{
    (void)pid;
    (void)sig;
    errno = EINVAL;
    return (-1);
}

int _link(char *old, char *new)
{
    (void)old;
    (void)new;
    errno = EMLINK;
    return -1;
}

int _lseek(int file, int ptr, int dir)
{
    (void)file;
    (void)ptr;
    (void)dir;
    return 0;
}

#if 0
caddr_t _sbrk(int incr)
{
    extern char _ebss;
    static char *heap_end;
    char *prev_heap_end;

    if (heap_end == 0) {
        heap_end = &_ebss;
    }
    prev_heap_end = heap_end;

    char * stack = (char*) __get_MSP();
    if (heap_end + incr > stack) {
        _write(STDERR_FILENO, "Heap and stack collision\n", 25);
        errno = ENOMEM;
        return (caddr_t) - 1;
        //abort ();
    }

    heap_end += incr;
    return (caddr_t) prev_heap_end;

}
#endif

int _read(int file, char *ptr, int len)
{
    char c = 0x00;

    /* TODO: remove this when we build in support */
    (void)len;
    switch (file) {
    case STDIN_FILENO:
#ifdef USE_ITM
        /* TODO */
#else
        /* TODO */
#endif
        *ptr++ = c;
        return 1;
        break;
    default:
        errno = EBADF;
        return -1;
    }
}

int _stat(const char *filepath, struct stat *st)
{
    (void)filepath;
    (void)st;
    st->st_mode = S_IFCHR;
    return 0;
}

clock_t _times(struct tms *buf)
{
    (void)buf;
    return -1;
}

int _unlink(char *name)
{
    (void)name;
    errno = ENOENT;
    return -1;
}

int _wait(int *status)
{
    (void)status;
    errno = ECHILD;
    return -1;
}

int _write(int file, char *ptr, int len)
{
    int n;
    char c;
    switch (file) {
    case STDOUT_FILENO: /*stdout*/
    case STDERR_FILENO: /* stderr */
        for (n = 0; n < len; n++) {
            c = (uint8_t)*ptr++;
#ifdef USE_ITM
            ITM_SendChar(c);
#else
            /* TODO: Add usart support */
#endif
        }
        break;
    default:
        errno = EBADF;
        return -1;
    }
    return len;
}

void* malloc(size_t size)
{
        return portMalloc(size);
}

void free(void *ptr)
{
        return portFree(ptr);
}

void* realloc(void *ptr, size_t new_size)
{
        return portRealloc(ptr, new_size);
}

void* calloc(size_t count, size_t elmnt_size)
{
        const size_t size = count * elmnt_size;
        void *val = portMalloc(size);
        if (val)
                memset(val, 0, size);

        return val;
}

/* My reentrant unsafe methods to get shit working */
void* _malloc_r(struct _reent *reent, size_t size)
{
        return malloc(size);
}

void* _calloc_r(struct _reent *reent, size_t count, size_t elmnt_size)
{
        return calloc(count, elmnt_size);
}

void _free_r(struct _reent *reent, void *data)
{
        return free(data);
}

void* _realloc_r(struct _reent *reent, void *data, size_t new_size)
{
        return realloc(data, new_size);
}
