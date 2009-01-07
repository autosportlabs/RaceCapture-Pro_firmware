#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "Board.h"
#include "task.h"
#include "loggerHardware.h"

extern xSemaphoreHandle xOnPushbutton;


void pushbutton_irq_handler(void )__attribute__ ((naked));
void pushbutton_irq_handler( void )
{
//* enable the next PIO IRQ
    int dummy = AT91C_BASE_PIOA->PIO_ISR;
    //* suppress the compilation warning
    dummy =dummy;
    	
	portENTER_SWITCHING_ISR();
	portCHAR xTaskWoken = pdFALSE;
	
	xTaskWoken = xSemaphoreGiveFromISR( xOnPushbutton, xTaskWoken );

	AT91F_AIC_AcknowledgeIt (AT91C_BASE_AIC);
	AT91F_AIC_ClearIt(AT91C_BASE_AIC,AT91C_ID_PIOA);
	portEXIT_SWITCHING_ISR( xTaskWoken );
}

