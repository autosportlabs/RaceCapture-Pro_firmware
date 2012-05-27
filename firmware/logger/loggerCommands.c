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

static void setCharValueGeneric(unsigned int argc, char **argv, char *value){
	if (argc >=2){
		*value = (char)modp_atoi(argv[1]);
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

static void SendChannelConfigSuffix(ChannelConfig *c, const char *suf){
	if (NULL != c){
		SendNameSuffixString("label",suf,c->label);
		SendNameSuffixString("units",suf,c->units);
		SendNameSuffixInt("sampleRate",suf,decodeSampleRate(c->sampleRate));
	}
}

static void SendChannelConfig(ChannelConfig *c){
	if (NULL != c){
		SendNameString("label",c->label);
		SendNameString("units",c->units);
		SendNameInt("sampleRate",decodeSampleRate(c->sampleRate));
	}
}

void GetAnalogConfig(unsigned int argc, char **argv){
	ADCConfig * c = AssertAdcGetParam(argc,argv);
	if (NULL != c){
		SendChannelConfig(&(c->cfg));
		SendNameInt("logPrec",c->loggingPrecision);
		SendNameInt("scalingMode",c->scalingMode);
		SendNameFloat("scaling",c->linearScaling,ANALOG_SCALING_PRECISION);
		for (int i = 0; i < ANALOG_SCALING_BINS; i++){
			SendNameIndexInt("mapRaw",i,c->scalingMap.rawValues[i]);
		}
		for (int i = 0; i < ANALOG_SCALING_BINS; i++){
			SendNameIndexFloat("mapScaled",i,c->scalingMap.scaledValues[i],ANALOG_SCALING_PRECISION);
		}
	}
}

void SetAnalogConfig(unsigned int argc, char **argv){
	ADCConfig * c = AssertAdcGetParam(argc,argv);
	if (NULL != c){
		if (argc > 8 && argc < 18){
			SendCommandError(ERROR_CODE_MISSING_PARAMS);
			return;
		}
		if (argc > 2) setLabelGeneric(c->cfg.label,argv[2]);
		if (argc > 3) setLabelGeneric(c->cfg.units,argv[3]);
		if (argc > 4) c->cfg.sampleRate = encodeSampleRate(modp_atoi(argv[4]));
		if (argc > 5) c->loggingPrecision = modp_atoi(argv[5]);
		if (argc > 6) c->scalingMode = filterAnalogScalingMode(modp_atoi(argv[6]));
		if (argc > 7) c->linearScaling = modp_atof(argv[7]);
		if (argc > 17){
			for (int i = 8; i < 13; i++){
				c->scalingMap.rawValues[i-8]  = modp_atoi(argv[i]);
			}
			for (int i = 13; i < 18; i++){
				c->scalingMap.scaledValues[i-13] = modp_atof(argv[i]);
			}
		}
		SendCommandOK();
	}
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

void GetPwmConfig(unsigned int argc, char **argv){
	PWMConfig *c = AssertPwmGetParam(argc,argv);
	if (NULL != c){
		SendChannelConfig(&(c->cfg));
		SendNameInt("logPrec",c->loggingPrecision);
		SendNameInt("outputMode",c->outputMode);
		SendNameInt("loggingMode",c->loggingMode);
		SendNameInt("startupDutyCycle",c->startupDutyCycle);
		SendNameInt("startupPeriod",c->startupPeriod);
		SendNameFloat("voltageScaling",c->voltageScaling,VOLTAGE_SCALING_PRECISION);
	}
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

void SetPwmOutputMode(unsigned int argc, char **argv){
	PWMConfig *c = AssertPwmSetParam(argc,argv);
	if (NULL != c){
		c->outputMode = filterPWMOutputConfig(modp_atoi(argv[2]));
		SendCommandOK();
	}
}

void GetPwmOutputMode(unsigned int argc, char **argv){
	PWMConfig *c = AssertPwmGetParam(argc,argv);
	if (NULL != c) SendNameInt("outputMode",c->outputMode);
}

void SetPwmLoggingMode(unsigned int argc, char **argv){
	PWMConfig *c = AssertPwmSetParam(argc,argv);
	if (NULL != c) {
		c->loggingMode = filterPWMLoggingConfig(modp_atoi(argv[2]));
		SendCommandOK();
	}
}

void GetPwmLoggingMode(unsigned int argc, char **argv){
	PWMConfig *c = AssertPwmGetParam(argc,argv);
	if (NULL !=c ) SendNameInt("loggingMode",c->loggingMode);
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
	SendNameFloat("voltageScaling",c->voltageScaling,VOLTAGE_SCALING_PRECISION);
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

void GetGpsConfig(unsigned int argc, char **argv){
	GPSConfig *cfg = &(getWorkingLoggerConfig()->GPSConfig);
	SendNameInt("installed",cfg->GPSInstalled);
	SendChannelConfigSuffix(&(cfg->latitudeCfg),"lat");
	SendChannelConfigSuffix(&(cfg->longitudeCfg),"long");
	SendChannelConfigSuffix(&(cfg->velocityCfg),"vel");
	SendChannelConfigSuffix(&(cfg->timeCfg),"time");
	SendChannelConfigSuffix(&(cfg->qualityCfg),"qual");
	SendChannelConfigSuffix(&(cfg->satellitesCfg),"sats");
	SendChannelConfigSuffix(&(cfg->lapCountCfg),"lapCount");
	SendChannelConfigSuffix(&(cfg->lapTimeCfg),"lapTime");
	SendNameFloat("startFinishLat",cfg->startFinishLatitude,DEFAULT_GPS_POSITION_LOGGING_PRECISION);
	SendNameFloat("startFinishLong", cfg->startFinishLongitude,DEFAULT_GPS_POSITION_LOGGING_PRECISION);
	SendNameFloat("startFinishRadius",cfg->startFinishRadius,DEFAULT_GPS_RADIUS_LOGGING_PRECISION);
}

void SetGpsInstalled(unsigned int argc, char **argv){
	GPSConfig * c = AssertSetGpsParam(argc);
	if (NULL != c){
		c->GPSInstalled = (modp_atoi(argv[1]) != 0);
		SendCommandOK();
	}
}

void getGpsInstalled(unsigned int argc, char **argv){
	SendNameInt("installed",getGpsConfig()->GPSInstalled);
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
	SendNameFloat("startFinishLat",getWorkingLoggerConfig()->GPSConfig.startFinishLatitude,DEFAULT_GPS_POSITION_LOGGING_PRECISION);
	SendNameFloat("startFinishLong", getWorkingLoggerConfig()->GPSConfig.startFinishLongitude,DEFAULT_GPS_POSITION_LOGGING_PRECISION);
	SendNameFloat("startFinishRadius",getWorkingLoggerConfig()->GPSConfig.startFinishRadius,DEFAULT_GPS_RADIUS_LOGGING_PRECISION);
}

void SetGpsQualityLabel(unsigned int argc, char **argv){
	GPSConfig * c = AssertSetGpsParam(argc);
	if (NULL != c){
		setLabelGeneric(c->qualityCfg.label,argv[1]);
		SendCommandOK();
	}
}

void GetGpsQualityLabel(unsigned int argc, char **argv){
	SendNameString("label", getGpsConfig()->qualityCfg.label);
}

void SetGpsStatsLabel(unsigned int argc, char **argv){
	GPSConfig * c = AssertSetGpsParam(argc);
	if (NULL != c){
		setLabelGeneric(c->satellitesCfg.label, argv[1]);
		SendCommandOK();
	}
}

void GetGpsStatsLabel(unsigned int argc, char **argv){
	SendNameString("label", getWorkingLoggerConfig()->GPSConfig.satellitesCfg.label);
}

void SetGpsLatitudeLabel(unsigned int argc, char **argv){
	GPSConfig * c = AssertSetGpsParam(argc);
	if (NULL != c){
		setLabelGeneric(c->latitudeCfg.label, argv[1]);
		SendCommandOK();
	}
}

void GetGpsLatitudeLabel(unsigned int argc, char **argv){
	SendNameString("label", getWorkingLoggerConfig()->GPSConfig.latitudeCfg.label);
}

void SetGpsLongitudeLabel(unsigned int argc, char **argv){
	GPSConfig *c = AssertSetGpsParam(argc);
	if (NULL != c){
		setLabelGeneric(c->longitudeCfg.label, argv[1]);
		SendCommandOK();
	}
}

void GetGpsLongitudeLabel(unsigned int argc, char **argv){
	SendNameString("label", getWorkingLoggerConfig()->GPSConfig.longitudeCfg.label);
}

void SetGpsTimeLabel(unsigned int argc, char **argv){
	GPSConfig *c = AssertSetGpsParam(argc);
	if (NULL != c){
		setLabelGeneric(c->timeCfg.label, argv[1]);
		SendCommandOK();
	}
}

void GetGpsTimeLabel(unsigned int argc, char **argv){
	SendNameString("label", getWorkingLoggerConfig()->GPSConfig.timeCfg.label);
}

void SetLapCountLabel(unsigned int argc, char **argv){
	setCmdChannelLabel(argc,argv,getGpsConfig()->lapCountCfg.label);
}

void GetLapCountLabel(unsigned int argc, char **argv){
	SendNameString("label",getGpsConfig()->lapCountCfg.label);
}

void SetLapCountSampleRate(unsigned int argc, char **argv){
	setSampleRateGeneric(argc,argv,&(getGpsConfig()->lapCountCfg.sampleRate));
}

void GetLapCountSampleRate(unsigned int argc, char **argv){
	sendSampleRateGeneric("sampleRate", getGpsConfig()->lapCountCfg.sampleRate);
}

void SetLapTimeLabel(unsigned int argc, char **argv){
	setCmdChannelLabel(argc,argv,getGpsConfig()->lapTimeCfg.label);
}

void GetLapTimeLabel(unsigned int argc, char **argv){
	SendNameString("label",getGpsConfig()->lapTimeCfg.label);
}

void SetLapTimeSampleRate(unsigned int argc, char **argv){
	setSampleRateGeneric(argc,argv,&(getGpsConfig()->lapTimeCfg.sampleRate));
}

void GetLapTimeSampleRate(unsigned int argc, char **argv){
	sendSampleRateGeneric("sampleRate",getGpsConfig()->lapTimeCfg.sampleRate);
}

void SetGpsVelocityLabel(unsigned int argc, char **argv){
	GPSConfig * c = AssertSetGpsParam(argc);
	if (NULL != c){
		setLabelGeneric(c->velocityCfg.label, argv[1]);
		SendCommandOK();
	}
}

void GetGpsVelocityLabel(unsigned int argc, char **argv){
	SendNameString("label", getWorkingLoggerConfig()->GPSConfig.velocityCfg.label);
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
	SendNameInt("sampleRate",decodeSampleRate(getWorkingLoggerConfig()->GPSConfig.latitudeCfg.sampleRate));
}

void SetGpsVelocitySampleRate(unsigned int argc, char **argv){
	GPSConfig *c = AssertSetGpsParam(argc);
	if (NULL != c){
		c->velocityCfg.sampleRate = encodeSampleRate(modp_atoi(argv[1]));
		SendCommandOK();
	}
}

void GetGpsVelocitySampleRate(unsigned int argc, char **argv){
	SendNameInt("sampleRate",decodeSampleRate(getWorkingLoggerConfig()->GPSConfig.velocityCfg.sampleRate));
}

void SetGpsTimeSampleRate(unsigned int argc, char **argv){
	GPSConfig *c = AssertSetGpsParam(argc);
	if (NULL != c){
		c->timeCfg.sampleRate = encodeSampleRate(modp_atoi(argv[1]));
		SendCommandOK();
	}
}

void GetGpsTimeSampleRate(unsigned int argc, char **argv){
	SendNameInt("sampleRate", decodeSampleRate(getWorkingLoggerConfig()->GPSConfig.timeCfg.sampleRate));
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

void GetGpioConfig(unsigned int argc, char **argv){
	GPIOConfig *c = AssertGpioGetParam(argc,argv);
	if (NULL != c){
		SendChannelConfig(&(c->cfg));
		SendNameInt("mode",c->mode);
	}
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
	if (NULL != c) SendNameString("label",c->cfg.label);
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
	if (NULL != c) SendNameInt("sampleRate",decodeSampleRate(c->cfg.sampleRate));
}

void SetGpioMode(unsigned int argc, char **argv){
	GPIOConfig * c = AssertGpioSetParam(argc,argv);
	if (NULL != c){
		c->mode = filterGPIOMode(modp_atoi(argv[2]));
		SendCommandOK();
	}
}

void GetGpioMode(unsigned int argc, char **argv){
	GPIOConfig * c = AssertGpioGetParam(argc,argv);
	if (NULL != c) SendNameInt("mode",c->mode);
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

void GetTimerConfig(unsigned int argc, char **argv){
	TimerConfig * c = AssertTimerGetParam(argc,argv);
	if (NULL != c){
		SendChannelConfig(&(c->cfg));
		SendNameInt("logPrec",c->loggingPrecision);
		SendNameInt("slowTimer",c->slowTimerEnabled);
		SendNameInt("mode",c->mode);
		SendNameInt("pulsePerRev",c->pulsePerRevolution);
		SendNameInt("divider",c->timerDivider);
		SendNameInt("scaling",c->calculatedScaling);
	}
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
	if (NULL !=c) SendNameString("label",c->cfg.label);
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
	if (NULL != c) SendNameInt("sampleRate", decodeSampleRate(c->cfg.sampleRate));
}

void SetTimerMode(unsigned int argc, char **argv){
	TimerConfig * c = AssertTimerSetParam(argc,argv);
	if (NULL != c){
		c->mode = filterTimerMode(modp_atoi(argv[2]));
		SendCommandOK();
	}
}

void GetTimerMode(unsigned int argc, char **argv){
	TimerConfig * c = AssertTimerGetParam(argc,argv);
	if (NULL != c) SendNameInt("mode", c->mode);
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
		SendNameInt("pulsePerRev",c->pulsePerRevolution);
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
		SendNameInt("divider",c->timerDivider);
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
		SendNameInt("scaling",c->calculatedScaling);
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
	SendNameInt("installed",c->AccelInstalled);
}

void GetAccelConfig(unsigned int argc, char **argv){
	AccelConfig * c= AssertAccelGetParam(argc,argv);
	if (NULL != c){
		SendChannelConfig(&(c->cfg));
		SendNameInt("mode",c->mode);
		SendNameInt("channel",c->accelChannel);
		SendNameInt("zeroValue",c->zeroValue);
	}
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
		SendNameString("label",c->cfg.label);
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
	if (NULL != c) SendNameInt("sampleRate",decodeSampleRate(c->cfg.sampleRate));
}


void SetAccelMode(unsigned int argc, char **argv){
	AccelConfig * c = AssertAccelSetParam(argc,argv);
	if (NULL != c){
		c->mode = filterAccelMode(modp_atoi(argv[2]));
		SendCommandOK();
	}
}

void GetAccelMode(unsigned int argc, char **argv){
	AccelConfig * c = AssertAccelGetParam(argc,argv);
	if (NULL != c) SendNameInt("mode",c->mode);
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
	if (NULL != c) SendNameInt("channel",c->accelChannel);
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

void GetLoggerOutputConfig(unsigned int argc, char **argv){
	LoggerOutputConfig *c = &(getWorkingLoggerConfig()->LoggerOutputConfig);
	SendNameInt("sdLoggingMode",c->sdLoggingMode);
	SendNameInt("telemetryMode",c->telemetryMode);
	SendNameUint("p2pDestAddrHigh",c->p2pDestinationAddrHigh);
	SendNameUint("p2pDestAddrLow",c->p2pDestinationAddrLow);
}

void SetLoggerOutputConfig(unsigned int argc, char **argv){
	LoggerOutputConfig *c = &(getWorkingLoggerConfig()->LoggerOutputConfig);
	if (argc > 1){
		c->sdLoggingMode = filterSdLoggingMode((char)modp_atoui(argv[1]));
	}
	if (argc > 2){
		c->telemetryMode = filterTelemetryMode((char)modp_atoui(argv[2]));
	}
	if (argc > 3){
		c->p2pDestinationAddrHigh = modp_atoui(argv[3]);
	}
	if (argc > 4){
		c->p2pDestinationAddrLow = modp_atoui(argv[4]);
	}
	SendCommandOK();
}


/*
//Logger configuration editing
lua_register(L,"setAccelZeroValue",Lua_SetAccelZeroValue);
*/
