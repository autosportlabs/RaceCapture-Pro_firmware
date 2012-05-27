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
\
{"setPwmClockFreq", "Sets the PWM clock frequency for the specified channel.","<channel> <frequency>", SetPwmClockFreq}, \
{"getPwmClockFreq", "Gets the PWM clock frequency for the specified channel.","<channel>", GetPwmClockFreq}, \
\
{"setPwmLabel", "Sets the label for the specified PWM/Analog Output channel.","<channel> <label>", SetPwmLabel}, \
{"getPwmLabel", "Gets the label for the specified PWM/Analog Output channel.","<channel>", GetPwmLabel}, \
\
{"setPwmSampleRate", "Sets the sample rate for the specified PWM/Analog Output channel.","<channel> <sampleRate>", SetPwmSampleRate}, \
{"getPwmSampleRate", "Gets the sample rate for the specified PWM/Analog Output channel.","<channel>", GetPwmSampleRate}, \
\
{"setPwmOutputMode", "Sets the output mode for the specified PWM/Analog Output channel.","<channel> <config>", SetPwmOutputMode}, \
{"getPwmOutputMode", "Gets the output mode for the specified PWM/Analog Output channel.","<channel>", GetPwmOutputMode}, \
\
{"setPwmLoggingCfg", "Sets the logging mode for the specified PWM/Analog Output channel.","<channel> <config>", SetPwmLoggingMode}, \
{"getPwmLoggingCfg", "Sets the logging mode for the specified PWM/Analog Output channel.","<channel>", GetPwmLoggingMode}, \
\
{"setPwmStartupDutyCycle", "Sets the startup duty cycle for the specified PWM/Analog Output channel.","<channel> <dutyCycle>", SetPwmStartupDutyCycle}, \
{"getPwmStartupDutyCycle", "Gets the startup duty cycle for the specified PWM/Analog Output channel.","<channel>", GetPwmStartupDutyCycle}, \
\
{"setPwmStartupPeriod", "Sets the startup period for the specified PWM/Analog Output channel.","<channel> <period>", SetPWMStartupPeriod}, \
{"getPwmStartupPeriod", "Gets the startup period for the specified PWM/Analog Output channel.","<channel>", GetPwmStartupPeriod}, \
\
{"setPwmVoltageScaling", "Sets the voltage scaling for the specified PWM/Analog Output channel.","<channel> <scaling>", SetPwmVoltageScaling}, \
{"getPwmVoltageScaling", "Gets the voltage scaling for the specified PWM/Analog Output channel.","<channel>", getPwmVoltageScaling}, \
\
{"getGpsConfig", "Gets the configuration for Gps features","",GetGpsConfig}, \
\
{"setGpsInstalled", "Sets the installed state of the GPS module.","<installed>", SetGpsInstalled}, \
{"getGpsInstalled", "Gets the installed state of the GPS module.","", getGpsInstalled}, \
\
{"setGpsStartFinish", "Sets the GPS-detected start/finish line.","<latitude> <longitude> [radius]", SetGpsStartFinish}, \
{"getGpsStartFinish", "Gets the GPS-detected start/finish line.","", GetGpsStartFinish}, \
\
{"setGpsQualityLabel", "Sets the GPS quality label.","<label>", SetGpsQualityLabel}, \
{"getGpsQualityLabel", "Gets the GPS quality label.","", GetGpsQualityLabel}, \
\
{"setGpsStatsLabel", "Sets the GPS stats label.","<label>", SetGpsStatsLabel}, \
{"getGpsStatsLabel", "Gets the GPS stats label.","", GetGpsStatsLabel}, \
\
{"setGpsLatitudeLabel", "Sets the GPS latitude label.","<label>", SetGpsLatitudeLabel}, \
{"getGpsLatitudeLabel", "Gets the GPS latitude label.","", GetGpsLatitudeLabel}, \
\
{"setGpsLongitudeLabel", "Sets the GPS longitude label.","<label>", SetGpsLongitudeLabel}, \
{"getGpsLongitudeLabel", "Gets the GPS longitude label.","", GetGpsLongitudeLabel}, \
\
{"setGpsTimeLabel", "Sets the GPS time label.","<label>", SetGpsTimeLabel}, \
{"getGpsTimeLabel", "Gets the GPS time label.","", GetGpsTimeLabel}, \
\
{"setGpsVelocityLabel", "Sets the GPS velocity label.","<label>", SetGpsVelocityLabel}, \
{"getGpsVelocityLabel", "Gets the GPS velocity label.","", GetGpsVelocityLabel}, \
\
{"setGpsPositionSampleRate", "Sets the GPS position sample rate.","<sampleRate>", SetGpsPositionSampleRate}, \
{"getGpsPositionSampleRate", "Gets the GPS position sample rate.","", GetGpsPositionSampleRate}, \
\
{"setGpsVelocitySampleRate", "Sets the GPS velocity sample rate.","<sampleRate>", SetGpsVelocitySampleRate}, \
{"getGpsVelocitySampleRate", "Gets the GPS velocity sample rate.","", GetGpsVelocitySampleRate}, \
\
{"setGpsTimeSampleRate", "Sets the GPS time sample rate.","<sampleRate>", SetGpsTimeSampleRate}, \
{"getGpsTimeSampleRate", "Gets the GPS time sample rate.","", GetGpsTimeSampleRate}, \
\
{"setLapCountSampleRate", "Sets the Lap Count sample rate.","<sampleRate>", SetLapCountSampleRate}, \
{"getLapCountSampleRate", "Gets the Lap Count sample rate.","", GetLapCountSampleRate}, \
\
{"setLapTimeSampleRate", "Sets the Lap Time sample rate.","<sampleRate>", SetLapTimeSampleRate}, \
{"getLapTimeSampleRate", "Gets the Lap Time sample rate.","", GetLapTimeSampleRate}, \
\
{"setLapCountLabel", "Sets the Lap Count label.","<label>", SetLapCountLabel}, \
{"getLapCountLabel", "Gets the Lap Count label.","", GetLapCountLabel}, \
\
{"setLapTimeLabel", "Sets the Lap Time label.","<label>", SetLapTimeLabel}, \
{"getLapTimeLabel", "Gets the Lap Time label.","", GetLapTimeLabel}, \
\
{"getGpioConfig","Gets the configuration for the specified GPIO port.","<port>", GetGpioConfig}, \
\
{"setGpioLabel", "Sets the label for the specified GPIO port.","<port> <label>", SetGpioLabel}, \
{"getGpioLabel", "Gets the label for the specified GPIO port.","<port>", GetGpioLabel}, \
\
{"setGpioSampleRate", "Sets the sample rate for the specified GPIO port.","<port> <sampleRate>", SetGpioSampleRate}, \
{"getGpioSampleRate", "Gets the sample rate for the specified GPIO port.","<port>", GetGpioSampleRate}, \
\
{"setGpioMode", "Sets the mode for the specified GPIO port.","<port> <mode>", SetGpioMode}, \
{"getGpioMode", "Gets the mode for the specified GPIO port.","<port>", GetGpioMode}, \
\
{"getTimerConfig", "Gets the configuration for the specified timer channel.","<channel>", GetTimerConfig}, \
{"setTimerLabel", "Sets the label for the specified timer channel.","<channel> <label>", SetTimerLabel}, \
{"getTimerLabel", "Gets the label for the specified timer channel.","<channel>", GetTimerLabel}, \
\
{"setTimerSampleRate", "Sets the sample rate for the specified timer channel.","<channel> <sampleRate>", SetTimerSampleRate}, \
{"getTimerSampleRate", "Gets the sample rate for the specified timer channel.","<channel>", GetTimerSampleRate}, \
\
{"setTimerMode", "Sets the mode for the specified timer channel.","<channel> <mode>", SetTimerMode}, \
{"getTimerMode", "Gets the mode for the specified timer channel.","<channel>", GetTimerMode}, \
\
{"setTimerPulsePerRev", "Sets the pulse per revolution for the specified timer channel.","<channel> <pulses>", SetTimerPulsePerRev}, \
{"getTimerPulsePerRev", "Gets the pulse per revolution for the specified timer channel.","<channel>", GetTimerPulsePerRev}, \
\
{"setTimerDivider", "Sets the divider for the specified timer channel.","<channel> <divider>", SetTimerDivider}, \
{"getTimerDivider", "Gets the divider for the specified timer channel.","<channel>", GetTimerDivider}, \
\
{"calculateTimerScaling", "Calculates and updates the scaling for the specified timer channel.","<channel>", CalculateTimerScaling}, \
{"getTimerScaling", "Gets the scaling for the specified timer channel.","<channel>", GetTimerScaling}, \
\
{"getAccelConfig", "Gets the configuration for the specified accelerometer channel.","<channel>",GetAccelConfig}, \
\
{"setAccelInstalled", "Sets the installed state of the accelerometer.","", SetAccelInstalled}, \
{"getAccelInstalled", "Gets the installed state of the accelerometer.","", GetAccelInstalled}, \
\
{"setAccelLabel", "Sets the label for the specified accelerometer channel.","<channel> <label>", SetAccelLabel}, \
{"getAccelLabel", "Gets the label for the specified accelerometer channel.","<channel>", GetAccelLabel}, \
\
{"setAccelSampleRate", "Sets the sample rate for the specified accelerometer channel.","<channel> <sampleRate>", SetAccelSampleRate}, \
{"getAccelSampleRate", "Gets the sample rate for the specified accelerometer channel.","<channel>", GetAccelSampleRate}, \
\
{"setAccelMode", "Sets the mode for the specified accelerometer channel.","<channel> <mode>", SetAccelMode}, \
{"getAccelMode", "Gets the mode for the specified accelerometer channel.","<channel>", GetAccelMode}, \
\
{"setAccelChannel", "Sets the X/Y/Z orientation for the specified accelerometer channel.","<channel> <orientation>", SetAccelChannel}, \
{"getAccelChannel", "Gets the X/Y/Z orientation for the specified accelerometer channel.","<channel>", GetAccelChannel}, \
\
{"setAccelZeroValue", "Sets the zero value for the specified accelerometer channel.","<channel> <zeroValue>", SetAccelZeroValue}, \
{"getAccelZeroValue", "Gets the zero value for the specified accelerometer channel.","<channel>", GetAccelZeroValue}, \
\
{"calibrateAccelZero", "Calibrates the current accelerometer values as the zero point.","", CalibrateAccelZero}


