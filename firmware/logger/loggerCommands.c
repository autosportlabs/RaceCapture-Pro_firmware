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

static GPSConfig * getGpsConfig(){
	return &(getWorkingLoggerConfig()->GPSConfig);
}

static void setCmdChannelLabel(unsigned int argc, char **argv, char *label){
	if (argc >= 2){
		setLabelGeneric(label,argv[1]);
		SendCommandOK();
	}
	else{
		SendCommandError(ERROR_CODE_MISSING_PARAMS);
	}
}

static void setSampleRateGeneric(unsigned int argc, char **argv, int *sampleRate){
	if (argc >= 2){
		*sampleRate = encodeSampleRate(modp_atoi(argv[1]));
		SendCommandOK();
	}
	else{
		SendCommandError(ERROR_CODE_MISSING_PARAMS);
	}
}

static void sendSampleRateGeneric(char *name,int sampleRate){
	SendNameInt(name,decodeSampleRate(sampleRate));
}

static LoggerConfig * AssertSetParam(unsigned int argc, unsigned int requiredParams){
	LoggerConfig *c = NULL;
	if (argc >= requiredParams){
		c = getWorkingLoggerConfig();
	}
	else{
		SendCommandError(ERROR_CODE_MISSING_PARAMS);
	}
	return c;
}

static ADCConfig * AssertAdcSetParam(unsigned int argc, char **argv){
	ADCConfig *c = NULL;
	if (argc >= 3){
		c = getADCConfigChannel(modp_atoi(argv[1]));
		if (NULL == c) SendCommandError(ERROR_CODE_INVALID_PARAM);
	}
	else{
		SendCommandError(ERROR_CODE_MISSING_PARAMS);
	}
	return c;
}

