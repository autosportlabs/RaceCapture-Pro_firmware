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

#include "usart.h"

void InitSerial()
{
  unsigned int mode =
                        AT91C_US_CLKS_CLOCK
                        | AT91C_US_CHRL_8_BITS
                        | AT91C_US_PAR_NONE
                        | AT91C_US_NBSTOP_1_BIT
                        | AT91C_US_CHMODE_NORMAL;


 	AT91F_PIO_CfgPeriph(AT91C_BASE_PIOA, 
			AT91C_PA6_TXD0, 				// mux function A
			0); // mux funtion B

	AT91F_PIO_CfgPeriph(AT91C_BASE_PIOA, 
			AT91C_PA5_RXD0, 				// mux function A
			0); // mux funtion B  

    // Configure the USART in the desired mode @115200 bauds
    USART_Configure(AT91C_BASE_US0, mode, 115200, BOARD_MCK);

    // Enable receiver & transmitter
    USART_SetTransmitterEnabled(AT91C_BASE_US0, 1);
    USART_SetReceiverEnabled(AT91C_BASE_US0, 1);




 	AT91F_PIO_CfgPeriph(AT91C_BASE_PIOA, 
			AT91C_PA22_TXD1, 				// mux function A
			0); // mux funtion B

	AT91F_PIO_CfgPeriph(AT91C_BASE_PIOA, 
			AT91C_PA21_RXD1, 				// mux function A
			0); // mux funtion B  

    // Configure the USART in the desired mode @115200 bauds
    USART_Configure(AT91C_BASE_US1, mode, 115200, BOARD_MCK);

    // Enable receiver & transmitter
    USART_SetTransmitterEnabled(AT91C_BASE_US1, 1);
    USART_SetReceiverEnabled(AT91C_BASE_US1, 1);
}

