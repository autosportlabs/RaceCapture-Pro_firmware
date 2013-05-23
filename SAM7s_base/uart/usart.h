/* ----------------------------------------------------------------------------
 *         ATMEL Microcontroller Software Support 
 * ----------------------------------------------------------------------------
 * Copyright (c) 2008, Atmel Corporation
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the disclaimer below.
 *
 * Atmel's name may not be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * DISCLAIMER: THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ----------------------------------------------------------------------------
 */

//------------------------------------------------------------------------------
/// \dir
/// !Purpose
/// 
/// This module provides several definitions and methods for using an USART
/// peripheral.
///
/// !Usage
/// -# Enable the USART peripheral clock in the PMC.
/// -# Enable the required USART PIOs (see pio.h).
/// -# Configure the UART by calling USART_Configure.
/// -# Enable the transmitter and/or the receiver of the USART using
///    USART_SetTransmitterEnabled and USART_SetReceiverEnabled.
/// -# Send data through the USART using the USART_Write and
///    USART_WriteBuffer methods.
/// -# Receive data from the USART using the USART_Read and
///    USART_ReadBuffer functions; the availability of data can be polled
///    with USART_IsDataAvailable.
/// -# Disable the transmitter and/or the receiver of the USART with
///    USART_SetTransmitterEnabled and USART_SetReceiverEnabled.
//------------------------------------------------------------------------------

#ifndef USART_H
#define USART_H
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"


#define USART_MODE_8N1 AT91C_US_CLKS_CLOCK | AT91C_US_CHRL_8_BITS | AT91C_US_PAR_NONE | AT91C_US_NBSTOP_1_BIT | AT91C_US_CHMODE_NORMAL

//------------------------------------------------------------------------------
//         Headers
//------------------------------------------------------------------------------

#include <board.h>

//------------------------------------------------------------------------------
//         Definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/// \page "USART modes"
/// This page lists several common operating modes for an USART peripheral.
/// 
/// !Modes
/// - USART_MODE_ASYNCHRONOUS

/// Basic asynchronous mode, i.e. 8 bits no parity.
#define USART_MODE_ASYNCHRONOUS     (AT91C_US_CHRL_8_BITS | AT91C_US_PAR_NONE)
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//         Exported functions
//------------------------------------------------------------------------------

int initUsart();

void initUsart0(unsigned int mode, unsigned int baud);

void initUsart1(unsigned int mode, unsigned int baud);

void usart0_flush(void);

char usart0_getcharWait(portTickType delay);

char usart0_getchar();

int usart0_putchar(char c);

int usart0_puts (const char* s );

int usart0_readLine(char *s, int len);

int usart0_readLineWait(char *s, int len, portTickType delay);

char usart1_getcharWait(portTickType delay);

char usart1_getchar();

int usart1_putchar(char c);

int usart1_puts (const char* s );

int usart1_readLine(char *s, int len);

int usart1_readLineWait(char *s, int len, portTickType delay);

void usart1_flush(void);

#endif //#ifndef USART_H
