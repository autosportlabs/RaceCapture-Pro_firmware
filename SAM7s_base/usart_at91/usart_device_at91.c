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
//         Headers
//------------------------------------------------------------------------------

#include "usart_device.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "printk.h"
#include "board.h"

#define GPS_BAUDRATE 38400
#define TELEMETRY_BAUDRATE 115200
#define USART_INTERRUPT_LEVEL 5

void usart0_irq_handler (void);
void usart1_irq_handler (void);

xQueueHandle xUsart0Tx;
xQueueHandle xUsart0Rx;
xQueueHandle xUsart1Tx;
xQueueHandle xUsart1Rx;

static void USART_Configure(AT91S_USART *usart,
                            unsigned int mode,
                            unsigned int baudrate,
                            unsigned int masterClock)
{
    // Reset and disable receiver & transmitter
    usart->US_CR = AT91C_US_RSTRX | AT91C_US_RSTTX
                   | AT91C_US_RXDIS | AT91C_US_TXDIS;

    // Configure mode
    usart->US_MR = mode;

    // Configure baudrate
    // Asynchronous, no oversampling
    if (((mode & AT91C_US_SYNC) == 0)
        && ((mode & AT91C_US_OVER) == 0)) {

        usart->US_BRGR = (masterClock / baudrate) / 16;
    }
    // TODO other modes
}

static int initQueues()
{

    int success = 1;

    /* Create the queues used to hold Rx and Tx characters. */
    /* Telemetry USART */
    xUsart0Rx = xQueueCreate(512, ( unsigned portBASE_TYPE ) sizeof( signed portCHAR ) );
    xUsart0Tx = xQueueCreate(512, ( unsigned portBASE_TYPE ) sizeof( signed portCHAR ) );

    /* GPS USART */
    xUsart1Rx = xQueueCreate(300, ( unsigned portBASE_TYPE ) sizeof( signed portCHAR ) );
    xUsart1Tx = xQueueCreate(300, ( unsigned portBASE_TYPE ) sizeof( signed portCHAR ) );

    if (xUsart0Rx == NULL ||
        xUsart1Rx == NULL ||
        xUsart0Rx == NULL ||
        xUsart0Rx == NULL
       ) success = 0;

    return success;
}

static unsigned int createUartMode(unsigned int bits, unsigned int parity, unsigned int stopBits)
{

    switch (bits) {
    case 5:
        bits = AT91C_US_CHRL_5_BITS;
        break;
    case 6:
        bits = AT91C_US_CHRL_6_BITS;
        break;
    case 7:
        bits = AT91C_US_CHRL_7_BITS;
        break;
    case 8:
    default:
        bits = AT91C_US_CHRL_8_BITS;
        break;
    }

    switch (parity) {
    case 1:
        parity = AT91C_US_PAR_EVEN;
        break;
    case 2:
        parity = AT91C_US_PAR_ODD;
        break;
    case 0:
    default:
        parity = AT91C_US_PAR_NONE;
    }

    switch (stopBits) {
    case 2:
        stopBits = AT91C_US_NBSTOP_2_BIT;
        break;
    case 1:
    default:
        stopBits = AT91C_US_NBSTOP_1_BIT;
        break;
    }

    return AT91C_US_CLKS_CLOCK | bits | parity | stopBits | AT91C_US_CHMODE_NORMAL;
}

int usart_device_init()
{
    AT91F_PMC_EnablePeriphClock( AT91C_BASE_PMC,(1 << AT91C_ID_US0) | (1 << AT91C_ID_US1));
    if (!initQueues()) return 0;
    usart_device_init_0(8, 0, 1, TELEMETRY_BAUDRATE);
    usart_device_init_1(8, 0, 1, GPS_BAUDRATE);
    return 1;
}

int usart_device_init_serial(Serial *serial, uart_id_t id)
{
    int rc = 1;
    switch(id) {
    case UART_TELEMETRY:
        serial->init = &usart_device_init_0;
        serial->flush = &usart0_flush;
        serial->get_c = &usart0_getchar;
        serial->get_c_wait = &usart0_getcharWait;
        serial->get_line = &usart0_readLine;
        serial->get_line_wait = &usart0_readLineWait;
        serial->put_c = &usart0_putchar;
        serial->put_s = &usart0_puts;
        break;
    case UART_GPS:
        serial->init = &usart_device_init_1;
        serial->flush = &usart1_flush;
        serial->get_c = &usart1_getchar;
        serial->get_c_wait = &usart1_getcharWait;
        serial->get_line = &usart1_readLine;
        serial->get_line_wait = &usart1_readLineWait;
        serial->put_c = &usart1_putchar;
        serial->put_s = &usart1_puts;
        break;
    default:
        rc = 0;
        break;
    }
    return rc;
}

