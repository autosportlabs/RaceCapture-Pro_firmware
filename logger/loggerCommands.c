/*
 * loggerCommands.c
 *
 *  Created on: Jul 24, 2011
 *      Author: brent
 */
#include <stddef.h>
#include <string.h>
#include "loggerCommands.h"
#include "modp_numtoa.h"
#include "modp_atonum.h"
#include "loggerConfig.h"
#include "sdcard.h"

void TestSD(unsigned int argc, char **argv){

	int lines = 1;
	int doFlush = 0;
	if (argc > 1) lines = modp_atoi(argv[1]);
	if (argc > 2) doFlush = modp_atoi(argv[2]);
	TestSDWrite(lines,doFlush);
}

void FlashLoggerConfig(unsigned int argc, char **argv){
	int result = flashLoggerConfig();
	if (0 == result){
		SendCommandOK();
	}
	else{
		SendCommandError(result);
	}
}

struct ADCConfig * AssertAdcSetParam(unsigned int argc, char **argv){
	struct ADCConfig *c = NULL;
	if (argc >= 3){
		c = getADCConfigChannel(modp_atoi(argv[1]));
		if (NULL == c) SendCommandError(ERROR_CODE_INVALID_PARAM);
	}
	else{
		SendCommandError(ERROR_CODE_MISSING_PARAMS);
	}
	return c;
}

struct ADCConfig * AssertAdcGetParam(unsigned int argc, char **argv){
	struct ADCConfig *c = NULL;
	if (argc >= 2){
		c = getADCConfigChannel(modp_atoi(argv[1]));
		if (NULL == c) SendCommandError(ERROR_CODE_INVALID_PARAM);
	}
	else{
		SendCommandError(ERROR_CODE_MISSING_PARAMS);
	}
	return c;
}


void SetAnalogLabel(unsigned int argc, char **argv){

	if (argc >= 4){
		struct ADCConfig *c = getADCConfigChannel(modp_atoi(argv[1]));
		if (NULL != c){
			setLabelGeneric(c->label,argv[2]);
			setLabelGeneric(c->units,argv[3]);
			SendCommandOK();
		}
		else{
			SendCommandError(ERROR_CODE_INVALID_PARAM);
		}
	}
	else{
		SendCommandError(ERROR_CODE_MISSING_PARAMS);
	}
}

void GetAnalogLabel(unsigned int argc, char **argv){
	struct ADCConfig * c = AssertAdcGetParam(argc,argv);
	if (NULL != c){
		SendNameString("label",c->label);
		SendNameString("units",c->units);
	}

}


void SetAnalogSampleRate(unsigned int argc, char **argv){
	struct ADCConfig * c = AssertAdcSetParam(argc,argv);
	if (NULL != c){
		c->sampleRate = encodeSampleRate(modp_atoi(argv[2]));
		SendCommandOK();
	}
}

void GetAnalogSampleRate(unsigned int argc, char **argv){
	struct ADCConfig * c = AssertAdcGetParam(argc,argv);
	if (NULL != c) SendNameInt("sampleRate",decodeSampleRate(c->sampleRate));
}

void SetAnalogScaling(unsigned int argc, char **argv){
	struct ADCConfig * c = AssertAdcSetParam(argc,argv);
	if (NULL != c){
		c->scaling = modp_atof(argv[2]);
		SendCommandOK();
	}
}

void GetAnalogScaling(unsigned int argc, char **argv){
	struct ADCConfig * c = AssertAdcGetParam(argc,argv);
	if (NULL != c) SendNameFloat("scaling",c->scaling,2);
}

void SetPwmClockFreq(unsigned int argc, char **argv){
	if (argc >= 2){
		getWorkingLoggerConfig()->PWMClockFrequency = filterPWMClockFrequency(modp_atoi(argv[1]));
		SendCommandOK();
	}
	else{
		SendCommandError(ERROR_CODE_MISSING_PARAMS);
	}
}

void GetPwmClockFreq(unsigned int argc, char **argv){
	SendNameInt("frequency",getWorkingLoggerConfig()->PWMClockFrequency);
}