static ADCConfig * AssertAdcGetParam(unsigned int argc, char **argv){
	ADCConfig *c = NULL;
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
		ADCConfig *c = getADCConfigChannel(modp_atoi(argv[1]));
		if (NULL != c){
			setLabelGeneric(c->cfg.label,argv[2]);
			setLabelGeneric(c->cfg.units,argv[3]);
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
	ADCConfig * c = AssertAdcGetParam(argc,argv);
	if (NULL != c){
		SendNameString("label",c->cfg.label);
		SendNameString("units",c->cfg.units);
	}

}


void SetAnalogSampleRate(unsigned int argc, char **argv){
	ADCConfig * c = AssertAdcSetParam(argc,argv);
	if (NULL != c){
		c->cfg.sampleRate = encodeSampleRate(modp_atoi(argv[2]));
		SendCommandOK();
	}
}

void GetAnalogSampleRate(unsigned int argc, char **argv){
	ADCConfig * c = AssertAdcGetParam(argc,argv);
	if (NULL != c) SendNameInt("sampleRate",decodeSampleRate(c->cfg.sampleRate));
}

void SetAnalogScaling(unsigned int argc, char **argv){
	ADCConfig * c = AssertAdcSetParam(argc,argv);
	if (NULL != c){
		c->scaling = modp_atof(argv[2]);
		SendCommandOK();
	}
}

void GetAnalogScaling(unsigned int argc, char **argv){
	ADCConfig * c = AssertAdcGetParam(argc,argv);
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

static PWMConfig * AssertPwmSetParam(unsigned int argc, char **argv){
	PWMConfig *c = NULL;
	if (argc >= 3){
		c = getPWMConfigChannel(modp_atoi(argv[1]));
		if (NULL == c) SendCommandError(ERROR_CODE_INVALID_PARAM);
	}
	else{
		SendCommandError(ERROR_CODE_MISSING_PARAMS);
	}
	return c;
}

static PWMConfig * AssertPwmGetParam(unsigned int argc, char **argv){
	PWMConfig *c = NULL;
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
	PWMConfig *c = AssertPwmSetParam(argc,argv);
	if (NULL != c){
		setLabelGeneric(c->cfg.label,argv[2]);
		SendCommandOK();
	}
}

void GetPwmLabel(unsigned int argc, char **argv){
	PWMConfig *c = AssertPwmGetParam(argc,argv);
	if (NULL !=c ) SendNameString("label",c->cfg.label);
}

void SetPwmSampleRate(unsigned int argc, char **argv){
	PWMConfig *c = AssertPwmSetParam(argc,argv);
	if (NULL != c){
		c->cfg.sampleRate = encodeSampleRate(modp_atoi(argv[2]));
		SendCommandOK();
	}
}

void GetPwmSampleRate(unsigned int argc, char **argv){
	PWMConfig *c = AssertPwmGetParam(argc,argv);
	if (NULL !=c ) SendNameInt("sampleRate",decodeSampleRate(c->cfg.sampleRate));
}

void SetPwmOutputCfg(unsigned int argc, char **argv){
	PWMConfig *c = AssertPwmSetParam(argc,argv);
	if (NULL != c){
		c->outputConfig = filterPWMOutputConfig(modp_atoi(argv[2]));
		SendCommandOK();
	}
}

void GetPwmOutputCfg(unsigned int argc, char **argv){
	PWMConfig *c = AssertPwmGetParam(argc,argv);
	if (NULL != c) SendNameInt("outputConfig",c->outputConfig);
}

void SetPwmLoggingCfg(unsigned int argc, char **argv){
	PWMConfig *c = AssertPwmSetParam(argc,argv);
	if (NULL != c) {
		c->loggingConfig = filterPWMLoggingConfig(modp_atoi(argv[2]));
		SendCommandOK();
	}
}

void GetPwmLoggingCfg(unsigned int argc, char **argv){
	PWMConfig *c = AssertPwmGetParam(argc,argv);
	if (NULL !=c ) SendNameInt("loggingConfig",c->loggingConfig);
}

void SetPwmStartupDutyCycle(unsigned int argc, char **argv){
	PWMConfig *c = AssertPwmSetParam(argc,argv);
	if (NULL != c){
		c->startupDutyCycle = filterPWMDutyCycle(modp_atoi(argv[2]));
		SendCommandOK();
	}
}

void GetPwmStartupDutyCycle(unsigned int argc, char **argv){
	PWMConfig *c = AssertPwmGetParam(argc,argv);
	if (NULL != c) SendNameInt("dutyCycle",c->startupDutyCycle);
}

void SetPWMStartupPeriod(unsigned int argc, char **argv){
	PWMConfig *c = AssertPwmSetParam(argc,argv);
	if (NULL != c){
		c->startupPeriod = filterPWMPeriod(modp_atoi(argv[2]));
		SendCommandOK();
	}
}

void GetPwmStartupPeriod(unsigned int argc, char **argv){
	PWMConfig *c = AssertPwmGetParam(argc,argv);
	if (NULL != c) SendNameInt("startupPeriod",c->startupPeriod);
}

void SetPwmVoltageScaling(unsigned int argc, char **argv){
	PWMConfig *c = AssertPwmSetParam(argc,argv);
	if (NULL != c){
		c->voltageScaling = modp_atof(argv[2]);
		SendCommandOK();
	}
}

void getPwmVoltageScaling(unsigned int argc, char **argv){
	PWMConfig *c = AssertPwmGetParam(argc,argv);
	if (NULL == c) return;
	SendNameFloat("voltageScaling",c->voltageScaling,2);
}

static GPSConfig * AssertSetGpsParam(unsigned int argc){
	if (argc >= 2){
		return &(getWorkingLoggerConfig()->GPSConfig);
	}
	else{
		SendCommandError(ERROR_CODE_MISSING_PARAMS);
	}
	return NULL;
}

void SetGpsInstalled(unsigned int argc, char **argv){
	GPSConfig * c = AssertSetGpsParam(argc);
	if (NULL != c){
		c->GPSInstalled = (modp_atoi(argv[1]) != 0);
		SendCommandOK();
	}
}

void getGpsInstalled(unsigned int argc, char **argv){
	SendNameInt("gpsInstalled",getGpsConfig()->GPSInstalled);
}

void SetGpsStartFinish(unsigned int argc, char **argv){
	LoggerConfig *c = AssertSetParam(argc,4);
	if (NULL != c){
		c->GPSConfig.startFinishLatitude = modp_atof(argv[1]);
		c->GPSConfig.startFinishLongitude = modp_atof(argv[2]);
		if (argc > 3) c->GPSConfig.startFinishRadius = modp_atof(argv[3]);
		SendCommandOK();
	}
}

void GetGpsStartFinish(unsigned int argc, char **argv){
	SendNameFloat("gpsStartFinishLatitude",getWorkingLoggerConfig()->GPSConfig.startFinishLatitude,DEFAULT_GPS_POSITION_LOGGING_PRECISION);
	SendNameFloat("gpsStartFinishLongitude", getWorkingLoggerConfig()->GPSConfig.startFinishLongitude,DEFAULT_GPS_POSITION_LOGGING_PRECISION);
	SendNameFloat("gpsStartFinishRadius",getWorkingLoggerConfig()->GPSConfig.startFinishRadius,DEFAULT_GPS_RADIUS_LOGGING_PRECISION);
}

void SetGpsQualityLabel(unsigned int argc, char **argv){
	GPSConfig * c = AssertSetGpsParam(argc);
	if (NULL != c){
		setLabelGeneric(c->qualityCfg.label,argv[1]);
		SendCommandOK();
	}
}

void GetGpsQualityLabel(unsigned int argc, char **argv){
	SendNameString("gpsQualityLabel", getGpsConfig()->qualityCfg.label);
}

void SetGpsStatsLabel(unsigned int argc, char **argv){
	GPSConfig * c = AssertSetGpsParam(argc);
	if (NULL != c){
		setLabelGeneric(c->satellitesCfg.label, argv[1]);
		SendCommandOK();
	}
}

void GetGpsStatsLabel(unsigned int argc, char **argv){
	SendNameString("gpsStatsLabel", getWorkingLoggerConfig()->GPSConfig.satellitesCfg.label);
}

void SetGpsLatitudeLabel(unsigned int argc, char **argv){
	GPSConfig * c = AssertSetGpsParam(argc);
	if (NULL != c){
		setLabelGeneric(c->latitudeCfg.label, argv[1]);
		SendCommandOK();
	}
}

void GetGpsLatitudeLabel(unsigned int argc, char **argv){
	SendNameString("gpsLatitudeLabel", getWorkingLoggerConfig()->GPSConfig.latitudeCfg.label);
}

void SetGpsLongitudeLabel(unsigned int argc, char **argv){
	GPSConfig *c = AssertSetGpsParam(argc);
	if (NULL != c){
		setLabelGeneric(c->longitudeCfg.label, argv[1]);
		SendCommandOK();
	}
}

void GetGpsLongitudeLabel(unsigned int argc, char **argv){
	SendNameString("gpsLongitudeLabel", getWorkingLoggerConfig()->GPSConfig.longitudeCfg.label);
}

void SetGpsTimeLabel(unsigned int argc, char **argv){
	GPSConfig *c = AssertSetGpsParam(argc);
	if (NULL != c){
		setLabelGeneric(c->timeCfg.label, argv[1]);
		SendCommandOK();
	}
}

void GetGpsTimeLabel(unsigned int argc, char **argv){
	SendNameString("gpsTimeLabel", getWorkingLoggerConfig()->GPSConfig.timeCfg.label);
}

void SetLapCountLabel(unsigned int argc, char **argv){
	setCmdChannelLabel(argc,argv,getGpsConfig()->lapCountCfg.label);
}

void GetLapCountLabel(unsigned int argc, char **argv){
	SendNameString("lapCountLabel",getGpsConfig()->lapCountCfg.label);
}

void SetLapCountSampleRate(unsigned int argc, char **argv){
	setSampleRateGeneric(argc,argv,&(getGpsConfig()->lapCountCfg.sampleRate));
}

void GetLapCountSampleRate(unsigned int argc, char **argv){
	sendSampleRateGeneric("lapCountSampleRate", getGpsConfig()->lapCountCfg.sampleRate);
}

void SetLapTimeLabel(unsigned int argc, char **argv){
	setCmdChannelLabel(argc,argv,getGpsConfig()->lapTimeCfg.label);
}

void GetLapTimeLabel(unsigned int argc, char **argv){
	SendNameString("lapTimeLabel",getGpsConfig()->lapTimeCfg.label);
}

void SetLapTimeSampleRate(unsigned int argc, char **argv){
	setSampleRateGeneric(argc,argv,&(getGpsConfig()->lapTimeCfg.sampleRate));
}

void GetLapTimeSampleRate(unsigned int argc, char **argv){
	sendSampleRateGeneric("lapTimeSampleRate",getGpsConfig()->lapTimeCfg.sampleRate);
}

void SetGpsVelocityLabel(unsigned int argc, char **argv){
	GPSConfig * c = AssertSetGpsParam(argc);
	if (NULL != c){
		setLabelGeneric(c->velocityCfg.label, argv[1]);
		SendCommandOK();
	}
}

void GetGpsVelocityLabel(unsigned int argc, char **argv){
	SendNameString("gpsVelocityLabel", getWorkingLoggerConfig()->GPSConfig.velocityCfg.label);
}

void SetGpsPositionSampleRate(unsigned int argc, char **argv){
	GPSConfig *c = AssertSetGpsParam(argc);
	if (NULL != c){
		c->latitudeCfg.sampleRate = c->longitudeCfg.sampleRate = encodeSampleRate(modp_atoi(argv[1]));
		SendCommandOK();
	}
}

void GetGpsPositionSampleRate(unsigned int argc, char **argv){
	//TODO we pull one to represent all... is there a better way? individual settable sample rates?
	SendNameInt("gpsPositionSampleRate",decodeSampleRate(getWorkingLoggerConfig()->GPSConfig.latitudeCfg.sampleRate));
}

void SetGpsVelocitySampleRate(unsigned int argc, char **argv){
	GPSConfig *c = AssertSetGpsParam(argc);
	if (NULL != c){
		c->velocityCfg.sampleRate = encodeSampleRate(modp_atoi(argv[1]));
		SendCommandOK();
	}
}

void GetGpsVelocitySampleRate(unsigned int argc, char **argv){
	SendNameInt("gpsVelocitySampleRate",decodeSampleRate(getWorkingLoggerConfig()->GPSConfig.velocityCfg.sampleRate));
}

void SetGpsTimeSampleRate(unsigned int argc, char **argv){
	GPSConfig *c = AssertSetGpsParam(argc);
	if (NULL != c){
		c->timeCfg.sampleRate = encodeSampleRate(modp_atoi(argv[1]));
		SendCommandOK();
	}
}

void GetGpsTimeSampleRate(unsigned int argc, char **argv){
	SendNameInt("gpsTimeSampleRate", decodeSampleRate(getWorkingLoggerConfig()->GPSConfig.timeCfg.sampleRate));
}

GPIOConfig * AssertGpioSetParam(unsigned int argc, char **argv){
	GPIOConfig *c = NULL;
	if (argc >= 3){
		c = getGPIOConfigChannel(modp_atoi(argv[1]));
		if (NULL == c) SendCommandError(ERROR_CODE_INVALID_PARAM);
	}
	else{
		SendCommandError(ERROR_CODE_MISSING_PARAMS);
	}
	return c;
}

GPIOConfig * AssertGpioGetParam(unsigned int argc, char **argv){
	GPIOConfig *c = NULL;
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
	GPIOConfig * c = AssertGpioSetParam(argc,argv);
	if (NULL != c){
		setLabelGeneric(c->cfg.label,argv[2]);
		SendCommandOK();
	}
}

void GetGpioLabel(unsigned int argc, char **argv){
	GPIOConfig * c = AssertGpioGetParam(argc,argv);
	if (NULL != c) SendNameString("gpioLabel",c->cfg.label);
}

void SetGpioSampleRate(unsigned int argc, char **argv){
	GPIOConfig * c = AssertGpioSetParam(argc,argv);
	if (NULL != c){
		c->cfg.sampleRate = encodeSampleRate(modp_atoi(argv[2]));
		SendCommandOK();
	}
}

void GetGpioSampleRate(unsigned int argc, char **argv){
	GPIOConfig * c = AssertGpioGetParam(argc,argv);
	if (NULL != c) SendNameInt("gpioSampleRate",decodeSampleRate(c->cfg.sampleRate));
}

void SetGpioConfig(unsigned int argc, char **argv){
	GPIOConfig * c = AssertGpioSetParam(argc,argv);
	if (NULL != c){
		c->config = filterGPIOConfig(modp_atoi(argv[2]));
		SendCommandOK();
	}
}

void GetGpioConfig(unsigned int argc, char **argv){
	GPIOConfig * c = AssertGpioGetParam(argc,argv);
	if (NULL != c) SendNameInt("gpioConfig",c->config);
}

TimerConfig * AssertTimerSetParam(unsigned int argc, char **argv){
	TimerConfig *c = NULL;
	if (argc >= 3){
		c = getTimerConfigChannel(modp_atoi(argv[1]));
		if (NULL == c) SendCommandError(ERROR_CODE_INVALID_PARAM);
	}
	else{
		SendCommandError(ERROR_CODE_MISSING_PARAMS);
	}
	return c;
}

TimerConfig* AssertTimerGetParam(unsigned int argc, char **argv){
	TimerConfig *c = NULL;
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
	TimerConfig * c = AssertTimerSetParam(argc,argv);
	if (NULL != c){
		setLabelGeneric(c->cfg.label,argv[2]);
		SendCommandOK();
	}
}

void GetTimerLabel(unsigned int argc, char **argv){
	TimerConfig * c = AssertTimerGetParam(argc,argv);
	if (NULL !=c) SendNameString("timerLabel",c->cfg.label);
}

void SetTimerSampleRate(unsigned int argc, char **argv){
	TimerConfig * c = AssertTimerSetParam(argc,argv);
	if (NULL != c){
		c->cfg.sampleRate = encodeSampleRate(modp_atoi(argv[2]));
		SendCommandOK();
	}
}

void GetTimerSampleRate(unsigned int argc, char **argv){
	TimerConfig * c = AssertTimerGetParam(argc,argv);
	if (NULL != c) SendNameInt("timerSampleRate", decodeSampleRate(c->cfg.sampleRate));
}

void SetTimerCfg(unsigned int argc, char **argv){
	TimerConfig * c = AssertTimerSetParam(argc,argv);
	if (NULL != c){
		c->config = filterTimerConfig(modp_atoi(argv[2]));
		SendCommandOK();
	}
}

void GetTimerCfg(unsigned int argc, char **argv){
	TimerConfig * c = AssertTimerGetParam(argc,argv);
	if (NULL != c) SendNameInt("timerConfig", c->config);
}

void SetTimerPulsePerRev(unsigned int argc, char **argv){
	TimerConfig * c = AssertTimerSetParam(argc,argv);
	if (NULL != c){
		c->pulsePerRevolution = modp_atoi(argv[2]);
		SendCommandOK();
	}
}

void GetTimerPulsePerRev(unsigned int argc, char **argv){
	TimerConfig * c = AssertTimerGetParam(argc,argv);
	if (NULL != c){
		SendNameInt("timerPulsePerRev",c->pulsePerRevolution);
	}
}

void SetTimerDivider(unsigned int argc, char **argv){
	TimerConfig * c = AssertTimerSetParam(argc,argv);
	if (NULL != c){
		c->timerDivider = filterTimerDivider(modp_atoi(argv[2]));
		SendCommandOK();
	}
}

void GetTimerDivider(unsigned int argc, char **argv){
	TimerConfig * c = AssertTimerGetParam(argc,argv);
	if (NULL != c){
		SendNameInt("timerDivider",c->timerDivider);
	}
}

void CalculateTimerScaling(unsigned int argc, char **argv){
	TimerConfig * c = AssertTimerGetParam(argc,argv);
	if (NULL != c){
		calculateTimerScaling(c);
		SendCommandOK();
	}
}

void GetTimerScaling(unsigned int argc, char **argv){
	TimerConfig * c = AssertTimerGetParam(argc,argv);
	if (NULL != c){
		SendNameInt("timerScaling",c->calculatedScaling);
		SendCommandOK();
	}
}

AccelConfig * AssertAccelSetParam(unsigned int argc, char **argv){
	AccelConfig *c = NULL;
	if (argc >= 3){
		c = getAccelConfigChannel(modp_atoi(argv[1]));
		if (NULL == c) SendCommandError(ERROR_CODE_INVALID_PARAM);
	}
	else{
		SendCommandError(ERROR_CODE_MISSING_PARAMS);
	}
	return c;
}

AccelConfig * AssertAccelGetParam(unsigned int argc, char **argv){
	AccelConfig *c = NULL;
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
	LoggerConfig * c = AssertSetParam(argc,2);
	if (NULL != c){
		c->AccelInstalled = (modp_atoi(argv[1]) != 0);
		SendCommandOK();
	}
}

void GetAccelInstalled(unsigned int argc, char **argv){
	LoggerConfig * c = getWorkingLoggerConfig();
	SendNameInt("accelInstalled",c->AccelInstalled);
}

void SetAccelLabel(unsigned int argc, char **argv){
	AccelConfig * c = AssertAccelSetParam(argc,argv);
	if (NULL != c){
		setLabelGeneric(c->cfg.label,argv[2]);
		SendCommandOK();
	}
}

void GetAccelLabel(unsigned int argc, char **argv){
	AccelConfig * c = AssertAccelGetParam(argc,argv);
	if (NULL != c){
		SendNameString("accelLabel",c->cfg.label);
	}
}

void SetAccelSampleRate(unsigned int argc, char **argv){
	AccelConfig * c = AssertAccelSetParam(argc,argv);
	if (NULL != c){
		c->cfg.sampleRate = encodeSampleRate(modp_atoi(argv[2]));
		SendCommandOK();
	}
}

void GetAccelSampleRate(unsigned int argc, char **argv){
	AccelConfig * c = AssertAccelGetParam(argc,argv);
	if (NULL != c) SendNameInt("accelSampleRate",decodeSampleRate(c->cfg.sampleRate));
}


void SetAccelConfig(unsigned int argc, char **argv){
	AccelConfig * c = AssertAccelSetParam(argc,argv);
	if (NULL != c){
		c->config = filterAccelConfig(modp_atoi(argv[2]));
		SendCommandOK();
	}
}

void GetAccelConfig(unsigned int argc, char **argv){
	AccelConfig * c = AssertAccelGetParam(argc,argv);
	if (NULL != c) SendNameInt("accelConfig",c->config);
}

void SetAccelChannel(unsigned int argc, char **argv){
	AccelConfig * c = AssertAccelSetParam(argc,argv);
	if (NULL != c){
		c->accelChannel = filterAccelChannel(modp_atoi(argv[2]));
		SendCommandOK();
	}
}

void GetAccelChannel(unsigned int argc, char **argv){
	AccelConfig * c = AssertAccelGetParam(argc,argv);
	if (NULL != c) SendNameInt("accelChannel",c->accelChannel);
}

void SetAccelZeroValue(unsigned int argc, char **argv){
	AccelConfig * c = AssertAccelSetParam(argc,argv);
	if (NULL != c){
		c->zeroValue = modp_atoi(argv[2]);
		SendCommandOK();
	}
}

void GetAccelZeroValue(unsigned int argc, char **argv){
	AccelConfig * c = AssertAccelGetParam(argc,argv);
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
