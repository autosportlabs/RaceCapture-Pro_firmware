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

#endif /* WATCHDOG_H_ */
