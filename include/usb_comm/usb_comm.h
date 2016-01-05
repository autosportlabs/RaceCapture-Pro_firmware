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

#ifndef USB_COMM_H_
#define USB_COMM_H_

#include "cpp_guard.h"
#include "serial.h"

#include <stddef.h>

CPP_GUARD_BEGIN

void usb_init_serial(Serial *serial);

void startUSBCommTask(int priority);

void onUSBCommTask(void *);

void usb_init(unsigned int bits, unsigned int parity, unsigned int stopBits, unsigned int baud);

void usb_flush(void);

int usb_readLine(char *s, int len);

int usb_readLineWait(char *s, int len, size_t delay);

char usb_getchar(void);

int usb_getcharWait(char *c, size_t delay);

void usb_putchar(char c);

void usb_puts(const char* s );

CPP_GUARD_END

#endif /*USB_COMM_H_*/
