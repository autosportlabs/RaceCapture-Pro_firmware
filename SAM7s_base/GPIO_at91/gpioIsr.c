#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "board.h"
#include "task.h"
#include "GPIO_device_at91_pin_map.h"

extern xSemaphoreHandle xOnPushbutton;


/* The ISR can cause a context switch so is declared naked. */
void gpio_irq_handler( void ) __attribute__ ((naked));
void gpio_irq_handler( void )
{

    portENTER_SWITCHING_ISR();

//* enable the next PIO IRQ
    int dummy = AT91C_BASE_PIOA->PIO_ISR;
    //* suppress the compilation warning
    dummy =dummy;
    portCHAR xTaskWoken = pdFALSE;

    unsigned int pio_input = AT91C_BASE_PIOA->PIO_PDSR;
    if( (pio_input & PIO_PUSHBUTTON_SWITCH) == 0) {
        xTaskWoken = xSemaphoreGiveFromISR( xOnPushbutton, xTaskWoken );
    }
    /* Clear AIC to complete ISR processing */
    AT91C_BASE_AIC->AIC_EOICR = 0;
    portEXIT_SWITCHING_ISR( xTaskWoken );
}