void TestSD(unsigned int argc, char **argv);

void FlashLoggerConfig(unsigned int argc, char **argv);

void GetLoggerOutputConfig(unsigned int argc, char **argv);
void SetLoggerOutputConfig(unsigned int argc, char **argv);

void GetAnalogConfig(unsigned int argc, char **argv);
void SetAnalogConfig(unsigned int argc, char **argv);

void GetPwmConfig(unsigned int argc, char **argv);

void SetPwmClockFreq(unsigned int argc, char **argv);
void GetPwmClockFreq(unsigned int argc, char **argv);

void SetPwmLabel(unsigned int argc, char **argv);
void GetPwmLabel(unsigned int argc, char **argv);

void SetPwmSampleRate(unsigned int argc, char **argv);
void GetPwmSampleRate(unsigned int argc, char **argv);

void SetPwmOutputMode(unsigned int argc, char **argv);
void GetPwmOutputMode(unsigned int argc, char **argv);

void SetPwmLoggingMode(unsigned int argc, char **argv);
void GetPwmLoggingMode(unsigned int argc, char **argv);

void SetPwmStartupDutyCycle(unsigned int argc, char **argv);
void GetPwmStartupDutyCycle(unsigned int argc, char **argv);

void SetPWMStartupPeriod(unsigned int argc, char **argv);
void GetPwmStartupPeriod(unsigned int argc, char **argv);

