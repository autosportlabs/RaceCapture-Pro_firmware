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

#ifndef USART_DEVICE_H
#define USART_DEVICE_H

#include "cpp_guard.h"
#include "serial.h"
#include "usart.h"

#include <stddef.h>

CPP_GUARD_BEGIN

int usart_device_init();

void usart_device_config(uint8_t port, uint8_t bits, uint8_t parity, uint8_t stopbits, uint32_t baud);

int usart_device_init_serial(Serial *serial, uart_id_t id);

void usart_device_init_0(unsigned int bits, unsigned int parity, unsigned int stopBits, unsigned int baud);

void usart_device_init_1(unsigned int bits, unsigned int parity, unsigned int stopBits, unsigned int baud);

void usart_device_init_2(unsigned int bits, unsigned int parity, unsigned int stopBits, unsigned int baud);

void usart_device_init_3(unsigned int bits, unsigned int parity, unsigned int stopBits, unsigned int baud);

void usart0_flush(void);

char usart0_getchar();

int usart0_getcharWait(char *c, size_t delay);

void usart0_putchar(char c);

void usart0_puts (const char* s );

int usart0_readLine(char *s, int len);

int usart0_readLineWait(char *s, int len, size_t delay);


void usart1_flush(void);

char usart1_getchar();

int usart1_getcharWait(char *c, size_t delay);

void usart1_putchar(char c);

void usart1_puts (const char* s );

int usart1_readLine(char *s, int len);

int usart1_readLineWait(char *s, int len, size_t delay);


void usart2_flush(void);

char usart2_getchar();

int usart2_getcharWait(char *c, size_t delay);

void usart2_putchar(char c);

void usart2_puts (const char* s );

int usart2_readLine(char *s, int len);

int usart2_readLineWait(char *s, int len, size_t delay);


void usart3_flush(void);

char usart3_getchar();

int usart3_getcharWait(char *c, size_t delay);

void usart3_putchar(char c);

void usart3_puts (const char* s );

int usart3_readLine(char *s, int len);

int usart3_readLineWait(char *s, int len, size_t delay);

CPP_GUARD_END

#endif //#ifndef USART_DEVICE_H
