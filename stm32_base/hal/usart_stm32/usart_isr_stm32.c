#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"
#include "stm32f4xx_usart.h"

extern xQueueHandle xUsart0Tx; 
extern xQueueHandle xUsart0Rx;
extern xQueueHandle xUsart1Tx; 
extern xQueueHandle xUsart1Rx;


void USART1_IRQHandler( void ){

	portBASE_TYPE xTaskWokenByTx = pdFALSE, xTaskWokenByPost = pdFALSE;
	signed portCHAR cChar;

	if (USART_GetITStatus(USART1, USART_IT_TXE) != RESET)
	{
		/* The interrupt was caused by the TX becoming empty.  Are there any more characters to transmit? */
		if( xQueueReceiveFromISR( xUsart0Tx, &cChar, &xTaskWokenByTx ) == pdTRUE )
		{
			// A character was retrieved from the queue so can be sent to the USART
			USART_SendData(USART1, cChar);
		}
		else
		{
			/* Queue empty, nothing to send so turn off the Tx interrupt. */
			USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
		}		
	}

	if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
	{
		/* The interrupt was caused by a character being received.  Grab the
		character from the rx and place it in the queue or received
		characters. */
		cChar = USART_ReceiveData(USART1);
		xQueueSendFromISR( xUsart0Rx, &cChar, &xTaskWokenByPost );
	}

	/* If a task was woken by either a character being received or a character 
	being transmitted then we may need to switch to another task. */
	portEND_SWITCHING_ISR( xTaskWokenByPost || xTaskWokenByTx );
}

void USART3_IRQHandler( void ){

	portBASE_TYPE xTaskWokenByTx = pdFALSE, xTaskWokenByPost = pdFALSE;
	signed portCHAR cChar;

	if (USART_GetITStatus(USART3, USART_IT_TXE) != RESET)
	{
		/* The interrupt was caused by the TX becoming empty.  Are there any more characters to transmit? */
		if( xQueueReceiveFromISR( xUsart1Tx, &cChar, &xTaskWokenByTx ) == pdTRUE )
		{
			// A character was retrieved from the queue so can be sent to the USART
			USART_SendData(USART3, cChar);
		}
		else
		{
			/* Queue empty, nothing to send so turn off the Tx interrupt. */
			USART_ITConfig(USART3, USART_IT_TXE, DISABLE);
		}		
	}

	if (USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
	{
		/* The interrupt was caused by a character being received.  Grab the
		character from the rx and place it in the queue or received
		characters. */
		cChar = USART_ReceiveData(USART3);
		xQueueSendFromISR( xUsart1Rx, &cChar, &xTaskWokenByPost );
	}

	/* If a task was woken by either a character being received or a character 
	being transmitted then we may need to switch to another task. */
	portEND_SWITCHING_ISR( xTaskWokenByPost || xTaskWokenByTx );
}
