/**
 * Race Capture Firmware
 *
 * Copyright (C) 2014 Autosport Labs
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
 *
 * Authors: Stieg
 */

#include "ff.h"


FRESULT f_sync (FIL* fp)
{
        return FR_OK;
}

FRESULT f_close (FIL* fp)
{
        return FR_OK;
}

FRESULT f_open(FIL* fp,
               const TCHAR* path,
               BYTE mode)
{
        return FR_OK;
}

int f_puts(const TCHAR* str,
           FIL* fp)
{
        return 0;
}

FRESULT f_write (
    FIL* fp,			/* Pointer to the file object */
    const void *buff,	/* Pointer to the data to be written */
    UINT btw,			/* Number of bytes to write */
    UINT* bw			/* Pointer to number of bytes written */
)
{
        return FR_OK;
}

FRESULT f_lseek (
    FIL* fp,		/* Pointer to the file object */
    DWORD ofs		/* File pointer from top of file */
)
{
        return FR_OK;
}
