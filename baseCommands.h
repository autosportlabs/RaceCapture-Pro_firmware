/*
 * baseCommands.h
 *
 *  Created on: Jul 23, 2011
 *      Author: brent
 */

#ifndef BASECOMMANDS_H_
#define BASECOMMANDS_H_

#include "constants.h"

void InitBaseCommands();

void TestCommand(unsigned char argc, char **argv);

void ShowTaskInfo(unsigned char argc, char **argv);

#endif /* BASECOMMANDS_H_ */
