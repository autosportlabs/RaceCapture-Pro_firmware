#include "GPIO_device.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#define	MODE_INPUT	0
#define MODE_OUTPUT	1

void gpio_irq_handler ( void );

static unsigned int GetGPIOBits(void){
	return 0;

}

static void clear_GPIO_bits(unsigned int portBits){
}

static void set_GPIO_bits(unsigned int portBits){
}

void GPIO_device_init_port(unsigned int port, unsigned int mode){

}

void GPIO_device_init_base(void){

}

void GPIO_device_init_SD_card_IO(void){
}

int GPIO_device_is_SD_card_present(void){
	return 1;
}

int GPIO_device_is_SD_card_writable(void){
	return 1;
}

int GPIO_device_is_button_pressed(void){
	return 0;
}

void GPIO_device_set(unsigned int port, unsigned int state){

}

unsigned int GPIO_device_get(unsigned int port){
	return 0;
}

void readGpios(unsigned int *gpio1, unsigned int *gpio2, unsigned int *gpio3){
}


void GPIO_device_init_pushbutton(void){
}
