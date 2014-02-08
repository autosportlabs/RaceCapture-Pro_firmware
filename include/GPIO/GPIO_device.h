/*
 * GPIO_device.h
 *
 *  Created on: Feb 7, 2014
 *      Author: brent
 */

#ifndef GPIO_DEVICE_H_
#define GPIO_DEVICE_H_

void GPIO_device_init_port(unsigned int port, unsigned int mode);
void GPIO_device_init_SD_card_IO(void);
void GPIO_device_init_pushbutton(void);
void GPIO_device_init_events(void);

unsigned int GPIO_device_get(unsigned int port);
void GPIO_device_set(unsigned int port, unsigned int state);

int GPIO_device_is_SD_card_present(void);
int GPIO_device_is_SD_card_writable(void);
int GPIO_device_is_button_pressed(void);

#endif /* GPIO_DEVICE_H_ */
