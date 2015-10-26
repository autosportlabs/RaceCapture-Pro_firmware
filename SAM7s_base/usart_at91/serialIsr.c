/*
 * Race Capture Pro Firmware
 *
 * Copyright (C) 2015 Autosport Labs
 *
 * This file is part of the Race Capture Pro fimrware suite
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

#include "FreeRTOS.h"
#include "board.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"
#include "usart_device_at91.h"

/* The ISR can cause a context switch so is declared naked. */
void usart0_irq_handler( void ) __attribute__ ((naked));
void usart0_irq_handler( void )
{

    portENTER_SWITCHING_ISR();

    unsigned portLONG ulStatus;
    signed portCHAR cChar;
    portBASE_TYPE xTaskWokenByTx = pdFALSE, xTaskWokenByPost = pdFALSE;

    /* What caused the interrupt? */
    ulStatus = AT91C_BASE_US0->US_CSR &= AT91C_BASE_US0->US_IMR;

    if( ulStatus & AT91C_US_TXRDY ) {
        /* The interrupt was caused by the THR becoming empty.  Are there any
        more characters to transmit? */
            if( xQueueReceiveFromISR(get_txrx_queue(0)->tx, &cChar,
                                     &xTaskWokenByTx ) == pdTRUE ) {
            /* A character was retrieved from the queue so can be sent to the
            THR now. */
            AT91C_BASE_US0->US_THR = cChar;
        } else {
            /* Queue empty, nothing to send so turn off the Tx interrupt. */
            AT91F_US_DisableIt( AT91C_BASE_US0, AT91C_US_TXRDY );
        }
    }

    if( ulStatus & AT91C_US_RXRDY ) {
        /* The interrupt was caused by a character being received.  Grab the
        character from the RHR and place it in the queue or received
        characters. */
        cChar = AT91C_BASE_US0->US_RHR;
        xTaskWokenByPost = xQueueSendFromISR(get_txrx_queue(0)->rx, &cChar,
                                             xTaskWokenByPost );
    }

    /* If a task was woken by either a character being received or a character
    being transmitted then we may need to switch to another task. */

    /* End the interrupt in the AIC. */
    AT91C_BASE_AIC->AIC_EOICR = 0;

    portEXIT_SWITCHING_ISR( ( xTaskWokenByPost || xTaskWokenByTx ) );
}



/* The ISR can cause a context switch so is declared naked. */
void usart1_irq_handler( void ) __attribute__ ((naked));
void usart1_irq_handler( void )
{

    portENTER_SWITCHING_ISR();

    unsigned portLONG ulStatus;
    signed portCHAR cChar;
    portBASE_TYPE xTaskWokenByTx = pdFALSE, xTaskWokenByPost = pdFALSE;

    /* What caused the interrupt? */
    ulStatus = AT91C_BASE_US1->US_CSR &= AT91C_BASE_US1->US_IMR;

    if( ulStatus & AT91C_US_TXRDY ) {
        /* The interrupt was caused by the THR becoming empty.  Are there any
        more characters to transmit? */

            if( xQueueReceiveFromISR(get_txrx_queue(1)->tx, &cChar,
                                     &xTaskWokenByTx ) == pdTRUE ) {
            /* A character was retrieved from the queue so can be sent to the
            THR now. */
            AT91C_BASE_US1->US_THR = cChar;
        } else {
            /* Queue empty, nothing to send so turn off the Tx interrupt. */
            AT91F_US_DisableIt( AT91C_BASE_US1, AT91C_US_TXRDY );
        }
    }

    if( ulStatus & AT91C_US_RXRDY ) {
        /* The interrupt was caused by a character being received.  Grab the
        character from the RHR and place it in the queue or received
        characters. */
        cChar = AT91C_BASE_US1->US_RHR;
        xTaskWokenByPost = xQueueSendFromISR(get_txrx_queue(1)->rx, &cChar,
                                             xTaskWokenByPost );
    }

    /* If a task was woken by either a character being received or a character
    being transmitted then we may need to switch to another task. */

    /* End the interrupt in the AIC. */
    AT91C_BASE_AIC->AIC_EOICR = 0;

    portEXIT_SWITCHING_ISR( xTaskWokenByPost || xTaskWokenByTx );

}