struct PWMConfig * AssertPwmSetParam(unsigned int argc, char **argv){
	struct PWMConfig *c = NULL;
	if (argc >= 3){
		c = getPWMConfigChannel(modp_atoi(argv[1]));
		if (NULL == c) SendCommandError(ERROR_CODE_INVALID_PARAM);
	}
	else{
		SendCommandError(ERROR_CODE_MISSING_PARAMS);
	}
	return c;
}

struct PWMConfig * AssertPwmGetParam(unsigned int argc, char **argv){
	struct PWMConfig *c = NULL;
	if (argc >= 2){
		c = getPWMConfigChannel(modp_atoi(argv[1]));
		if (NULL == c) SendCommandError(ERROR_CODE_INVALID_PARAM);
	}
	else{
		SendCommandError(ERROR_CODE_MISSING_PARAMS);
	}
	return c;
}

void SetPwmLabel(unsigned int argc, char **argv){
	struct PWMConfig *c = AssertPwmSetParam(argc,argv);
	if (NULL != c){
		setLabelGeneric(c->label,argv[2]);
		SendCommandOK();
	}
}

void GetPwmLabel(unsigned int argc, char **argv){
	struct PWMConfig *c = AssertPwmGetParam(argc,argv);
	if (NULL !=c ) SendNameString("label",c->label);
}

void SetPwmSampleRate(unsigned int argc, char **argv){
	struct PWMConfig *c = AssertPwmSetParam(argc,argv);
	if (NULL != c){
		c->sampleRate = encodeSampleRate(modp_atoi(argv[2]));
		SendCommandOK();
	}
}

void GetPwmSampleRate(unsigned int argc, char **argv){
	struct PWMConfig *c = AssertPwmGetParam(argc,argv);
	if (NULL !=c ) SendNameInt("sampleRate",decodeSampleRate(c->sampleRate));
}

void SetPwmOutputCfg(unsigned int argc, char **argv){
	struct PWMConfig *c = AssertPwmSetParam(argc,argv);
	if (NULL != c){
		c->outputConfig = filterPWMOutputConfig(modp_atoi(argv[2]));
		SendCommandOK();
	}
}

void GetPwmOutputCfg(unsigned int argc, char **argv){
	struct PWMConfig *c = AssertPwmGetParam(argc,argv);
	if (NULL != c) SendNameInt("outputConfig",c->outputConfig);
}

void SetPwmLoggingCfg(unsigned int argc, char **argv){
	struct PWMConfig *c = AssertPwmSetParam(argc,argv);
	if (NULL != c) {
		c->loggingConfig = filterPWMLoggingConfig(modp_atoi(argv[2]));
		SendCommandOK();
	}
}

void GetPwmLoggingCfg(unsigned int argc, char **argv){
	struct PWMConfig *c = AssertPwmGetParam(argc,argv);
	if (NULL !=c ) SendNameInt("loggingConfig",c->loggingConfig);
}

void SetPwmStartupDutyCycle(unsigned int argc, char **argv){
	struct PWMConfig *c = AssertPwmSetParam(argc,argv);
	if (NULL != c){
		c->startupDutyCycle = filterPWMDutyCycle(modp_atoi(argv[2]));
		SendCommandOK();
	}
}

void GetPwmStartupDutyCycle(unsigned int argc, char **argv){
	struct PWMConfig *c = AssertPwmGetParam(argc,argv);
	if (NULL != c) SendNameInt("dutyCycle",c->startupDutyCycle);
}

void SetPWMStartupPeriod(unsigned int argc, char **argv){
	struct PWMConfig *c = AssertPwmSetParam(argc,argv);
	if (NULL != c){
		c->startupPeriod = filterPWMPeriod(modp_atoi(argv[2]));
		SendCommandOK();
	}
}

void GetPwmStartupPeriod(unsigned int argc, char **argv){
	struct PWMConfig *c = AssertPwmGetParam(argc,argv);
	if (NULL != c) SendNameInt("startupPeriod",c->startupPeriod);
}

void SetPwmVoltageScaling(unsigned int argc, char **argv){
	struct PWMConfig *c = AssertPwmSetParam(argc,argv);
	if (NULL != c){
		c->voltageScaling = modp_atof(argv[2]);
		SendCommandOK();
	}
}

