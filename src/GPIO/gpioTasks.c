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
#include "taskUtil.h"
#include "printk.h"

#define DEBOUNCE_DELAY_PERIOD		30
#define GPIO_TASK_STACK_SIZE		50

xSemaphoreHandle xOnPushbutton;

void startGPIOTasks(int priority){
	
	vSemaphoreCreateBinary( xOnPushbutton );
	xSemaphoreTake(xOnPushbutton, 0);
	xTaskCreate( onPushbuttonTask, 	( signed portCHAR * ) "PushbuttonTask", 	GPIO_TASK_STACK_SIZE, 	NULL, 	priority, 	NULL );
}


void onPushbuttonTask(void *pvParameters){
	
	while(1){
		if ( xSemaphoreTake(xOnPushbutton, portMAX_DELAY) == pdTRUE){
			delayMs(DEBOUNCE_DELAY_PERIOD);

			if (GPIO_device_is_button_pressed()){
				pr_info("button pressed\r\n");
				if (isLogging()){
					pr_info("logging stop\r\n");
					stopLogging();
				}
				else{
					pr_info("logging start\r\n");
					startLogging();
				}
			}
		}
	}
}
