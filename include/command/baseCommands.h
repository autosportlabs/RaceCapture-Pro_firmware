/*
 * baseCommands.h
 *
 *  Created on: Jul 23, 2011
 *      Author: brent
 */

#ifndef BASECOMMANDS_H_
#define BASECOMMANDS_H_

#include "constants.h"
#include "serial.h"


#define BASE_COMMANDS \
		{"showTasks", "Show status of running tasks", "", ShowTaskInfo}, \
		{"version", "Gets the version numbers", "", GetVersion}, \
		{"showStats", "Info on system statistics.","", ShowStats}, \
		{"sysReset", "Reset the system", "", ResetSystem}




void ShowTaskInfo(Serial *serial, unsigned int argc, char **argv);
void GetVersion(Serial *serial, unsigned int argc, char **argv);
void ShowStats(Serial *serial, unsigned int argc, char **argv);
void ResetSystem(Serial *serial, unsigned int argc, char **argv);

#endif /* BASECOMMANDS_H_ */
