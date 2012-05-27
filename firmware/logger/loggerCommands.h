/*
 * loggerCommands.h
 *
 *  Created on: Jul 24, 2011
 *      Author: brent
 */

#ifndef LOGGERCOMMANDS_H_
#define LOGGERCOMMANDS_H_

#include "usb_comm.h"
#include "command.h"

extern const struct cmd_t g_loggerCommands[];

#define LOGGER_COMMANDS \
{"testSD", "Test Write to SD card.","<lineWrites>", TestSD}, \
{"flashLoggerCfg", "Write the active Logger Configuration to flash.","", FlashLoggerConfig}, \
\
{"getOutputConfig", "Gets the Output Configuration","",GetLoggerOutputConfig}, \
{"setOutputConfig", "Sets the Output Configuration","[sdLoggingMode] [telemetryMode] [p2pDestAddrHigh] [p2pDestAddrLow]",SetLoggerOutputConfig}, \
\
{"getAnalogConfig", "Gets the configuration for the specified analog channel","<channel>", GetAnalogConfig}, \
{"setAnalogConfig", "Sets the configuration for the specified analog channel","<channel> [label] [units] [sampleRate] [loggingPrecision] [scalingMode] [linearScaling] < [mapRaw_0] [mapRaw_1] [mapRaw_2] [mapRaw_3] [mapRaw_4] [mapScaled_0] [mapScaled_1] [mapScaled_2] [mapScaled_3] [mapScaled_4] >",SetAnalogConfig}, \
\
{"getPwmConfig","Gets the configuration for the specified PWM/Analog Output channel.","<channel>",GetPwmConfig}, \
{"setPwmConfig","Sets the configuration for the specified PWM/Analog Output channel.","<channel> [label] [units] [sampleRate] [loggingPrecision] [outputMode] [loggingMode] [startupDutyCycle] [startupPeriod] [voltageScaling]",SetPwmConfig}, \
\
{"setPwmClockFreq", "Sets the PWM clock frequency for the specified channel.","<frequency>", SetPwmClockFreq}, \
{"getPwmClockFreq", "Gets the PWM clock frequency for the specified channel.","", GetPwmClockFreq}, \
\
{"getGpsConfig", "Gets the configuration for Gps features","",GetGpsConfig}, \
{"setGpsConfig", "Sets the configuration for Gps features","[installed] [label_lat] [units_lat] [sampleRate_lat] [label_long] [units_long] [sampleRate_long] [label_vel] [units_vel] [sampleRate_vel] [label_time] [units_time] [sampleRate_time] [label_qual] [units_qual] [sampleRate_qual] [label_sats] [units_sats] [sampleRate_sats]",SetGpsConfig}, \
\
{"getStartFinishConfig", "Gets the start/finish line configuration","",GetStartFinishConfig}, \
{"setStartFinishConfig", "Sets the start/finish line configuration","[label_lapCount] [units_lapCount] [sampleRate_lapCount] [label_lapTime] [units_lapTime] [sampleRate_lapTime] [startFinishLat] [startFinishLong] [startFinishRadius]",SetStartFinishConfig}, \
\
{"getGpioConfig","Gets the configuration for the specified GPIO port.","<port>", GetGpioConfig}, \
{"setGpioConfig","Sets the configuration for the specified GPIO port.","<port> [label] [units] [sampleRate] [mode]", SetGpioConfig}, \
\
{"getTimerConfig", "Gets the configuration for the specified timer channel.","<channel>", GetTimerConfig}, \
{"setTimerConfig", "Sets the configuration for the specified timer channel.","<channel>", SetTimerConfig}, \
\
{"getAccelConfig", "Gets the configuration for the specified accelerometer channel.","<channel>",GetAccelConfig}, \
{"setAccelConfig", "Sets the configuration for the specified accelerometer channel.","<channel>",SetAccelConfig}, \
\
{"getAccelInstalled", "Gets the installed state of the accelerometer.","", GetAccelInstalled}, \
{"setAccelInstalled", "Sets the installed state of the accelerometer.","", SetAccelInstalled}, \
\
{"calibrateAccelZero", "Calibrates the current accelerometer values as the zero point.","", CalibrateAccelZero}


void TestSD(unsigned int argc, char **argv);

void FlashLoggerConfig(unsigned int argc, char **argv);

void GetLoggerOutputConfig(unsigned int argc, char **argv);
void SetLoggerOutputConfig(unsigned int argc, char **argv);

void GetAnalogConfig(unsigned int argc, char **argv);
void SetAnalogConfig(unsigned int argc, char **argv);

void GetPwmConfig(unsigned int argc, char **argv);
void SetPwmConfig(unsigned int argc, char **argv);

void SetPwmClockFreq(unsigned int argc, char **argv);
void GetPwmClockFreq(unsigned int argc, char **argv);

void GetGpsConfig(unsigned int argc, char **argv);
void SetGpsConfig(unsigned int argc, char **argv);

void GetStartFinishConfig(unsigned int argc, char **argv);
void SetStartFinishConfig(unsigned int argc, char **argv);

void GetGpioConfig(unsigned int argc, char **argv);
void SetGpioConfig(unsigned int argc, char **argv);

void GetTimerConfig(unsigned int argc, char **argv);
void SetTimerConfig(unsigned int argc, char **argv);

void SetAccelInstalled(unsigned int argc, char **argv);
void GetAccelInstalled(unsigned int argc, char **argv);

void GetAccelConfig(unsigned int argc, char **argv);
void SetAccelConfig(unsigned int argc, char **argv);

void CalibrateAccelZero(unsigned int argc, char **argv);


#endif /* LOGGERCOMMANDS_H_ */
