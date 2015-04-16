/*
 * loggerTaskEx.h
 *
 *  Created on: Mar 3, 2012
 *      Author: brent
 */

#ifndef LOGGERTASKEX_H_
#define LOGGERTASKEX_H_

#include "loggerNotifications.h"
#include <stdbool.h>
#include <stdint.h>

void startLogging();
void stopLogging();

void startLoggerTaskEx( int priority);
void loggerTaskEx(void *params);

#endif /* LOGGERTASKEX_H_ */