void getPwmVoltageScaling(unsigned int argc, char **argv){
	struct PWMConfig *c = AssertPwmGetParam(argc,argv);
	if (NULL == c) return;
	SendNameFloat("voltageScaling",c->voltageScaling,2);
}

struct LoggerConfig * AssertSetBaseParam(unsigned int argc){
	if (argc >= 2){
		return getWorkingLoggerConfig();
	}
	else{
		SendCommandError(ERROR_CODE_MISSING_PARAMS);
	}
	return NULL;
}

void SetGpsInstalled(unsigned int argc, char **argv){
	struct LoggerConfig * c = AssertSetBaseParam(argc);
	if (NULL != c){
		c->GPSInstalled = (modp_atoi(argv[1]) != 0);
		SendCommandOK();
	}
}

void getGpsInstalled(unsigned int argc, char **argv){
	SendNameInt("gpsInstalled",getWorkingLoggerConfig()->GPSInstalled);
}

void SetGpsQualityLabel(unsigned int argc, char **argv){
	struct LoggerConfig * c = AssertSetBaseParam(argc);
	if (NULL != c){
		setLabelGeneric(c->GPSConfig.qualityLabel,argv[1]);
		SendCommandOK();
	}
}

void GetGpsQualityLabel(unsigned int argc, char **argv){
	SendNameString("gpsQualityLabel", getWorkingLoggerConfig()->GPSConfig.qualityLabel);
}

void SetGpsStatsLabel(unsigned int argc, char **argv){
	struct LoggerConfig * c = AssertSetBaseParam(argc);
	if (NULL != c){
		setLabelGeneric(c->GPSConfig.satsLabel, argv[1]);
		SendCommandOK();
	}
}

void GetGpsStatsLabel(unsigned int argc, char **argv){
	SendNameString("gpsStatsLabel", getWorkingLoggerConfig()->GPSConfig.satsLabel);
}

void SetGpsLatitudeLabel(unsigned int argc, char **argv){
	struct LoggerConfig * c = AssertSetBaseParam(argc);
	if (NULL != c){
		setLabelGeneric(c->GPSConfig.latitiudeLabel, argv[1]);
		SendCommandOK();
	}
}

void GetGpsLatitudeLabel(unsigned int argc, char **argv){
	SendNameString("gpsLatitudeLabel", getWorkingLoggerConfig()->GPSConfig.latitiudeLabel);
}

void SetGpsLongitudeLabel(unsigned int argc, char **argv){
	struct LoggerConfig * c = AssertSetBaseParam(argc);
	if (NULL != c){
		setLabelGeneric(c->GPSConfig.longitudeLabel, argv[1]);
		SendCommandOK();
	}
}

void GetGpsLongitudeLabel(unsigned int argc, char **argv){
	SendNameString("gpsLongitudeLabel", getWorkingLoggerConfig()->GPSConfig.longitudeLabel);
}

void SetGpsTimeLabel(unsigned int argc, char **argv){
	struct LoggerConfig * c = AssertSetBaseParam(argc);
	if (NULL != c){
		setLabelGeneric(c->GPSConfig.timeLabel, argv[1]);
		SendCommandOK();
	}
}

void GetGpsTimeLabel(unsigned int argc, char **argv){
	SendNameString("gpsTimeLabel", getWorkingLoggerConfig()->GPSConfig.timeLabel);
}

void SetGpsVelocityLabel(unsigned int argc, char **argv){
	struct LoggerConfig * c = AssertSetBaseParam(argc);
	if (NULL != c){
		setLabelGeneric(c->GPSConfig.velocityLabel, argv[1]);
		SendCommandOK();
	}
}

void GetGpsVelocityLabel(unsigned int argc, char **argv){
	SendNameString("gpsVelocityLabel", getWorkingLoggerConfig()->GPSConfig.velocityLabel);
}

void SetGpsPositionSampleRate(unsigned int argc, char **argv){
	struct LoggerConfig * c = AssertSetBaseParam(argc);
	if (NULL != c){
		c->GPSConfig.positionSampleRate = encodeSampleRate(modp_atoi(argv[1]));
		SendCommandOK();
	}
}

