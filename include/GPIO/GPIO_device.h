/*
 * GPIO_device.h
 *
 *  Created on: Feb 7, 2014
 *      Author: brent
 */

#ifndef GPIO_DEVICE_H_
#define GPIO_DEVICE_H_
#include <stdint.h>
#include <loggerConfig.h>
int GPIO_device_init(LoggerConfig *loggerConfig);
unsigned int GPIO_device_get(unsigned int port);
void GPIO_device_set(unsigned int port, unsigned int state);
int GPIO_device_is_button_pressed(void);

#endif /* GPIO_DEVICE_H_ */
