#include "gpioTasks.h"
#include "board.h"
#include "lib_AT91SAM7S256.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "loggerHardware.h"
#include "loggerTaskEx.h"
#include "GPIO_device.h"

#define DEBOUNCE_DELAY_PERIOD		( ( portTickType )30 / portTICK_RATE_MS  )
#define GPIO_TASK_STACK_SIZE		50

xSemaphoreHandle xOnPushbutton;

void startGPIOTasks(int priority){
	
	vSemaphoreCreateBinary( xOnPushbutton );
	

	xTaskCreate( onPushbuttonTask, 	( signed portCHAR * ) "PushbuttonTask", 	GPIO_TASK_STACK_SIZE, 	NULL, 	priority, 	NULL );

}


void onPushbuttonTask(void *pvParameters){
	
	portTickType xDelayPeriod = DEBOUNCE_DELAY_PERIOD;
	
	while(1){
		if ( xSemaphoreTake(xOnPushbutton, portMAX_DELAY) == pdTRUE){
			vTaskDelay( xDelayPeriod );
			if (GPIO_device_is_button_pressed()){
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
