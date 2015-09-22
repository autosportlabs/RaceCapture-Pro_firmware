/*
 * watchdog.h
 *
 *  Created on: Feb 7, 2014
 *      Author: brent
 */

#ifndef WATCHDOG_H_
#define WATCHDOG_H_
#include <stdbool.h>

void watchdog_reset(void);
void watchdog_init(int timeoutMs);
bool watchdog_is_watchdog_reset(void);
bool watchdog_is_poweron_reset(void);

#endif /* WATCHDOG_H_ */
