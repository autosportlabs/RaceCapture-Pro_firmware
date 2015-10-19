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

#include "FreeRTOS.h"
#include "board.h"
#include "printk.h"
#include "queue.h"
#include "task.h"
#include "usart_device.h"
#include "usart_device_at91.h"

#define GPS_BAUDRATE 38400
#define TELEMETRY_BAUDRATE 115200
#define USART_INTERRUPT_LEVEL 5

#define GPS_RX_QUEUE_LEN	512
#define GPS_TX_QUEUE_LEN	512
#define TELEM_RX_QUEUE_LEN	512
#define TELEM_TX_QUEUE_LEN	8

void usart0_irq_handler (void);
void usart1_irq_handler (void);

static struct txrx_queue usart_txrx_queues[2];

struct txrx_queue* get_txrx_queue(const int idx) {
        return &usart_txrx_queues[idx];
}

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

static int init_txrx_queue(const int idx,
                           const size_t tx_len,
                           const size_t rx_len)
{
        const unsigned portBASE_TYPE char_size =
                (unsigned portBASE_TYPE) sizeof(signed portCHAR);

        usart_txrx_queues[idx].tx = xQueueCreate(tx_len, char_size);
        usart_txrx_queues[idx].rx = xQueueCreate(rx_len, char_size);

        return usart_txrx_queues[idx].tx && usart_txrx_queues[idx].rx;
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

    /* Init device 0 (Telemetry) */
    if (!init_txrx_queue(0, TELEM_TX_QUEUE_LEN, TELEM_RX_QUEUE_LEN))
            return 0;
    usart_device_init_0(8, 0, 1, TELEMETRY_BAUDRATE);

    /* Init device 1 (GPS) */
    if (!init_txrx_queue(1, GPS_TX_QUEUE_LEN, GPS_RX_QUEUE_LEN))
            return 0;
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

static void _usart_flush(const int idx)
{
        char rx;
        while(xQueueReceive(usart_txrx_queues[idx].rx, &rx, 0));
}

void usart0_flush(void)
{
        _usart_flush(0);
}

void usart1_flush(void)
{
        _usart_flush(1);
}

static int _usart_get_char_wait(const int idx, char *c, size_t delay)
{
        return xQueueReceive(usart_txrx_queues[idx].rx, c, delay) == pdTRUE;
}

int usart0_getcharWait(char *c, size_t delay)
{
        return _usart_get_char_wait(0, c, delay);
}

int usart1_getcharWait(char *c, size_t delay)
{
        return _usart_get_char_wait(1, c, delay);
}

char usart0_getchar()
{
        char c;
        _usart_get_char_wait(0, &c, portMAX_DELAY);
        return c;
}

char usart1_getchar()
{
        char c;
        _usart_get_char_wait(1, &c, portMAX_DELAY);
        return c;
}

static void _usart_putchar(const int idx, char c)
{
        char buf[2];
        buf[0] = c;
        buf[1] = '\0';
        pr_debug(buf);

        xQueueSend(usart_txrx_queues[idx].tx, &c, portMAX_DELAY);

        //Enable transmitter interrupt
        AT91F_US_EnableIt(AT91C_BASE_US0, AT91C_US_TXRDY | AT91C_US_RXRDY);
}


void usart0_putchar(char c)
{
        _usart_putchar(0, c);
}

void usart1_putchar(char c)
{
        _usart_putchar(1, c);
}

void usart0_puts (const char* s )
{
        while(*s)
                _usart_putchar(0, *s++);
}

void usart1_puts (const char* s )
{
        while(*s)
                _usart_putchar(1, *s++);
}

static int _usart_read_line_wait(const int idx, char *s, int len, size_t delay)
{
        int count = 0;

        while(count < len - 1) {
                char c;
                if (!_usart_get_char_wait(idx, &c, delay))
                        break;
                *s++ = c;
                count++;
                if ('\n' == c)
                        break;
        }

        *s = '\0';
        return count;
}

int usart0_readLineWait(char *s, int len, size_t delay)
{
        return _usart_read_line_wait(0, s, len, delay);
}

int usart0_readLine(char *s, int len)
{
        return _usart_read_line_wait(0, s, len, portMAX_DELAY);
}

int usart1_readLineWait(char *s, int len, size_t delay)
{
        return _usart_read_line_wait(1, s, len, delay);
}

int usart1_readLine(char *s, int len)
{
        return _usart_read_line_wait(1, s, len, portMAX_DELAY);
}