void GetGpsPositionSampleRate(unsigned int argc, char **argv){
	SendNameInt("gpsPositionSampleRate",decodeSampleRate(getWorkingLoggerConfig()->GPSConfig.positionSampleRate));
}

void SetGpsVelocitySampleRate(unsigned int argc, char **argv){
	struct LoggerConfig * c = AssertSetBaseParam(argc);
	if (NULL != c){
		c->GPSConfig.velocitySampleRate = encodeSampleRate(modp_atoi(argv[1]));
		SendCommandOK();
	}
}

void GetGpsVelocitySampleRate(unsigned int argc, char **argv){
	SendNameInt("gpsVelocitySampleRate",decodeSampleRate(getWorkingLoggerConfig()->GPSConfig.velocitySampleRate));
}

void SetGpsTimeSampleRate(unsigned int argc, char **argv){
	struct LoggerConfig * c = AssertSetBaseParam(argc);
	if (NULL != c){
		c->GPSConfig.timeSampleRate = encodeSampleRate(modp_atoi(argv[1]));
		SendCommandOK();
	}
}

void GetGpsTimeSampleRate(unsigned int argc, char **argv){
	SendNameInt("gpsTimeSampleRate", decodeSampleRate(getWorkingLoggerConfig()->GPSConfig.timeSampleRate));
}

struct GPIOConfig * AssertGpioSetParam(unsigned int argc, char **argv){
	struct GPIOConfig *c = NULL;
	if (argc >= 3){
		c = getGPIOConfigChannel(modp_atoi(argv[1]));
		if (NULL == c) SendCommandError(ERROR_CODE_INVALID_PARAM);
	}
	else{
		SendCommandError(ERROR_CODE_MISSING_PARAMS);
	}
	return c;
}

struct GPIOConfig * AssertGpioGetParam(unsigned int argc, char **argv){
	struct GPIOConfig *c = NULL;
	if (argc >= 2){
		c = getGPIOConfigChannel(modp_atoi(argv[1]));
		if (NULL == c) SendCommandError(ERROR_CODE_INVALID_PARAM);
	}
	else{
		SendCommandError(ERROR_CODE_MISSING_PARAMS);
	}
	return c;
}

void SetGpioLabel(unsigned int argc, char **argv){
	struct GPIOConfig * c = AssertGpioSetParam(argc,argv);
	if (NULL != c){
		setLabelGeneric(c->label,argv[2]);
		SendCommandOK();
	}
}

void GetGpioLabel(unsigned int argc, char **argv){
	struct GPIOConfig * c = AssertGpioGetParam(argc,argv);
	if (NULL != c) SendNameString("gpioLabel",c->label);
}

void SetGpioSampleRate(unsigned int argc, char **argv){
	struct GPIOConfig * c = AssertGpioSetParam(argc,argv);
	if (NULL != c){
		c->sampleRate = encodeSampleRate(modp_atoi(argv[2]));
		SendCommandOK();
	}
}

void GetGpioSampleRate(unsigned int argc, char **argv){
	struct GPIOConfig * c = AssertGpioGetParam(argc,argv);
	if (NULL != c) SendNameInt("gpioSampleRate",decodeSampleRate(c->sampleRate));
}

void SetGpioConfig(unsigned int argc, char **argv){
	struct GPIOConfig * c = AssertGpioSetParam(argc,argv);
	if (NULL != c){
		c->config = filterGPIOConfig(modp_atoi(argv[2]));
		SendCommandOK();
	}
}

void GetGpioConfig(unsigned int argc, char **argv){
	struct GPIOConfig * c = AssertGpioGetParam(argc,argv);
	if (NULL != c) SendNameInt("gpioConfig",c->config);
}

struct TimerConfig * AssertTimerSetParam(unsigned int argc, char **argv){
	struct TimerConfig *c = NULL;
	if (argc >= 3){
		c = getTimerConfigChannel(modp_atoi(argv[1]));
		if (NULL == c) SendCommandError(ERROR_CODE_INVALID_PARAM);
	}
	else{
		SendCommandError(ERROR_CODE_MISSING_PARAMS);
	}
	return c;
}

