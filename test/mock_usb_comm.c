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


#include "usb_comm.h"

#define BUFFER_SIZE 256 * 2

int usb_comm_init()
{
    return 1;
}

void usb_init_serial(Serial *serial)
{
    serial->init = &usb_init;
    serial->flush = &usb_flush;
    serial->get_c = &usb_getchar;
    serial->get_c_wait = &usb_getcharWait;
    serial->get_line = &usb_readLine;
    serial->get_line_wait = &usb_readLineWait;
    serial->put_c = &usb_putchar;
    serial->put_s = &usb_puts;
}

void usb_init(unsigned int bits, unsigned int parity, unsigned int stopBits, unsigned int baud) {}

void usb_flush(void) {}

int usb_getcharWait(char *c, size_t delay)
{
    return 0;
}

char usb_getchar(void)
{
    char c;
    return usb_getcharWait(&c, 0);
}

int usb_readLine(char *s, int len)
{
    return usb_readLineWait(s,len,0);
}

int usb_readLineWait(char *s, int len, size_t delay)
{
    return 0;
}

void usb_puts(const char *s)
{

}

void usb_putchar(char c)
{

}

void onUSBCommTask(void *pvParameters)
{
}
