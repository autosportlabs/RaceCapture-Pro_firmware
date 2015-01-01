/*
 * loggerCommands.h
 *
 *  Created on: Jul 24, 2011
 *      Author: brent
 */

#ifndef LOGGERCOMMANDS_H_
#define LOGGERCOMMANDS_H_

#include "command.h"

#define LOGGER_COMMANDS \
{"resetConfig", "Resets All configuration Data to factory default", "", ResetConfig}, \
{"testSD", "Test Write to SD card.","<lineWrites> <periodicFlush> <quietMode>", TestSD}, \
\
{"startTerminal", "Starts a debugging terminal session on the specified port.","<port> <baud> [echo 1|0]", StartTerminal },\
{"viewLog", "Prints out logging messages to the terminal as they happen", "", ViewLog },\
{"setLogLevel", "Sets the log level", "<level>", SetLogLevel },\
{"logGpsData", "Enables logging of raw GPS data from the GPS Mouse", "<1|0>", LogGpsData }

void ResetConfig(Serial *serial, unsigned int argc, char **argv);
void TestSD(Serial *serial, unsigned int argc, char **argv);

void StartTerminal(Serial *serial, unsigned int argc, char **argv);
void ViewLog(Serial *serial, unsigned int argc, char **argv);
void SetLogLevel(Serial *serial, unsigned int argc, char **argv);
void LogGpsData(Serial *serial, unsigned int argc, char **argv);


#endif /* LOGGERCOMMANDS_H_ */