struct TimerConfig* AssertTimerGetParam(unsigned int argc, char **argv){
	struct TimerConfig *c = NULL;
	if (argc >= 2){
		c = getTimerConfigChannel(modp_atoi(argv[1]));
		if (NULL == c) SendCommandError(ERROR_CODE_INVALID_PARAM);
	}
	else{
		SendCommandError(ERROR_CODE_MISSING_PARAMS);
	}
	return c;
}

void SetTimerLabel(unsigned int argc, char **argv){
	struct TimerConfig * c = AssertTimerSetParam(argc,argv);
	if (NULL != c){
		setLabelGeneric(c->label,argv[2]);
		SendCommandOK();
	}
}

void GetTimerLabel(unsigned int argc, char **argv){
	struct TimerConfig * c = AssertTimerGetParam(argc,argv);
	if (NULL !=c) SendNameString("timerLabel",c->label);
}

void SetTimerSampleRate(unsigned int argc, char **argv){
	struct TimerConfig * c = AssertTimerSetParam(argc,argv);
	if (NULL != c){
		c->sampleRate = encodeSampleRate(modp_atoi(argv[2]));
		SendCommandOK();
	}
}

void GetTimerSampleRate(unsigned int argc, char **argv){
	struct TimerConfig * c = AssertTimerGetParam(argc,argv);
	if (NULL != c) SendNameInt("timerSampleRate", decodeSampleRate(c->sampleRate));
}

void SetTimerCfg(unsigned int argc, char **argv){
	struct TimerConfig * c = AssertTimerSetParam(argc,argv);
	if (NULL != c){
		c->config = filterTimerConfig(modp_atoi(argv[2]));
		SendCommandOK();
	}
}

void GetTimerCfg(unsigned int argc, char **argv){
	struct TimerConfig * c = AssertTimerGetParam(argc,argv);
	if (NULL != c) SendNameInt("timerConfig", c->config);
}

void SetTimerPulsePerRev(unsigned int argc, char **argv){
	struct TimerConfig * c = AssertTimerSetParam(argc,argv);
	if (NULL != c){
		c->pulsePerRevolution = modp_atoi(argv[2]);
		SendCommandOK();
	}
}

void GetTimerPulsePerRev(unsigned int argc, char **argv){
	struct TimerConfig * c = AssertTimerGetParam(argc,argv);
	if (NULL != c){
		SendNameInt("timerPulsePerRev",c->pulsePerRevolution);
	}
}

void SetTimerDivider(unsigned int argc, char **argv){
	struct TimerConfig * c = AssertTimerSetParam(argc,argv);
	if (NULL != c){
		c->timerDivider = filterTimerDivider(modp_atoi(argv[2]));
		SendCommandOK();
	}
}

void GetTimerDivider(unsigned int argc, char **argv){
	struct TimerConfig * c = AssertTimerGetParam(argc,argv);
	if (NULL != c){
		SendNameInt("timerDivider",c->timerDivider);
	}
}

void CalculateTimerScaling(unsigned int argc, char **argv){
	struct TimerConfig * c = AssertTimerGetParam(argc,argv);
	if (NULL != c){
		calculateTimerScaling(c);
		SendCommandOK();
	}
}

void GetTimerScaling(unsigned int argc, char **argv){
	struct TimerConfig * c = AssertTimerGetParam(argc,argv);
	if (NULL != c){
		SendNameInt("timerScaling",c->calculatedScaling);
		SendCommandOK();
	}
}

struct AccelConfig * AssertAccelSetParam(unsigned int argc, char **argv){
	struct AccelConfig *c = NULL;
	if (argc >= 3){
		c = getAccelConfigChannel(modp_atoi(argv[1]));
		if (NULL == c) SendCommandError(ERROR_CODE_INVALID_PARAM);
	}
	else{
		SendCommandError(ERROR_CODE_MISSING_PARAMS);
	}
	return c;
}

