/*
 * logging.cpp
 *
 *  Created on: May 20, 2009
 *      Author: brent
 */
#include "logging.h"

bool loggingVerbose = true;

void EnableVerbose(bool enabled){
	loggingVerbose = enabled;
}

