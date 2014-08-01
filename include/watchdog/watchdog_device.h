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
int watchdog_device_is_watchdog_reset();
int watchdog_device_is_poweron_reset();
#endif /* WATCHDOG_DEVICE_H_ */