//------------------------------------------------------------------------------
//         Exported functions
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
/// Configures an USART peripheral with the specified parameters.
/// \param usart  Pointer to the USART peripheral to configure.
/// \param mode  Desired value for the USART mode register (see the datasheet).
/// \param baudrate  Baudrate at which the USART should operate (in Hz).
/// \param masterClock  Frequency of the system master clock (in Hz).
//------------------------------------------------------------------------------
void USART_Configure(AT91S_USART *usart,
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

//------------------------------------------------------------------------------
/// Enables or disables the transmitter of an USART peripheral.
/// \param usart  Pointer to an USART peripheral
/// \param enabled  If true, the transmitter is enabled; otherwise it is
///                 disabled.
//------------------------------------------------------------------------------
void USART_SetTransmitterEnabled(AT91S_USART *usart,
                                        unsigned char enabled)
{
    if (enabled) {

        usart->US_CR = AT91C_US_TXEN;
    }
    else {

        usart->US_CR = AT91C_US_TXDIS;
    }
}

//------------------------------------------------------------------------------
/// Enables or disables the receiver of an USART peripheral
/// \param usart  Pointer to an USART peripheral
/// \param enabled  If true, the receiver is enabled; otherwise it is disabled.
//------------------------------------------------------------------------------
void USART_SetReceiverEnabled(AT91S_USART *usart,
                                     unsigned char enabled)
{
    if (enabled) {

        usart->US_CR = AT91C_US_RXEN;
    }
    else {

        usart->US_CR = AT91C_US_RXDIS;
    }
}



int uart_putc(AT91S_USART *pUSART,int ch) 
{
	while (!(pUSART->US_CSR & AT91C_US_TXRDY));   /* Wait for Empty Tx Buffer */
	return (pUSART->US_THR = ch);                 /* Transmit Character */
}	

int uart0_putchar (int ch) {      
  return uart_putc(AT91C_BASE_US0, ch );
}

int uart0_puts (char* s )
{
	while ( *s ) uart0_putchar(*s++ );
	return 0;
}

int uart1_putchar (int ch)
{            
  return uart_putc(AT91C_BASE_US1, ch );
}

int uart1_puts (char* s )
{
	while ( *s ) uart1_putchar(*s++ );
	return 0;
}

//------------------------------------------------------------------------------
/// Sends one packet of data through the specified USART peripheral. This
/// function operates synchronously, so it only returns when the data has been
/// actually sent.
/// \param usart  Pointer to an USART peripheral.
/// \param data  Data to send including 9nth bit and sync field if necessary (in
///              the same format as the US_THR register in the datasheet).
/// \param timeOut  Time out value (0 = no timeout).
//------------------------------------------------------------------------------
void USART_Write(
    AT91S_USART *usart,
    unsigned short data,
    volatile unsigned int timeOut)
{
    if (timeOut == 0) {

        while ((usart->US_CSR & AT91C_US_TXEMPTY) == 0);
    }
    else {

        while ((usart->US_CSR & AT91C_US_TXEMPTY) == 0) {

            if (timeOut == 0) {

                return;
            }
            timeOut--;
        }
    }

    usart->US_THR = data;
}

//------------------------------------------------------------------------------
/// Sends the contents of a data buffer through the specified USART peripheral.
/// This function returns immediately (1 if the buffer has been queued, 0
/// otherwise); poll the ENDTX and TXBUFE bits of the USART status register
/// to check for the transfer completion.
/// \param usart  Pointer to an USART peripheral.
/// \param buffer  Pointer to the data buffer to send.
/// \param size  Size of the data buffer (in bytes).
//------------------------------------------------------------------------------
unsigned char USART_WriteBuffer(
    AT91S_USART *usart,
    void *buffer,
    unsigned int size)
{
    // Check if the first PDC bank is free
    if ((usart->US_TCR == 0) && (usart->US_TNCR == 0)) {

        usart->US_TPR = (unsigned int) buffer;
        usart->US_TCR = size;
        usart->US_PTCR = AT91C_PDC_TXTEN;

        return 1;
    }
    // Check if the second PDC bank is free
    else if (usart->US_TNCR == 0) {

        usart->US_TNPR = (unsigned int) buffer;
        usart->US_TNCR = size;

        return 1;
    }
    else {

        return 0;
    }
}

//------------------------------------------------------------------------------
/// Reads and return a packet of data on the specified USART peripheral. This
/// function operates asynchronously, so it waits until some data has been
/// received.
/// \param usart  Pointer to an USART peripheral.
/// \param timeOut  Time out value (0 -> no timeout).
//------------------------------------------------------------------------------
unsigned short USART_Read(
    AT91S_USART *usart,
    volatile unsigned int timeOut)
{
    if (timeOut == 0) {

        while ((usart->US_CSR & AT91C_US_RXRDY) == 0);
    }
    else {

        while ((usart->US_CSR & AT91C_US_RXRDY) == 0) {

            if (timeOut == 0) {

                return 0;
            }
            timeOut--;
        }
    }

    return usart->US_RHR;
}

//------------------------------------------------------------------------------
/// Reads data from an USART peripheral, filling the provided buffer until it
/// becomes full. This function returns immediately with 1 if the buffer has
/// been queued for transmission; otherwise 0.
/// \param usart  Pointer to an USART peripheral.
/// \param buffer  Pointer to the buffer where the received data will be stored.
/// \param size  Size of the data buffer (in bytes).
//------------------------------------------------------------------------------
unsigned char USART_ReadBuffer(AT91S_USART *usart,
                                      void *buffer,
                                      unsigned int size)
{
    // Check if the first PDC bank is free
    if ((usart->US_RCR == 0) && (usart->US_RNCR == 0)) {

        usart->US_RPR = (unsigned int) buffer;
        usart->US_RCR = size;
        usart->US_PTCR = AT91C_PDC_RXTEN;

        return 1;
    }
    // Check if the second PDC bank is free
    else if (usart->US_RNCR == 0) {

        usart->US_RNPR = (unsigned int) buffer;
        usart->US_RNCR = size;

        return 1;
    }
    else {

        return 0;
    }
}

//------------------------------------------------------------------------------
/// Returns 1 if some data has been received and can be read from an USART;
/// otherwise returns 0.
/// \param usart  Pointer to an AT91S_USART instance.
//------------------------------------------------------------------------------
unsigned char USART_IsDataAvailable(AT91S_USART *usart)
{
    if ((usart->US_CSR & AT91C_US_RXRDY) != 0) {

        return 1;
    }
    else {

        return 0;
    }
}

