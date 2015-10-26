#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "board.h"
#include "task.h"

extern xQueueHandle xUsart0Tx;
extern xQueueHandle xUsart0Rx;
extern xQueueHandle xUsart1Tx;
extern xQueueHandle xUsart1Rx;


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
        if( xQueueReceiveFromISR( xUsart0Tx, &cChar, &xTaskWokenByTx ) == pdTRUE ) {
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
        xTaskWokenByPost = xQueueSendFromISR( xUsart0Rx, &cChar, xTaskWokenByPost );
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
        if( xQueueReceiveFromISR( xUsart1Tx, &cChar, &xTaskWokenByTx ) == pdTRUE ) {
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
        xTaskWokenByPost = xQueueSendFromISR( xUsart1Rx, &cChar, xTaskWokenByPost );
    }

    /* If a task was woken by either a character being received or a character
    being transmitted then we may need to switch to another task. */

    /* End the interrupt in the AIC. */
    AT91C_BASE_AIC->AIC_EOICR = 0;

    portEXIT_SWITCHING_ISR( xTaskWokenByPost || xTaskWokenByTx );

}
