/*
 * loggerCommands.h
 *
 *  Created on: Jul 24, 2011
 *      Author: brent
 */

#ifndef LOGGERCOMMANDS_H_
#define LOGGERCOMMANDS_H_

void InitLoggerCommands();
void ExecLuaCommand(unsigned char argc, char **argv);
void WriteScriptPage(unsigned char argc, char **argv);
void ReadScriptPage(unsigned char argc, char **argv);





#endif /* LOGGERCOMMANDS_H_ */