void SetPwmVoltageScaling(unsigned int argc, char **argv);
void getPwmVoltageScaling(unsigned int argc, char **argv);

void GetGpsConfig(unsigned int argc, char **argv);

void SetGpsInstalled(unsigned int argc, char **argv);
void getGpsInstalled(unsigned int argc, char **argv);

void SetGpsStartFinish(unsigned int argc, char **argv);
void GetGpsStartFinish(unsigned int argc, char **argv);

void SetGpsQualityLabel(unsigned int argc, char **argv);
void GetGpsQualityLabel(unsigned int argc, char **argv);

void SetGpsStatsLabel(unsigned int argc, char **argv);
void GetGpsStatsLabel(unsigned int argc, char **argv);

void SetGpsLatitudeLabel(unsigned int argc, char **argv);
void GetGpsLatitudeLabel(unsigned int argc, char **argv);

void SetGpsLongitudeLabel(unsigned int argc, char **argv);
void GetGpsLongitudeLabel(unsigned int argc, char **argv);

void SetGpsTimeLabel(unsigned int argc, char **argv);
void GetGpsTimeLabel(unsigned int argc, char **argv);

void SetGpsVelocityLabel(unsigned int argc, char **argv);
void GetGpsVelocityLabel(unsigned int argc, char **argv);

