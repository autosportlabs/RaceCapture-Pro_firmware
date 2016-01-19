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

#ifndef MOCKSERIAL_H_
#define MOCKSERIAL_H_

#include "cpp_guard.h"
#include "serial.h"

CPP_GUARD_BEGIN

void mock_flush(void);

int mock_get_c_wait(char *c, size_t delay);

char mock_get_c();

void mock_put_c(char c);

void mock_put_s(const char* s );

int mock_get_line_wait(char *s, int len, size_t delay);

int mock_get_line(char *s, int len);

void setupMockSerial();

Serial * getMockSerial();

void mock_setRxBuffer(const char *src);

char * mock_getTxBuffer();

void mock_appendRxBuffer(const char *src);

void mock_resetTxBuffer();

CPP_GUARD_END

#endif /* MOCKSERIAL_H_ */
