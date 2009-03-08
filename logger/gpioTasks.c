#include "gpioTasks.h"
#include "loggerPinDefs.h"
#include "board.h"
#include "lib_AT91SAM7S256.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"


#define DEBOUNCE_DELAY_PERIOD			( ( portTickType )30 / portTICK_RATE_MS  )
#define GPIO_TASK_PRIORITY 			( tskIDLE_PRIORITY + 4 )
#define GPIO_TASK_STACK_SIZE		50

void gpio_irq_handler ( void );

xSemaphoreHandle xOnPushbutton;
xSemaphoreHandle xOnGPI1;
xSemaphoreHandle xOnGPI2;
xSemaphoreHandle xOnGPI3;

extern xSemaphoreHandle g_xLoggerStart;
extern int g_loggingShouldRun;

void createGPIOTasks(){
	
	vSemaphoreCreateBinary( xOnPushbutton );
	vSemaphoreCreateBinary( xOnGPI1 );
	vSemaphoreCreateBinary( xOnGPI2 );
	vSemaphoreCreateBinary( xOnGPI3 );

	xTaskCreate( onPushbuttonTask, 	( signed portCHAR * ) "PushbuttonTask", 	GPIO_TASK_STACK_SIZE, 	NULL, 	GPIO_TASK_PRIORITY, 	NULL );
	xTaskCreate( onGPI1Task, 		( signed portCHAR * ) "GPI1Task", 			GPIO_TASK_STACK_SIZE, 	NULL, 	GPIO_TASK_PRIORITY, 	NULL );
	xTaskCreate( onGPI2Task,	 	( signed portCHAR * ) "GPI2Task", 			GPIO_TASK_STACK_SIZE, 	NULL, 	GPIO_TASK_PRIORITY, 	NULL );
	xTaskCreate( onGPI3Task,	 	( signed portCHAR * ) "GPI3Task", 			GPIO_TASK_STACK_SIZE, 	NULL, 	GPIO_TASK_PRIORITY, 	NULL );
	
	portENTER_CRITICAL();
    AT91PS_AIC     pAic;
	pAic = AT91C_BASE_AIC;
	AT91F_PIO_CfgInput(AT91C_BASE_PIOA, ENABLED_GPIO_PINS);
	AT91F_PIO_CfgPullup(AT91C_BASE_PIOA, ENABLED_GPIO_PINS);
	AT91F_PIO_CfgInputFilter(AT91C_BASE_PIOA,ENABLED_GPIO_PINS);
	AT91F_PIO_CfgOpendrain(AT91C_BASE_PIOA,ENABLED_GPIO_PINS);
	AT91F_PIO_InterruptEnable(AT91C_BASE_PIOA,ENABLED_GPIO_PINS);

	AT91F_AIC_ConfigureIt ( pAic, AT91C_ID_PIOA, PUSHBUTTON_INTERRUPT_LEVEL, AT91C_AIC_SRCTYPE_INT_HIGH_LEVEL, gpio_irq_handler);
	AT91F_AIC_EnableIt (pAic, AT91C_ID_PIOA);
	portEXIT_CRITICAL();

	
}


void onPushbuttonTask(void *pvParameters){
	
	portTickType xDelayPeriod = DEBOUNCE_DELAY_PERIOD;
	
	while(1){
		if ( xSemaphoreTake(xOnPushbutton, portMAX_DELAY) == pdTRUE){
			vTaskDelay( xDelayPeriod );
			if ((AT91F_PIO_GetInput(AT91C_BASE_PIOA) & PIO_PUSHBUTTON_SWITCH) == 0){
				if (g_loggingShouldRun){
					g_loggingShouldRun = 0;	//stop logging
				}
				else{
					xSemaphoreGive(g_xLoggerStart);	//start logging					
				}
			}
		}
	}
}

void onGPI1Task(void *params){
	while(1){
		if ( xSemaphoreTake(xOnGPI1, portMAX_DELAY) == pdTRUE){
		}
	}
}

void onGPI2Task(void *params){
	while(1){
		if ( xSemaphoreTake(xOnGPI2, portMAX_DELAY) == pdTRUE){
		}
	}
}

void onGPI3Task(void *params){
	while(1){
		if ( xSemaphoreTake(xOnGPI3, portMAX_DELAY) == pdTRUE){
		}
	}
}
