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
{"sample", "Gets a single sample of the currently enabled channels","",SampleData}, \
{"testSD", "Test Write to SD card.","<lineWrites> <periodicFlush> <quietMode>", TestSD}, \
{"flashLoggerCfg", "Write the active Logger Configuration to flash.","", FlashLoggerConfig}, \
\
{"getCellCfg", "Gets the cellular connectivity configuration", "", GetCellConfig}, \
{"setCellCfg", "Sets the cellular connectivity configuration", "<apnHost> <apnUser> <apnPassword>", SetCellConfig}, \
\
{"getBtCfg", "Gets the Bluetooth configuration", "", GetBluetoothConfig}, \
{"setBtCfg", "Sets the Bluetooth configuration", "<deviceName> <passcode>", SetBluetoothConfig}, \
{"getOutputCfg", "Gets the Output Configuration","",GetLoggerOutputConfig}, \
{"setOutputCfg", "Sets the Output Configuration","[sdLoggingMode] [telemetryMode] [p2pDestAddrHigh] [p2pDestAddrLow] [telemetryHost] [telemetryDeviceId]",SetLoggerOutputConfig}, \
\
{"getAnalogCfg", "Gets the configuration for the specified analog channel","<channel>", GetAnalogConfig}, \
{"setAnalogCfg", "Sets the configuration for the specified analog channel","<channel> [label] [units] [sampleRate] [loggingPrecision] [scalingMode] [linearScaling] < [mapRaw_0] [mapRaw_1] [mapRaw_2] [mapRaw_3] [mapRaw_4] [mapScaled_0] [mapScaled_1] [mapScaled_2] [mapScaled_3] [mapScaled_4] >",SetAnalogConfig}, \
\
{"getPwmCfg","Gets the configuration for the specified PWM/Analog Output channel.","<channel>",GetPwmConfig}, \
{"setPwmCfg","Sets the configuration for the specified PWM/Analog Output channel.","<channel> [label] [units] [sampleRate] [loggingPrecision] [outputMode] [loggingMode] [startupDutyCycle] [startupPeriod] [voltageScaling]",SetPwmConfig}, \
\
{"setPwmClockFreq", "Sets the PWM clock frequency for the specified channel.","<frequency>", SetPwmClockFreq}, \
{"getPwmClockFreq", "Gets the PWM clock frequency for the specified channel.","", GetPwmClockFreq}, \
\
{"getGpsCfg", "Gets the configuration for Gps features","",GetGpsConfig}, \
{"setGpsCfg", "Sets the configuration for Gps features","[installed] [label_lat] [units_lat] [sampleRate_lat] [label_long] [units_long] [sampleRate_long] [label_vel] [units_vel] [sampleRate_vel] [label_time] [units_time] [sampleRate_time] [label_date] [units_date] [sampleRate_date] [label_sats] [units_sats] [sampleRate_sats]",SetGpsConfig}, \
\
{"getDistCfg", "Gets the configuration for the distance channels", "", GetDistanceConfig}, \
{"setDistCfg", "Sets the configuration for the distance channels", "[label_lapDist] [units_lapDist] [sampleRate_lapDist] [label_totalDist] [units_totalDist] [sampleRate_totalDist]", SetDistanceConfig}, \
\
{"getStartFinishCfg", "Gets the start/finish line configuration","",GetStartFinishConfig}, \
{"setStartFinishCfg", "Sets the start/finish line configuration","[label_lapCount] [units_lapCount] [sampleRate_lapCount] [label_lapTime] [units_lapTime] [sampleRate_lapTime] [label_splitTime] [units_splitTime] [sampleRate_splitTime] [startFinishLat] [startFinishLong] [startFinishRadius] [splitLat] [splitLong] [splitRadius]",SetStartFinishConfig}, \
\
{"getGpioCfg","Gets the configuration for the specified GPIO port.","<port>", GetGpioConfig}, \
{"setGpioCfg","Sets the configuration for the specified GPIO port.","<port> [label] [units] [sampleRate] [mode]", SetGpioConfig}, \
\
{"getTimerCfg", "Gets the configuration for the specified timer channel.","<channel>", GetTimerConfig}, \
{"setTimerCfg", "Sets the configuration for the specified timer channel.","<channel> [label] [units] [sampleRate] [loggingPrecision] [slowTimerEnabled] [mode] [pulsePerRevolution] [timerDivider] [scaling]", SetTimerConfig}, \
\
{"getAccelCfg", "Gets the configuration for the specified accelerometer channel.","<channel>",GetAccelConfig}, \
{"setAccelCfg", "Sets the configuration for the specified accelerometer channel.","<channel> [label] [units] [sampleRate] [mode] [accelChannel] [zeroValue]",SetAccelConfig}, \
\
{"calibrateAccelZero", "Calibrates the current accelerometer values as the zero point.","", CalibrateAccelZero},\
\
{"startTerminal", "Starts a debugging terminal session on the specified port.","<port> <baud>", StartTerminal },\
{"viewLog", "Prints out logging messages to the terminal as they happen", "", ViewLog },\
{"setLogLevel", "Sets the log level", "<level>", SetLogLevel }


void SampleData(Serial *serial, unsigned int argc, char **argv);

void TestSD(Serial *serial, unsigned int argc, char **argv);

void FlashLoggerConfig(Serial *serial, unsigned int argc, char **argv);

void GetBluetoothConfig(Serial *serial, unsigned int argc, char **argv);
void SetBluetoothConfig(Serial *serial, unsigned int argc, char **argv);

void GetCellConfig(Serial *serial, unsigned int argc, char **argv);
void SetCellConfig(Serial *serial, unsigned int argc, char **argv);

void GetLoggerOutputConfig(Serial *serial, unsigned int argc, char **argv);
void SetLoggerOutputConfig(Serial *serial, unsigned int argc, char **argv);

void GetAnalogConfig(Serial *serial, unsigned int argc, char **argv);
void SetAnalogConfig(Serial *serial, unsigned int argc, char **argv);

void GetPwmConfig(Serial *serial, unsigned int argc, char **argv);
void SetPwmConfig(Serial *serial, unsigned int argc, char **argv);

void SetPwmClockFreq(Serial *serial, unsigned int argc, char **argv);
void GetPwmClockFreq(Serial *serial, unsigned int argc, char **argv);

void GetGpsConfig(Serial *serial, unsigned int argc, char **argv);
void SetGpsConfig(Serial *serial, unsigned int argc, char **argv);

void GetDistanceConfig(Serial *serial, unsigned int argc, char **argv);
void SetDistanceConfig(Serial *serial, unsigned int argc, char **argv);

void GetStartFinishConfig(Serial *serial, unsigned int argc, char **argv);
void SetStartFinishConfig(Serial *serial, unsigned int argc, char **argv);

void GetGpioConfig(Serial *serial, unsigned int argc, char **argv);
void SetGpioConfig(Serial *serial, unsigned int argc, char **argv);

void GetTimerConfig(Serial *serial, unsigned int argc, char **argv);
void SetTimerConfig(Serial *serial, unsigned int argc, char **argv);

void GetAccelConfig(Serial *serial, unsigned int argc, char **argv);
void SetAccelConfig(Serial *serial, unsigned int argc, char **argv);

void CalibrateAccelZero(Serial *serial, unsigned int argc, char **argv);

void StartTerminal(Serial *serial, unsigned int argc, char **argv);
void ViewLog(Serial *serial, unsigned int argc, char **argv);
void SetLogLevel(Serial *serial, unsigned int argc, char **argv);

#endif /* LOGGERCOMMANDS_H_ */