void SetGpsPositionSampleRate(unsigned int argc, char **argv);
void GetGpsPositionSampleRate(unsigned int argc, char **argv);

void SetGpsVelocitySampleRate(unsigned int argc, char **argv);
void GetGpsVelocitySampleRate(unsigned int argc, char **argv);

void SetGpsTimeSampleRate(unsigned int argc, char **argv);
void GetGpsTimeSampleRate(unsigned int argc, char **argv);

void SetLapCountLabel(unsigned int argc, char **argv);
void GetLapCountLabel(unsigned int argc, char **argv);

void SetLapCountSampleRate(unsigned int argc, char **argv);
void GetLapCountSampleRate(unsigned int argc, char **argv);

void SetLapTimeLabel(unsigned int argc, char **argv);
void GetLapTimeLabel(unsigned int argc, char **argv);

void SetLapTimeSampleRate(unsigned int argc, char **argv);
void GetLapTimeSampleRate(unsigned int argc, char **argv);

void GetGpioConfig(unsigned int argc, char **argv);

void SetGpioLabel(unsigned int argc, char **argv);
void GetGpioLabel(unsigned int argc, char **argv);

void SetGpioSampleRate(unsigned int argc, char **argv);
void GetGpioSampleRate(unsigned int argc, char **argv);

void SetGpioMode(unsigned int argc, char **argv);
void GetGpioMode(unsigned int argc, char **argv);

void GetTimerConfig(unsigned int argc, char **argv);
void SetTimerLabel(unsigned int argc, char **argv);
void GetTimerLabel(unsigned int argc, char **argv);

void SetTimerSampleRate(unsigned int argc, char **argv);
void GetTimerSampleRate(unsigned int argc, char **argv);

void SetTimerMode(unsigned int argc, char **argv);
void GetTimerMode(unsigned int argc, char **argv);

void SetTimerPulsePerRev(unsigned int argc, char **argv);
void GetTimerPulsePerRev(unsigned int argc, char **argv);

void SetTimerDivider(unsigned int argc, char **argv);
void GetTimerDivider(unsigned int argc, char **argv);

void CalculateTimerScaling(unsigned int argc, char **argv);
void GetTimerScaling(unsigned int argc, char **argv);

void GetAccelConfig(unsigned int argc, char **argv);

void SetAccelInstalled(unsigned int argc, char **argv);
void GetAccelInstalled(unsigned int argc, char **argv);

void SetAccelLabel(unsigned int argc, char **argv);
void GetAccelLabel(unsigned int argc, char **argv);

void SetAccelSampleRate(unsigned int argc, char **argv);
void GetAccelSampleRate(unsigned int argc, char **argv);

void SetAccelMode(unsigned int argc, char **argv);
void GetAccelMode(unsigned int argc, char **argv);

void SetAccelChannel(unsigned int argc, char **argv);
void GetAccelChannel(unsigned int argc, char **argv);

void SetAccelZeroValue(unsigned int argc, char **argv);
void GetAccelZeroValue(unsigned int argc, char **argv);

void CalibrateAccelZero(unsigned int argc, char **argv);


#endif /* LOGGERCOMMANDS_H_ */
