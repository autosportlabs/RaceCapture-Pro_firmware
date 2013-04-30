/*
 * loggerCommands.c
 *
 *  Created on: Jul 24, 2011
 *      Author: brent
 */
#include <stddef.h>
#include "mod_string.h"
#include "loggerCommands.h"
#include "modp_numtoa.h"
#include "modp_atonum.h"
#include "loggerConfig.h"
#include "loggerHardware.h"
#include "sdcard.h"
#include "loggerData.h"
#include "sampleRecord.h"
#include "usart.h"

void SampleData(Serial *serial, unsigned int argc, char **argv){
	SampleRecord sr;
	LoggerConfig * config = getWorkingLoggerConfig();
	initSampleRecord(config, &sr);
	populateSampleRecord(&sr,0,config);

	for (int i = 0; i < SAMPLE_RECORD_CHANNELS; i++){
		ChannelSample *sample = &(sr.Samples[i]);
		ChannelConfig * channelConfig = sample->channelConfig;

		if (SAMPLE_DISABLED == channelConfig->sampleRate) continue;
		if (sample->intValue == NIL_SAMPLE) continue;

		int precision = sample->precision;
		if (precision > 0){
			put_nameFloat(serial, channelConfig->label,sample->floatValue,precision);
		}
		else{
			put_nameInt(serial, channelConfig->label,sample->intValue);
		}
	}
}

void TestSD(Serial *serial, unsigned int argc, char **argv){

	int lines = 1;
	int doFlush = 0;
	int quiet = 0;
	if (argc > 1) lines = modp_atoi(argv[1]);
	if (argc > 2) doFlush = modp_atoi(argv[2]);
	if (argc > 3) quiet = modp_atoi(argv[3]);
	TestSDWrite(serial, lines,doFlush, quiet);
}

void FlashLoggerConfig(Serial *serial, unsigned int argc, char **argv){
	int result = flashLoggerConfig();
	if (0 == result){
		put_commandOK(serial);
	}
	else{
		put_commandError(serial, result);
	}
}

static void SetChannelConfig(ChannelConfig *c, unsigned int offset, unsigned int argc, char **argv){
	if (argc > offset) setLabelGeneric(c->label,argv[offset]);
	if (argc > offset + 1) setLabelGeneric(c->units,argv[offset + 1]);
	if (argc > offset + 2) c->sampleRate = encodeSampleRate(modp_atoi(argv[offset + 2]));
}

static void SendChannelConfigSuffix(Serial *serial, ChannelConfig *c, const char *suf){
	if (NULL != c){
		put_nameSuffixString(serial, "label",suf,c->label);
		put_nameSuffixString(serial, "units",suf,c->units);
		put_nameSuffixInt(serial, "sampleRate", suf, decodeSampleRate(c->sampleRate));
	}
}

static void SendChannelConfig(Serial *serial, ChannelConfig *c){
	if (NULL != c){
		put_nameString(serial, "label",c->label);
		put_nameString(serial, "units",c->units);
		put_nameInt(serial, "sampleRate",decodeSampleRate(c->sampleRate));
	}
}

static LoggerConfig * AssertSetParam(Serial *serial, unsigned int argc, unsigned int requiredParams){
	LoggerConfig *c = NULL;
	if (argc >= requiredParams){
		c = getWorkingLoggerConfig();
	}
	else{
		put_commandError(serial, ERROR_CODE_MISSING_PARAMS);
	}
	return c;
}

static ADCConfig * AssertAdcGetChannel(Serial *serial, unsigned int argc, char **argv){
	ADCConfig *c = NULL;
	if (argc >= 2){
		c = getADCConfigChannel(modp_atoi(argv[1]));
		if (NULL == c) put_commandError(serial, ERROR_CODE_INVALID_PARAM);
	}
	else{
		put_commandError(serial, ERROR_CODE_MISSING_PARAMS);
	}
	return c;
}

