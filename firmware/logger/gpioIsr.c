#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "Board.h"
#include "task.h"
#include "loggerPinDefs.h"

extern xSemaphoreHandle xOnPushbutton;
extern xSemaphoreHandle xOnGPI1;
extern xSemaphoreHandle xOnGPI2;
extern xSemaphoreHandle xOnGPI3;



void gpio_irq_handler(void )__attribute__ ((naked));
void gpio_irq_handler( void )
{
//* enable the next PIO IRQ
    int dummy = AT91C_BASE_PIOA->PIO_ISR;
    //* suppress the compilation warning
    dummy =dummy;
    	
	portENTER_SWITCHING_ISR();
	portCHAR xTaskWoken = pdFALSE;
	

	unsigned int pio_input = AT91F_PIO_GetInput(AT91C_BASE_PIOA);  
	if( (pio_input & PIO_PUSHBUTTON_SWITCH) == 0){
		xTaskWoken = xSemaphoreGiveFromISR( xOnPushbutton, xTaskWoken );
	}
	if( (pio_input & GPIO_1) == 0) {
		xTaskWoken = xSemaphoreGiveFromISR( xOnGPI1, xTaskWoken );
	}
	if( (pio_input & GPIO_2) == 0) {
		xTaskWoken = xSemaphoreGiveFromISR( xOnGPI2, xTaskWoken);	
	}
	if( (pio_input & GPIO_3) == 0) {
		xTaskWoken = xSemaphoreGiveFromISR( xOnGPI3, xTaskWoken);	
	}
	AT91F_AIC_AcknowledgeIt (AT91C_BASE_AIC);
	AT91F_AIC_ClearIt(AT91C_BASE_AIC,AT91C_ID_PIOA);
	portEXIT_SWITCHING_ISR( xTaskWoken );
}