void usart_device_init_0(unsigned int bits, unsigned int parity, unsigned int stopBits, unsigned int baud)
{

    unsigned int mode = createUartMode(bits, parity, stopBits);

    //Enable USART0

    AT91F_PIO_CfgPeriph(AT91C_BASE_PIOA,
                        AT91C_PA6_TXD0, 				// mux function A
                        0); // mux funtion B

    AT91F_PIO_CfgPeriph(AT91C_BASE_PIOA,
                        AT91C_PA5_RXD0, 				// mux function A
                        0); // mux funtion B

    // Configure the USART in the desired mode @115200 bauds
    USART_Configure(AT91C_BASE_US0, mode, baud, BOARD_MCK);

    // Enable receiver & transmitter
    AT91C_BASE_US0->US_CR = AT91C_US_TXEN;
    AT91C_BASE_US0->US_CR = AT91C_US_RXEN;

    /* Enable the Rx interrupts.  The Tx interrupts are not enabled
    until there are characters to be transmitted. */
    AT91F_US_EnableIt( AT91C_BASE_US0, AT91C_US_RXRDY );

    /* Enable the interrupts in the AIC. */
    AT91F_AIC_ConfigureIt( AT91C_BASE_AIC, AT91C_ID_US0, USART_INTERRUPT_LEVEL, AT91C_AIC_SRCTYPE_EXT_LOW_LEVEL, usart0_irq_handler );
    AT91F_AIC_EnableIt( AT91C_BASE_AIC, AT91C_ID_US0 );

}

void usart_device_config(uint8_t port, uint8_t bits, uint8_t parity, uint8_t stopbits, uint32_t baud)
{
    switch(port) {
    case 0:
        usart_device_init_0(bits, parity, stopbits, baud);
        break;
    case 1:
        usart_device_init_1(bits, parity, stopbits, baud);
        break;
    default:
        break;
    }
}

void usart_device_init_1(unsigned int bits, unsigned int parity, unsigned int stopBits, unsigned int baud)
{

    unsigned int mode = createUartMode(bits, parity, stopBits);

    AT91F_PIO_CfgPeriph(AT91C_BASE_PIOA,
                        AT91C_PA22_TXD1, 				// mux function A
                        0); // mux funtion B

    AT91F_PIO_CfgPeriph(AT91C_BASE_PIOA,
                        AT91C_PA21_RXD1, 				// mux function A
                        0); // mux funtion B

    // Configure the USART in the desired mode @115200 bauds
    USART_Configure(AT91C_BASE_US1, mode, baud, BOARD_MCK);

    // Enable receiver & transmitter
    AT91C_BASE_US1->US_CR = AT91C_US_TXEN;
    AT91C_BASE_US1->US_CR = AT91C_US_RXEN;

    /* Enable the Rx interrupts.  The Tx interrupts are not enabled
    until there are characters to be transmitted. */
    AT91F_US_EnableIt( AT91C_BASE_US1, AT91C_US_RXRDY );

    /* Enable the interrupts in the AIC. */
    AT91F_AIC_ConfigureIt( AT91C_BASE_AIC, AT91C_ID_US1, USART_INTERRUPT_LEVEL, AT91C_AIC_SRCTYPE_EXT_LOW_LEVEL, usart1_irq_handler );
    AT91F_AIC_EnableIt( AT91C_BASE_AIC, AT91C_ID_US1 );
}


void usart0_flush(void)
{
    char rx;
    while(xQueueReceive( xUsart0Rx, &rx, 0 ));
}

void usart1_flush(void)
{
    char rx;
    while(xQueueReceive( xUsart1Rx, &rx, 0 ));
}

int usart0_getcharWait(char *c, size_t delay)
{
    return xQueueReceive( xUsart0Rx, c, delay ) == pdTRUE ? 1 : 0;
}

int usart1_getcharWait(char *c, size_t delay)
{
    return xQueueReceive( xUsart1Rx, c, delay ) == pdTRUE ? 1 : 0;
}

char usart0_getchar()
{
    char c;
    usart0_getcharWait(&c, portMAX_DELAY);
    return c;
}

char usart1_getchar()
{
    char c;
    return usart1_getcharWait(&c, portMAX_DELAY);
    return c;
}

void usart0_putchar(char c)
{
    if (TRACE_LEVEL) {
        char buf[2];
        buf[0] = c;
        buf[1] = '\0';
        pr_debug(buf);
    }
    xQueueSend( xUsart0Tx, &c, portMAX_DELAY );
    //Enable transmitter interrupt
    AT91F_US_EnableIt( AT91C_BASE_US0, AT91C_US_TXRDY | AT91C_US_RXRDY );
}

void usart1_putchar(char c)
{
    if (TRACE_LEVEL) {
        char buf[2];
        buf[0] = c;
        buf[1] = '\0';
        pr_debug(buf);
    }
    xQueueSend( xUsart1Tx, &c, portMAX_DELAY );
    //Enable transmitter interrupt
    AT91F_US_EnableIt( AT91C_BASE_US1, AT91C_US_TXRDY | AT91C_US_RXRDY );
}

void usart0_puts (const char* s )
{
    while ( *s ) usart0_putchar(*s++ );
}

void usart1_puts (const char* s )
{
    while ( *s ) usart1_putchar(*s++ );
}

int usart0_readLineWait(char *s, int len, size_t delay)
{
    int count = 0;
    while(count < len - 1) {
        char c = 0;
        if (!usart0_getcharWait(&c, delay)) break;
        *s++ = c;
        count++;
        if (c == '\n') break;
    }
    *s = '\0';
    return count;
}

int usart0_readLine(char *s, int len)
{
    return usart0_readLineWait(s,len,portMAX_DELAY);
}

int usart1_readLineWait(char *s, int len, size_t delay)
{
    int count = 0;
    while(count < len - 1) {
        char c = 0;
        if (!usart1_getcharWait(&c, delay)) break;
        *s++ = c;
        count++;
        if (c == '\n') break;
    }
    *s = '\0';
    return count;
}

int usart1_readLine(char *s, int len)
{
    return usart1_readLineWait(s,len,portMAX_DELAY);
}