void GetAnalogConfig(Serial *serial, unsigned int argc, char **argv){
	ADCConfig * c = AssertAdcGetChannel(serial, argc,argv);
	if (NULL != c){
		SendChannelConfig(serial, &(c->cfg));
		put_nameInt(serial, "loggingPrecision",c->loggingPrecision);
		put_nameInt(serial, "scalingMode",c->scalingMode);
		put_nameFloat(serial, "scaling",c->linearScaling,ANALOG_SCALING_PRECISION);
		for (int i = 0; i < ANALOG_SCALING_BINS; i++){
			put_nameIndexInt(serial, "mapRaw",i,c->scalingMap.rawValues[i]);
		}
		for (int i = 0; i < ANALOG_SCALING_BINS; i++){
			put_nameIndexFloat(serial, "mapScaled",i,c->scalingMap.scaledValues[i],ANALOG_SCALING_PRECISION);
		}
	}
}

void SetAnalogConfig(Serial *serial, unsigned int argc, char **argv){
	ADCConfig * c = AssertAdcGetChannel(serial, argc,argv);
	if (NULL != c){
		if (argc > 8 && argc < 18){
			put_commandError(serial, ERROR_CODE_MISSING_PARAMS);
			return;
		}
		if (argc > 2) SetChannelConfig(&(c->cfg),2,argc,argv);
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
		put_commandOK(serial);
	}
}

static PWMConfig * AssertPwmGetChannel(Serial *serial, unsigned int argc, char **argv){
	PWMConfig *c = NULL;
	if (argc >= 2){
		c = getPwmConfigChannel(modp_atoi(argv[1]));
		if (NULL == c) put_commandError(serial, ERROR_CODE_INVALID_PARAM);
	}
	else{
		put_commandError(serial, ERROR_CODE_MISSING_PARAMS);
	}
	return c;
}

void SetPwmClockFreq(Serial *serial, unsigned int argc, char **argv){
	if (argc >= 2){
		getWorkingLoggerConfig()->PWMClockFrequency = filterPwmClockFrequency(modp_atoi(argv[1]));
		put_commandOK(serial);
	}
	else{
		put_commandError(serial, ERROR_CODE_MISSING_PARAMS);
	}
}

void GetPwmClockFreq(Serial *serial, unsigned int argc, char **argv){
	put_nameInt(serial, "frequency",getWorkingLoggerConfig()->PWMClockFrequency);
}

void GetPwmConfig(Serial *serial, unsigned int argc, char **argv){
	PWMConfig *c = AssertPwmGetChannel(serial, argc,argv);
	if (NULL != c){
		SendChannelConfig(serial, &(c->cfg));
		put_nameInt(serial, "loggingPrecision",c->loggingPrecision);
		put_nameInt(serial, "outputMode",c->outputMode);
		put_nameInt(serial, "loggingMode",c->loggingMode);
		put_nameInt(serial, "startupDutyCycle",c->startupDutyCycle);
		put_nameInt(serial, "startupPeriod",c->startupPeriod);
		put_nameFloat(serial, "voltageScaling",c->voltageScaling,VOLTAGE_SCALING_PRECISION);
	}
}

void SetPwmConfig(Serial *serial, unsigned int argc, char **argv){
	PWMConfig *c = AssertPwmGetChannel(serial, argc,argv);
	if (NULL != c){
		if (argc > 2) SetChannelConfig(&(c->cfg),2,argc,argv);
		if (argc > 5) c->loggingPrecision = modp_atoi(argv[5]);
		if (argc > 6) c->outputMode = filterPwmOutputMode(modp_atoi(argv[6]));
		if (argc > 7) c->loggingMode = filterPwmLoggingMode(modp_atoi(argv[7]));
		if (argc > 8) c->startupDutyCycle = filterPwmDutyCycle(modp_atoi(argv[8]));
		if (argc > 9) c->startupPeriod = filterPwmPeriod(modp_atoi(argv[9]));
		if (argc > 10) c->voltageScaling = modp_atof(argv[10]);
		put_commandOK(serial);
	}
}

