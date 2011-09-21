/*
 * baseCommands.h
 *
 *  Created on: Jul 23, 2011
 *      Author: brent
 */

#ifndef BASECOMMANDS_H_
#define BASECOMMANDS_H_

#include "constants.h"


#define BASE_COMMANDS \
		{"showTasks", "Show status of running tasks", "", ShowTaskInfo}


void ShowTaskInfo(unsigned int argc, char **argv);

#endif /* BASECOMMANDS_H_ */
