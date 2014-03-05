#include "GPIO_device.h"
#include "GPIO_device_at91_pin_map.h"
#include "board.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#define	MODE_INPUT	0
#define MODE_OUTPUT	1

void gpio_irq_handler ( void );
xSemaphoreHandle xOnPushbutton;

static unsigned int GetGPIOBits(void){
	return AT91F_PIO_GetInput(AT91C_BASE_PIOA);
}

static void clear_GPIO_bits(unsigned int portBits){
	AT91F_PIO_ClearOutput( AT91C_BASE_PIOA, portBits );
}

static void set_GPIO_bits(unsigned int portBits){
	AT91F_PIO_SetOutput( AT91C_BASE_PIOA, portBits );
}

void GPIO_device_init_port(unsigned int port, unsigned int mode){
	unsigned int portMask = 0;
	switch(port){
		case 0:
			portMask = GPIO_1;
			break;
		case 1:
			portMask = GPIO_2;
			break;
		case 2:
			portMask = GPIO_3;
			break;
		default:
			break;
	}

	if (mode == MODE_INPUT){
		AT91F_PIO_CfgInput(AT91C_BASE_PIOA, portMask);
		AT91C_BASE_PIOA->PIO_PPUDR = portMask; //disable pullup
		AT91C_BASE_PIOA->PIO_IFER = portMask; //enable input filter
	}
	else{
		AT91F_PIO_CfgOutput( AT91C_BASE_PIOA, portMask );
		AT91F_PIO_ClearOutput( AT91C_BASE_PIOA, portMask );
	}
}

void GPIO_device_init_base(void){
	// Enable the peripheral clock.
	AT91F_PMC_EnablePeriphClock( AT91C_BASE_PMC, (1 << AT91C_ID_PIOA) | (1 << AT91C_ID_IRQ0));

	portENTER_CRITICAL();
    AT91PS_AIC     pAic;
	pAic = AT91C_BASE_AIC;

	AT91F_PIO_InterruptEnable(AT91C_BASE_PIOA,PIO_PUSHBUTTON_SWITCH);

	AT91F_AIC_ConfigureIt ( pAic, AT91C_ID_PIOA, PUSHBUTTON_INTERRUPT_LEVEL, AT91C_AIC_SRCTYPE_INT_HIGH_LEVEL, gpio_irq_handler);
	AT91F_AIC_EnableIt (pAic, AT91C_ID_PIOA);
	portEXIT_CRITICAL();

}

void GPIO_device_init_SD_card_IO(void){
	AT91F_PIO_CfgInput(AT91C_BASE_PIOA, SD_CARD_DETECT | SD_WRITE_PROTECT);
	AT91C_BASE_PIOA->PIO_PPUER = SD_CARD_DETECT | SD_WRITE_PROTECT; //enable pullup
	AT91C_BASE_PIOA->PIO_IFER = SD_CARD_DETECT | SD_WRITE_PROTECT; //enable input filter
}

int GPIO_device_is_SD_card_present(void){
	return (GetGPIOBits() & SD_CARD_DETECT) == 0;
}

int GPIO_device_is_SD_card_writable(void){
	return (GetGPIOBits() & SD_WRITE_PROTECT) == 0;
}

int GPIO_device_is_button_pressed(void){
	return (GetGPIOBits() & PIO_PUSHBUTTON_SWITCH) != 0;
}

void GPIO_device_set(unsigned int port, unsigned int state){
	unsigned int gpioBits = 0;
	switch (port){
		case 0:
			gpioBits = GPIO_1;
			break;
		case 1:
			gpioBits = GPIO_2;
			break;
		case 2:
			gpioBits = GPIO_3;
			break;
	}
	if (state){
		set_GPIO_bits(gpioBits);
	} else{
		clear_GPIO_bits(gpioBits);
	}
}

unsigned int GPIO_device_get(unsigned int port){
	unsigned int gpioStates = AT91F_PIO_GetInput(AT91C_BASE_PIOA);
	int value = 0;
	switch (port){
		case 0:
			value = ((gpioStates & GPIO_1) != 0);
			break;
		case 1:
			value = ((gpioStates & GPIO_2) != 0);
			break;
		case 2:
			value = ((gpioStates & GPIO_3) != 0);
			break;
		default:
			break;
	}
	return value;
}

void readGpios(unsigned int *gpio1, unsigned int *gpio2, unsigned int *gpio3){
	unsigned int gpioStates = AT91F_PIO_GetInput(AT91C_BASE_PIOA);
	*gpio1 = ((gpioStates & GPIO_1) != 0);
	*gpio2 = ((gpioStates & GPIO_2) != 0);
	*gpio3 = ((gpioStates & GPIO_3) != 0);
}


void GPIO_device_init_pushbutton(void){
	AT91F_PIO_CfgInput(AT91C_BASE_PIOA, PIO_PUSHBUTTON_SWITCH);
	AT91C_BASE_PIOA->PIO_PPUER = PIO_PUSHBUTTON_SWITCH; //enable pullup
	AT91C_BASE_PIOA->PIO_IFER = PIO_PUSHBUTTON_SWITCH; //enable input filter
	AT91C_BASE_PIOA->PIO_MDER = PIO_PUSHBUTTON_SWITCH; //enable multi drain
}