void GetGpsConfig(Serial *serial, unsigned int argc, char **argv){
	GPSConfig *cfg = &(getWorkingLoggerConfig()->GPSConfig);
	put_nameInt(serial, "installed",cfg->GPSInstalled);
	SendChannelConfigSuffix(serial, &(cfg->latitudeCfg),"lat");
	SendChannelConfigSuffix(serial, &(cfg->longitudeCfg),"long");
	SendChannelConfigSuffix(serial, &(cfg->speedCfg),"vel");
	SendChannelConfigSuffix(serial, &(cfg->timeCfg),"time");
	SendChannelConfigSuffix(serial, &(cfg->satellitesCfg),"sats");
}

void SetGpsConfig(Serial *serial, unsigned int argc, char **argv){
	GPSConfig *cfg = &(getWorkingLoggerConfig()->GPSConfig);
	if (argc > 1) cfg->GPSInstalled = (modp_atoi(argv[1]) != 0);
	if (argc > 2) SetChannelConfig(&(cfg->latitudeCfg),2,argc,argv);
	if (argc > 5) SetChannelConfig(&(cfg->longitudeCfg),5,argc,argv);
	if (argc > 8) SetChannelConfig(&(cfg->speedCfg),8,argc,argv);
	if (argc > 11) SetChannelConfig(&(cfg->timeCfg),11,argc,argv);
	if (argc > 14) SetChannelConfig(&(cfg->satellitesCfg),14,argc,argv);
	put_commandOK(serial);
}

void GetStartFinishConfig(Serial *serial, unsigned int argc, char **argv){
	GPSConfig *cfg = &(getWorkingLoggerConfig()->GPSConfig);
	SendChannelConfigSuffix(serial, &(cfg->lapCountCfg),"lapCount");
	SendChannelConfigSuffix(serial, &(cfg->lapTimeCfg),"lapTime");
	SendChannelConfigSuffix(serial, &(cfg->splitTimeCfg), "splitTime");
	put_nameFloat(serial, "startFinishLat",cfg->startFinishConfig.latitude,DEFAULT_GPS_POSITION_LOGGING_PRECISION);
	put_nameFloat(serial, "startFinishLong", cfg->startFinishConfig.longitude,DEFAULT_GPS_POSITION_LOGGING_PRECISION);
	put_nameFloat(serial, "startFinishRadius",cfg->startFinishConfig.targetRadius,DEFAULT_GPS_RADIUS_LOGGING_PRECISION);
	put_nameFloat(serial, "splitLat", cfg->splitConfig.latitude, DEFAULT_GPS_POSITION_LOGGING_PRECISION);
	put_nameFloat(serial, "splitLong", cfg->splitConfig.longitude, DEFAULT_GPS_POSITION_LOGGING_PRECISION);
	put_nameFloat(serial, "splitRadius", cfg->splitConfig.targetRadius, DEFAULT_GPS_RADIUS_LOGGING_PRECISION);
}

void SetStartFinishConfig(Serial *serial, unsigned int argc, char **argv){
	GPSConfig *cfg = &(getWorkingLoggerConfig()->GPSConfig);
	if (argc > 1) SetChannelConfig(&(cfg->lapCountCfg),1,argc,argv);
	if (argc > 4) SetChannelConfig(&(cfg->lapTimeCfg),4,argc,argv);
	if (argc > 7) SetChannelConfig(&(cfg->splitTimeCfg), 7, argc, argv);
	if (argc > 10) cfg->startFinishConfig.latitude = modp_atof(argv[10]);
	if (argc > 11) cfg->startFinishConfig.longitude = modp_atof(argv[11]);
	if (argc > 12) cfg->startFinishConfig.targetRadius = modp_atof(argv[12]);
	if (argc > 13) cfg->splitConfig.latitude = modp_atof(argv[13]);
	if (argc > 14) cfg->splitConfig.longitude = modp_atof(argv[14]);
	if (argc > 15) cfg->splitConfig.targetRadius = modp_atof(argv[15]);
	put_commandOK(serial);
}

