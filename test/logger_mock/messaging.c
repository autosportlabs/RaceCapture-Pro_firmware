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

/*
 * This cheap harlet of a stub is only here to get my WiFi bits
 * to compile.  This is merely a band-aid in the name of progress.
 * Some good soul will eliminate it down the road and replace it
 * with full code unit testing....
 */

#include "serial.h"
#include <stddef.h>

int process_read_msg(struct Serial *serial, char *buff, size_t len)
{
        return 0;
}

void process_msg(struct Serial *serial, char * buffer, size_t bufferSize) {}
