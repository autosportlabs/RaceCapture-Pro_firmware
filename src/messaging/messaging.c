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


#include "messaging.h"
#include "mod_string.h"
#include "serial.h"
#include "printk.h"

static int lockedApiMode = 0;

void initMessaging()
{
    init_command();
    initApi();
}

void process_msg(Serial *serial, char * buffer, size_t bufferSize)
{
    if (lockedApiMode) {
        read_line(serial, buffer, bufferSize);
        if (buffer[0] =='\0') {
            lockedApiMode = 0;
            show_command_prompt(serial);
        } else {
            process_api(serial, buffer, bufferSize);
        }
    } else {
        interactive_read_line(serial, buffer, bufferSize);
        if (buffer[0] == '{') {
            lockedApiMode = 1;
            process_api(serial, buffer, bufferSize);
        } else {
            if (strlen(buffer) == 0) {
                show_welcome(serial);
            } else {
                int res = process_command(serial, buffer, bufferSize);
                if (res != COMMAND_OK) {
                    serial->put_s("Unknown Command- Press Enter for Help.");
                    put_crlf(serial);
                }
            }
            show_command_prompt(serial);
        }
    }
}
