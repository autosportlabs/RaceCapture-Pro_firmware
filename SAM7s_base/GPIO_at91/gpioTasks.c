#include "gpioTasks.h"
#include "board.h"
#include "lib_AT91SAM7S256.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "loggerHardware.h"
#include "loggerTaskEx.h"
#include "GPIO_device_at91_pin_map.h"

#define DEBOUNCE_DELAY_PERIOD		( ( portTickType )30 / portTICK_RATE_MS  )
#define GPIO_TASK_PRIORITY 			( tskIDLE_PRIORITY + 4 )
#define GPIO_TASK_STACK_SIZE		50

void gpio_irq_handler ( void );

xSemaphoreHandle xOnPushbutton;

void createGPIOTasks(){
	
	vSemaphoreCreateBinary( xOnPushbutton );
	
	portENTER_CRITICAL();
    AT91PS_AIC     pAic;
	pAic = AT91C_BASE_AIC;

	AT91F_PIO_InterruptEnable(AT91C_BASE_PIOA,PIO_PUSHBUTTON_SWITCH);

	AT91F_AIC_ConfigureIt ( pAic, AT91C_ID_PIOA, PUSHBUTTON_INTERRUPT_LEVEL, AT91C_AIC_SRCTYPE_INT_HIGH_LEVEL, gpio_irq_handler);
	AT91F_AIC_EnableIt (pAic, AT91C_ID_PIOA);
	portEXIT_CRITICAL();

	xTaskCreate( onPushbuttonTask, 	( signed portCHAR * ) "PushbuttonTask", 	GPIO_TASK_STACK_SIZE, 	NULL, 	GPIO_TASK_PRIORITY, 	NULL );

}


void onPushbuttonTask(void *pvParameters){
	
	portTickType xDelayPeriod = DEBOUNCE_DELAY_PERIOD;
	
	while(1){
		if ( xSemaphoreTake(xOnPushbutton, portMAX_DELAY) == pdTRUE){
			vTaskDelay( xDelayPeriod );
			if ((AT91F_PIO_GetInput(AT91C_BASE_PIOA) & PIO_PUSHBUTTON_SWITCH) == 0){
				if (isLogging()){
					stopLogging();
				}
				else{
					startLogging();
				}
			}
		}
	}
}