struct AccelConfig * AssertAccelGetParam(unsigned int argc, char **argv){
	struct AccelConfig *c = NULL;
	if (argc >= 2){
		c = getAccelConfigChannel(modp_atoi(argv[1]));
		if (NULL == c) SendCommandError(ERROR_CODE_INVALID_PARAM);
	}
	else{
		SendCommandError(ERROR_CODE_MISSING_PARAMS);
	}
	return c;
}

void SetAccelInstalled(unsigned int argc, char **argv){
	struct LoggerConfig * c = AssertSetBaseParam(argc);
	if (NULL != c){
		c->AccelInstalled = (modp_atoi(argv[1]) != 0);
		SendCommandOK();
	}
}

void GetAccelInstalled(unsigned int argc, char **argv){
	struct LoggerConfig * c = getWorkingLoggerConfig();
	SendNameInt("accelInstalled",c->AccelInstalled);
}

void SetAccelLabel(unsigned int argc, char **argv){
	struct AccelConfig * c = AssertAccelSetParam(argc,argv);
	if (NULL != c){
		setLabelGeneric(c->label,argv[2]);
		SendCommandOK();
	}
}

void GetAccelLabel(unsigned int argc, char **argv){
	struct AccelConfig * c = AssertAccelGetParam(argc,argv);
	if (NULL != c){
		SendNameString("accelLabel",c->label);
	}
}

void SetAccelSampleRate(unsigned int argc, char **argv){
	struct AccelConfig * c = AssertAccelSetParam(argc,argv);
	if (NULL != c){
		c->sampleRate = encodeSampleRate(modp_atoi(argv[2]));
		SendCommandOK();
	}
}

void GetAccelSampleRate(unsigned int argc, char **argv){
	struct AccelConfig * c = AssertAccelGetParam(argc,argv);
	if (NULL != c) SendNameInt("accelSampleRate",decodeSampleRate(c->sampleRate));
}

void SetAccelIdleSampleRate(unsigned int argc, char **argv){
	struct AccelConfig * c = AssertAccelSetParam(argc,argv);
	if (NULL != c){
		c->idleSampleRate = encodeSampleRate(modp_atoi(argv[2]));
		SendCommandOK();
	}
}

void GetAccelIdleSampleRate(unsigned int argc, char **argv){
	struct AccelConfig * c = AssertAccelGetParam(argc,argv);
	if (NULL != c) SendNameInt("accelIdleSampleRate",decodeSampleRate(c->idleSampleRate));
}

void SetAccelConfig(unsigned int argc, char **argv){
	struct AccelConfig * c = AssertAccelSetParam(argc,argv);
	if (NULL != c){
		c->config = filterAccelConfig(modp_atoi(argv[2]));
		SendCommandOK();
	}
}

void GetAccelConfig(unsigned int argc, char **argv){
	struct AccelConfig * c = AssertAccelGetParam(argc,argv);
	if (NULL != c) SendNameInt("accelConfig",c->config);
}

void SetAccelChannel(unsigned int argc, char **argv){
	struct AccelConfig * c = AssertAccelSetParam(argc,argv);
	if (NULL != c){
		c->accelChannel = filterAccelChannel(modp_atoi(argv[2]));
		SendCommandOK();
	}
}

void GetAccelChannel(unsigned int argc, char **argv){
	struct AccelConfig * c = AssertAccelGetParam(argc,argv);
	if (NULL != c) SendNameInt("accelChannel",c->accelChannel);
}

void SetAccelZeroValue(unsigned int argc, char **argv){
	struct AccelConfig * c = AssertAccelSetParam(argc,argv);
	if (NULL != c){
		c->zeroValue = modp_atoi(argv[2]);
		SendCommandOK();
	}
}

void GetAccelZeroValue(unsigned int argc, char **argv){
	struct AccelConfig * c = AssertAccelGetParam(argc,argv);
	if (NULL != c) SendNameInt("zeroValue",c->zeroValue);
}

void CalibrateAccelZero(unsigned int argc, char **argv){
	calibrateAccelZero();
	SendCommandOK();
}

/*
//Logger configuration editing
lua_register(L,"setAccelZeroValue",Lua_SetAccelZeroValue);
*/