static GPIOConfig * AssertGpioGetChannel(Serial *serial, unsigned int argc, char **argv){
	GPIOConfig *c = NULL;
	if (argc >= 2){
		c = getGPIOConfigChannel(modp_atoi(argv[1]));
		if (NULL == c) put_commandError(serial, ERROR_CODE_INVALID_PARAM);
	}
	else{
		put_commandError(serial, ERROR_CODE_MISSING_PARAMS);
	}
	return c;
}

void GetGpioConfig(Serial *serial, unsigned int argc, char **argv){
	GPIOConfig *c = AssertGpioGetChannel(serial, argc,argv);
	if (NULL != c){
		SendChannelConfig(serial, &(c->cfg));
		put_nameInt(serial, "mode",c->mode);
	}
}

void SetGpioConfig(Serial *serial, unsigned int argc, char **argv){
	GPIOConfig *c = AssertGpioGetChannel(serial, argc,argv);
	if (NULL != c){
		SetChannelConfig(&(c->cfg),2,argc,argv);
		if (argc > 5) c->mode = filterGpioMode(modp_atoi(argv[5]));
		InitGPIO(getWorkingLoggerConfig());
		put_commandOK(serial);
	}
}

static TimerConfig* AssertTimerGetChannel(Serial *serial, unsigned int argc, char **argv){
	TimerConfig *c = NULL;
	if (argc >= 2){
		c = getTimerConfigChannel(modp_atoi(argv[1]));
		if (NULL == c) put_commandError(serial, ERROR_CODE_INVALID_PARAM);
	}
	else{
		put_commandError(serial, ERROR_CODE_MISSING_PARAMS);
	}
	return c;
}

void GetTimerConfig(Serial *serial, unsigned int argc, char **argv){
	TimerConfig * c = AssertTimerGetChannel(serial, argc,argv);
	if (NULL != c){
		SendChannelConfig(serial, &(c->cfg));
		put_nameInt(serial, "loggingPrecision",c->loggingPrecision);
		put_nameInt(serial, "slowTimer",c->slowTimerEnabled);
		put_nameInt(serial, "mode",c->mode);
		put_nameInt(serial, "pulsePerRev",c->pulsePerRevolution);
		put_nameInt(serial, "divider",c->timerDivider);
		put_nameInt(serial, "scaling",c->calculatedScaling);
	}
}

void SetTimerConfig(Serial *serial, unsigned int argc, char **argv){
	TimerConfig * c = AssertTimerGetChannel(serial, argc,argv);
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
			calculateTimerScaling(BOARD_MCK, c);
		}
		put_commandOK(serial);
	}
}

static AccelConfig * AssertAccelGetChannel(Serial *serial, unsigned int argc, char **argv){
	AccelConfig *c = NULL;
	if (argc >= 2){
		c = getAccelConfigChannel(modp_atoi(argv[1]));
		if (NULL == c) put_commandError(serial, ERROR_CODE_INVALID_PARAM);
	}
	else{
		put_commandError(serial, ERROR_CODE_MISSING_PARAMS);
	}
	return c;
}

void SetAccelInstalled(Serial *serial, unsigned int argc, char **argv){
	LoggerConfig * c = AssertSetParam(serial, argc,2);
	if (NULL != c){
		c->AccelInstalled = (modp_atoi(argv[1]) != 0);
		put_commandOK(serial);
	}
}

void GetAccelInstalled(Serial *serial, unsigned int argc, char **argv){
	LoggerConfig * c = getWorkingLoggerConfig();
	put_nameInt(serial, "installed",c->AccelInstalled);
}

void GetAccelConfig(Serial *serial, unsigned int argc, char **argv){
	AccelConfig * c= AssertAccelGetChannel(serial, argc,argv);
	if (NULL != c){
		SendChannelConfig(serial, &(c->cfg));
		put_nameInt(serial, "mode",c->mode);
		put_nameInt(serial, "channel",c->accelChannel);
		put_nameInt(serial, "zeroValue",c->zeroValue);
	}
}

