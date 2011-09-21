/*
 * loggerCommands.h
 *
 *  Created on: Jul 24, 2011
 *      Author: brent
 */

#ifndef LOGGERCOMMANDS_H_
#define LOGGERCOMMANDS_H_

void InitLoggerCommands();

void FlashLoggerConfig(unsigned int argc, char **argv);

void SetAnalogLabel(unsigned int argc, char **argv);
void GetAnalogLabel(unsigned int argc, char **argv);

void GetAnalogSampleRate(unsigned int argc, char **argv);
void SetAnalogSampleRate(unsigned int argc, char **argv);

void SetAnalogScaling(unsigned int argc, char **argv);
void GetAnalogScaling(unsigned int argc, char **argv);


void SetPwmClockFreq(unsigned int argc, char **argv);
void GetPwmClockFreq(unsigned int argc, char **argv);

void SetPwmLabel(unsigned int argc, char **argv);
void GetPwmLabel(unsigned int argc, char **argv);

void SetPwmSampleRate(unsigned int argc, char **argv);
void GetPwmSampleRate(unsigned int argc, char **argv);

void SetPwmOutputCfg(unsigned int argc, char **argv);
void GetPwmOutputCfg(unsigned int argc, char **argv);

void SetPwmLoggingCfg(unsigned int argc, char **argv);
void GetPwmLoggingCfg(unsigned int argc, char **argv);

void SetPwmStartupDutyCycle(unsigned int argc, char **argv);
void GetPwmStartupDutyCycle(unsigned int argc, char **argv);

void SetPWMStartupPeriod(unsigned int argc, char **argv);
void GetPwmStartupPeriod(unsigned int argc, char **argv);

void SetPwmVoltageScaling(unsigned int argc, char **argv);
void getPwmVoltageScaling(unsigned int argc, char **argv);

void SetGpsInstalled(unsigned int argc, char **argv);
void getGpsInstalled(unsigned int argc, char **argv);

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

void SetGpioLabel(unsigned int argc, char **argv);
void GetGpioLabel(unsigned int argc, char **argv);

void SetGpioSampleRate(unsigned int argc, char **argv);
void GetGpioSampleRate(unsigned int argc, char **argv);

void SetGpioConfig(unsigned int argc, char **argv);
void GetGpioConfig(unsigned int argc, char **argv);

void SetTimerLabel(unsigned int argc, char **argv);
void GetTimerLabel(unsigned int argc, char **argv);

void SetTimerSampleRate(unsigned int argc, char **argv);
void GetTimerSampleRate(unsigned int argc, char **argv);

void SetTimerCfg(unsigned int argc, char **argv);
void GetTimerCfg(unsigned int argc, char **argv);

void SetTimerPulsePerRev(unsigned int argc, char **argv);
void GetTimerPulsePerRev(unsigned int argc, char **argv);

void SetTimerDivider(unsigned int argc, char **argv);
void GetTimerDivider(unsigned int argc, char **argv);

void CalculateTimerScaling(unsigned int argc, char **argv);
void GetTimerScaling(unsigned int argc, char **argv);

void SetAccelInstalled(unsigned int argc, char **argv);
void GetAccelInstalled(unsigned int argc, char **argv);

void SetAccelLabel(unsigned int argc, char **argv);
void GetAccelLabel(unsigned int argc, char **argv);

void SetAccelSampleRate(unsigned int argc, char **argv);
void GetAccelSampleRate(unsigned int argc, char **argv);

void SetAccelIdleSampeRate(unsigned int argc, char **argv);
void GetAccelIdleSampleRate(unsigned int argc, char **argv);

void SetAccelConfig(unsigned int argc, char **argv);
void GetAccelConfig(unsigned int argc, char **argv);

void SetAccelChannel(unsigned int argc, char **argv);
void GetAccelChannel(unsigned int argc, char **argv);

void SetAccelZeroValue(unsigned int argc, char **argv);
void GetAccelZeroValue(unsigned int argc, char **argv);

void CalibrateAccelZero(unsigned int argc, char **argv);


#endif /* LOGGERCOMMANDS_H_ */
