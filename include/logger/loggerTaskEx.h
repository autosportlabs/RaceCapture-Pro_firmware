/*
 * loggerTaskEx.h
 *
 *  Created on: Mar 3, 2012
 *      Author: brent
 */

#ifndef LOGGERTASKEX_H_
#define LOGGERTASKEX_H_

#include "loggerNotifications.h"

int isLogging();
void startLogging();
void stopLogging();

void startLoggerTaskEx();
void loggerTaskEx(void *params);

#endif /* LOGGERTASKEX_H_ */