void SetAccelConfig(Serial *serial, unsigned int argc, char **argv){
	AccelConfig * c= AssertAccelGetChannel(serial, argc,argv);
	if (NULL != c){
		SetChannelConfig(&(c->cfg),2,argc,argv);
		if (argc >= 5) c->mode = filterAccelMode(modp_atoi(argv[5]));
		if (argc >= 6) c->accelChannel = filterAccelChannel(modp_atoi(argv[6]));
		if (argc >= 7) c->zeroValue = modp_atoi(argv[7]);
		put_commandOK(serial);
	}
}

void CalibrateAccelZero(Serial *serial, unsigned int argc, char **argv){
	if (getWorkingLoggerConfig()->AccelInstalled){
		calibrateAccelZero();
		put_commandOK(serial);
	}
	else{
		put_commandError(serial, ERROR_CODE_INVALID_COMMAND);
	}

}

void GetLoggerOutputConfig(Serial *serial, unsigned int argc, char **argv){
	LoggerOutputConfig *c = &(getWorkingLoggerConfig()->LoggerOutputConfig);
	put_nameInt(serial, "sdLoggingMode",c->sdLoggingMode);
	put_nameInt(serial, "telemetryMode",c->telemetryMode);
	put_nameUint(serial, "p2pDestAddrHigh",c->p2pDestinationAddrHigh);
	put_nameUint(serial, "p2pDestAddrLow",c->p2pDestinationAddrLow);
	put_nameString(serial, "telemetryServerHost", c->telemetryServerHost);
	put_nameString(serial, "telemetryDeviceId", c->telemetryDeviceId);
}

void SetLoggerOutputConfig(Serial *serial, unsigned int argc, char **argv){
	LoggerOutputConfig *c = &(getWorkingLoggerConfig()->LoggerOutputConfig);
	if (argc > 1) c->sdLoggingMode = filterSdLoggingMode((char)modp_atoui(argv[1]));
	if (argc > 2) c->telemetryMode = filterTelemetryMode((char)modp_atoui(argv[2]));
	if (argc > 3) c->p2pDestinationAddrHigh = modp_atoui(argv[3]);
	if (argc > 4) c->p2pDestinationAddrLow = modp_atoui(argv[4]);
	if (argc > 5) setTextField(c->telemetryServerHost, argv[5], TELEMETRY_SERVER_HOST_LENGTH);
	if (argc > 6) setTextField(c->telemetryDeviceId, argv[6], DEVICE_ID_LENGTH);
	put_commandOK(serial);
}

static void StartTerminalSession(Serial *fromSerial, Serial *toSerial){

	while (1){
		char c = fromSerial->get_c_wait(0);
		if (c == 27) break;
		if (c){
			fromSerial->put_c(c);
			if (c == '\r') fromSerial->put_c('\n');
			toSerial->put_c(c);
			if (c == '\r') toSerial->put_c('\n');
		}
		c = toSerial->get_c_wait(0);
		if (c){
			fromSerial->put_c(c);
			if (c == '\r') fromSerial->put_c('\n');
		}
	}
}

void StartTerminal(Serial *serial, unsigned int argc, char **argv){
	if (argc < 3){
		put_commandError(serial, ERROR_CODE_MISSING_PARAMS);
		return;
	}

	serial->put_s("Entering Terminal. Press ESC to exit\r\n");

	unsigned int port = modp_atoui(argv[1]);
	unsigned int baud = modp_atoui(argv[2]);

	switch(port){
		case 0:
			initUsart0(USART_MODE_8N1, baud);
			StartTerminalSession(serial, get_serial_usart0());
			break;
		case 1:
			initUsart1(USART_MODE_8N1, baud);
			StartTerminalSession(serial, get_serial_usart1());
			break;
		default:
			put_commandError(serial, ERROR_CODE_INVALID_PARAM);
	}
}
