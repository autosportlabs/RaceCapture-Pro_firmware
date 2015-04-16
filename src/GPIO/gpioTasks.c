#include "gpioTasks.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "loggerTaskEx.h"
#include "logger.h"
#include "taskUtil.h"
#include "printk.h"
#include "GPIO.h"

#define DEBOUNCE_DELAY_PERIOD		30
#define GPIO_TASK_STACK_SIZE		50

xSemaphoreHandle xOnPushbutton;

void startGPIOTasks(int priority){
	vSemaphoreCreateBinary( xOnPushbutton );
	xTaskCreate( onPushbuttonTask, 	( signed portCHAR * ) "PushbuttonTask", 	GPIO_TASK_STACK_SIZE, 	NULL, 	priority, 	NULL );
}


void onPushbuttonTask(void *pvParameters){
	
	while(1){
		if ( xSemaphoreTake(xOnPushbutton, portMAX_DELAY) == pdTRUE){
			delayMs(DEBOUNCE_DELAY_PERIOD);

			if (GPIO_is_button_pressed()){
				if (logging_is_active()){
					stopLogging();
				}
				else{
					startLogging();
				}
			}
		}
	}
}
