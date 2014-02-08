/*
 * watchdog_device.h
 *
 *  Created on: Feb 7, 2014
 *      Author: brent
 */

#ifndef WATCHDOG_DEVICE_H_
#define WATCHDOG_DEVICE_H_

void watchdog_device_reset();
void watchdog_device_init(int timeoutMs);

#endif /* WATCHDOG_DEVICE_H_ */
