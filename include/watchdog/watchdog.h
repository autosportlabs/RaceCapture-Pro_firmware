/*
 * watchdog.h
 *
 *  Created on: Feb 7, 2014
 *      Author: brent
 */

#ifndef WATCHDOG_H_
#define WATCHDOG_H_

void watchdog_reset();
void watchdog_init(int timeoutMs);
int watchdog_is_watchdog_reset();
int watchdog_is_poweron_reset();

#endif /* WATCHDOG_H_ */
