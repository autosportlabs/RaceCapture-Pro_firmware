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

static void SetChannelConfig(ChannelConfig *c, unsigned int offset, unsigned int argc, char **argv){
	if (argc > offset) setLabelGeneric(c->label,argv[offset]);
	if (argc > offset + 1) setLabelGeneric(c->units,argv[offset + 1]);
	if (argc > offset + 2) c->sampleRate = encodeSampleRate(modp_atoi(argv[offset + 2]));
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
		SendNameInt("loggingPrecision",c->loggingPrecision);
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
		getWorkingLoggerConfig()->PWMClockFrequency = filterPwmClockFrequency(modp_atoi(argv[1]));
		SendCommandOK();
	}
	else{
		SendCommandError(ERROR_CODE_MISSING_PARAMS);
	}
}

void GetPwmClockFreq(unsigned int argc, char **argv){
	SendNameInt("frequency",getWorkingLoggerConfig()->PWMClockFrequency);
}

static PWMConfig * AssertPwmGetParam(unsigned int argc, char **argv){
	PWMConfig *c = NULL;
	if (argc >= 2){
		c = getPwmConfigChannel(modp_atoi(argv[1]));
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
		SendNameInt("loggingPrecision",c->loggingPrecision);
		SendNameInt("outputMode",c->outputMode);
		SendNameInt("loggingMode",c->loggingMode);
		SendNameInt("startupDutyCycle",c->startupDutyCycle);
		SendNameInt("startupPeriod",c->startupPeriod);
		SendNameFloat("voltageScaling",c->voltageScaling,VOLTAGE_SCALING_PRECISION);
	}
}

void SetPwmConfig(unsigned int argc, char **argv){
	PWMConfig *c = AssertPwmGetParam(argc,argv);
	if (NULL != c){
		if (argc > 2) setLabelGeneric(c->cfg.label,argv[2]);
		if (argc > 3) setLabelGeneric(c->cfg.units,argv[3]);
		if (argc > 4) c->cfg.sampleRate = encodeSampleRate(modp_atoi(argv[4]));
		if (argc > 5) c->loggingPrecision = modp_atoi(argv[5]);
		if (argc > 6) c->outputMode = filterPwmOutputMode(modp_atoi(argv[6]));
		if (argc > 7) c->loggingMode = filterPwmLoggingMode(modp_atoi(argv[7]));
		if (argc > 8) c->startupDutyCycle = filterPwmDutyCycle(modp_atoi(argv[8]));
		if (argc > 9) c->startupPeriod = filterPwmPeriod(modp_atoi(argv[9]));
		if (argc > 10) c->voltageScaling = modp_atof(argv[10]);
		SendCommandOK();
	}
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
}

void SetGpsConfig(unsigned int argc, char **argv){
	GPSConfig *cfg = &(getWorkingLoggerConfig()->GPSConfig);
	if (argc > 1) cfg->GPSInstalled = (modp_atoi(argv[1]) != 0);
	if (argc > 2) SetChannelConfig(&(cfg->latitudeCfg),2,argc,argv);
	if (argc > 5) SetChannelConfig(&(cfg->longitudeCfg),5,argc,argv);
	if (argc > 8) SetChannelConfig(&(cfg->velocityCfg),8,argc,argv);
	if (argc > 11) SetChannelConfig(&(cfg->timeCfg),11,argc,argv);
	if (argc > 14) SetChannelConfig(&(cfg->qualityCfg),14,argc,argv);
	if (argc > 17) SetChannelConfig(&(cfg->satellitesCfg),17,argc,argv);
	SendCommandOK();
}

void GetStartFinishConfig(unsigned int argc, char **argv){
	GPSConfig *cfg = &(getWorkingLoggerConfig()->GPSConfig);
	SendChannelConfigSuffix(&(cfg->lapCountCfg),"lapCount");
	SendChannelConfigSuffix(&(cfg->lapTimeCfg),"lapTime");
	SendNameFloat("startFinishLat",cfg->startFinishLatitude,DEFAULT_GPS_POSITION_LOGGING_PRECISION);
	SendNameFloat("startFinishLong", cfg->startFinishLongitude,DEFAULT_GPS_POSITION_LOGGING_PRECISION);
	SendNameFloat("startFinishRadius",cfg->startFinishRadius,DEFAULT_GPS_RADIUS_LOGGING_PRECISION);
}

void SetStartFinishConfig(unsigned int argc, char **argv){
	GPSConfig *cfg = &(getWorkingLoggerConfig()->GPSConfig);
	if (argc > 1) SetChannelConfig(&(cfg->lapCountCfg),1,argc,argv);
	if (argc > 4) SetChannelConfig(&(cfg->lapTimeCfg),4,argc,argv);
	if (argc > 7) cfg->startFinishLatitude = modp_atof(argv[7]);
	if (argc > 8) cfg->startFinishLongitude = modp_atof(argv[8]);
	if (argc > 9) cfg->startFinishRadius = modp_atof(argv[9]);
	SendCommandOK();
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

void SetGpioConfig(unsigned int argc, char **argv){
	GPIOConfig *c = AssertGpioGetParam(argc,argv);
	if (NULL != c){
		SetChannelConfig(&(c->cfg),2,argc,argv);
		if (argc > 5) c->mode = filterGpioMode(modp_atoi(argv[5]));
		SendCommandOK();
	}
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
		SendNameInt("loggingPrecision",c->loggingPrecision);
		SendNameInt("slowTimer",c->slowTimerEnabled);
		SendNameInt("mode",c->mode);
		SendNameInt("pulsePerRev",c->pulsePerRevolution);
		SendNameInt("divider",c->timerDivider);
		SendNameInt("scaling",c->calculatedScaling);
	}
}

void SetTimerConfig(unsigned int argc, char **argv){
	TimerConfig * c = AssertTimerGetParam(argc,argv);
	if (NULL != c){
		SetChannelConfig(&(c->cfg),2,argc,argv);
		if (argc > 5) c->loggingPrecision = modp_atoi(argv[5]);
		if (argc > 6) c->slowTimerEnabled = (modp_atoi(argv[6]) != 0);
		if (argc > 7) c->mode = filterTimerMode(modp_atoi(argv[7]));
		if (argc > 8) c->pulsePerRevolution = modp_atoi(argv[8]);
		if (argc > 9) c->timerDivider = filterTimerDivider(modp_atoi(argv[9]));
		if (argc > 10){
			c->calculatedScaling = modp_atoi(argv[10]);
		}
		else{
			calculateTimerScaling(c);
		}
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
