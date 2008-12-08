#include "gpio.h"
#include "Board.h"
#include "task.h"
#include "usb_comm.h"
#include "led_debug.h"
#include "gpio_pins.h"

#define DEBOUNCE_DELAY_PERIOD			( ( portTickType )30 / portTICK_RATE_MS  )

#define ENABLED_GPIO_PINS  PIO_PUSHBUTTON_SWITCH | PIO_GPI_1 | PIO_GPI_2 | PIO_GPI_3

xSemaphoreHandle xOnPushbutton;
xSemaphoreHandle xOnGPI1;
xSemaphoreHandle xOnGPI2;
xSemaphoreHandle xOnGPI3;

#define PUSHBUTTON_INTERRUPT_LEVEL	6
#define GPI1_INTERRUPT_LEVEL 5
#define GPI2_INTERRUPT_LEVEL 4
#define GPI3_INTERRUPT_LEVEL 3

void setGPO(int gpo, int state){
	
}

int readGPI(int gpi){
	
	return 0;
}

void onGPO1Task(void *pvParameters){

	portTickType xDelayPeriod = ( ( portTickType ) 2000 / portTICK_RATE_MS  );
	
	portENTER_CRITICAL();
    AT91PS_AIC     pAic;
	pAic = AT91C_BASE_AIC;
	AT91F_PIO_CfgOutput(AT91C_BASE_PIOA, PIO_GPI_1);
	portEXIT_CRITICAL();

	while(1){
	
		AT91F_PIO_SetOutput( AT91C_BASE_PIOA, PIO_GPI_1 );
		vTaskDelay(xDelayPeriod);	
		AT91F_PIO_ClearOutput( AT91C_BASE_PIOA, PIO_GPI_1 );
		vTaskDelay(xDelayPeriod);
		
	}	
}

void onGPO2Task(void *pvParameters){

	portTickType xDelayPeriod = ( ( portTickType ) 2000 / portTICK_RATE_MS  );
	
	portENTER_CRITICAL();
    AT91PS_AIC     pAic;
	pAic = AT91C_BASE_AIC;
	AT91F_PIO_CfgOutput(AT91C_BASE_PIOA, PIO_GPI_2);
	portEXIT_CRITICAL();

	while(1){
	
		AT91F_PIO_SetOutput( AT91C_BASE_PIOA, PIO_GPI_2 );
		vTaskDelay(xDelayPeriod);	
		AT91F_PIO_ClearOutput( AT91C_BASE_PIOA, PIO_GPI_2 );
		vTaskDelay(xDelayPeriod);
		
	}	
}
void onGPO3Task(void *pvParameters){

	portTickType xDelayPeriod = ( ( portTickType ) 2000 / portTICK_RATE_MS  );
	
	portENTER_CRITICAL();
    AT91PS_AIC     pAic;
	pAic = AT91C_BASE_AIC;
	AT91F_PIO_CfgOutput(AT91C_BASE_PIOA, PIO_GPI_3);
	portEXIT_CRITICAL();

	while(1){
		AT91F_PIO_SetOutput( AT91C_BASE_PIOA, PIO_GPI_3 );
		vTaskDelay(xDelayPeriod);	
		AT91F_PIO_ClearOutput( AT91C_BASE_PIOA, PIO_GPI_3 );
		vTaskDelay(xDelayPeriod);
	}	
}

void onGPI1Task(void *pvParameters){

}

void onGPI2Task(void *pvParameters){

}

void onGPI3Task(void *pvParameters){

}

void onPushbuttonTask(void *pvParameters){
	
	portTickType xDelayPeriod = DEBOUNCE_DELAY_PERIOD;
	
	portENTER_CRITICAL();
	vSemaphoreCreateBinary( xOnPushbutton );
    AT91PS_AIC     pAic;
	pAic = AT91C_BASE_AIC;
	AT91F_PIO_CfgInput(AT91C_BASE_PIOA, ENABLED_GPIO_PINS);
	AT91F_PIO_CfgPullup(AT91C_BASE_PIOA, ENABLED_GPIO_PINS);
	AT91F_PIO_CfgInputFilter(AT91C_BASE_PIOA,ENABLED_GPIO_PINS);
	AT91F_PIO_CfgOpendrain(AT91C_BASE_PIOA,ENABLED_GPIO_PINS);
	
	AT91F_PIO_InterruptEnable(AT91C_BASE_PIOA,ENABLED_GPIO_PINS);

	AT91F_AIC_ConfigureIt ( pAic, AT91C_ID_PIOA, PUSHBUTTON_INTERRUPT_LEVEL, AT91C_AIC_SRCTYPE_INT_HIGH_LEVEL, pushbutton_irq_handler);
	AT91F_AIC_EnableIt (pAic, AT91C_ID_PIOA);

	portEXIT_CRITICAL();
	
	while(1){
		if ( xSemaphoreTake(xOnPushbutton, portMAX_DELAY) == pdTRUE){
				vTaskDelay( xDelayPeriod );
				
				if (
					((AT91F_PIO_GetInput(AT91C_BASE_PIOA) & PIO_PUSHBUTTON_SWITCH) == 0) )
					{
						Toggle_LED(LED1);
					}

				if ( ((AT91F_PIO_GetInput(AT91C_BASE_PIOA) & PIO_GPI_1) == 0) ) 
				{
					//SendString("Pushbutton\n");
					Toggle_LED(LED2);
				}
		}
	}
}
