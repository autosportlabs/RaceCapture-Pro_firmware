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
#include "loggerHardware.h"
#include "sdcard.h"
#include "loggerData.h"
#include "sampleRecord.h"
#include "usart.h"

void SampleData(unsigned int argc, char **argv){
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
			SendNameFloat(channelConfig->label,sample->floatValue,precision);
		}
		else{
			SendNameInt(channelConfig->label,sample->intValue);
		}
	}
}

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

static ADCConfig * AssertAdcGetChannel(unsigned int argc, char **argv){
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

void GetAnalogConfig(unsigned int argc, char **argv){
	ADCConfig * c = AssertAdcGetChannel(argc,argv);
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
	ADCConfig * c = AssertAdcGetChannel(argc,argv);
	if (NULL != c){
		if (argc > 8 && argc < 18){
			SendCommandError(ERROR_CODE_MISSING_PARAMS);
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
		SendCommandOK();
	}
}

static PWMConfig * AssertPwmGetChannel(unsigned int argc, char **argv){
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

void GetPwmConfig(unsigned int argc, char **argv){
	PWMConfig *c = AssertPwmGetChannel(argc,argv);
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
	PWMConfig *c = AssertPwmGetChannel(argc,argv);
	if (NULL != c){
		if (argc > 2) SetChannelConfig(&(c->cfg),2,argc,argv);
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

static GPIOConfig * AssertGpioGetChannel(unsigned int argc, char **argv){
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
	GPIOConfig *c = AssertGpioGetChannel(argc,argv);
	if (NULL != c){
		SendChannelConfig(&(c->cfg));
		SendNameInt("mode",c->mode);
	}
}

void SetGpioConfig(unsigned int argc, char **argv){
	GPIOConfig *c = AssertGpioGetChannel(argc,argv);
	if (NULL != c){
		SetChannelConfig(&(c->cfg),2,argc,argv);
		if (argc > 5) c->mode = filterGpioMode(modp_atoi(argv[5]));
		InitGPIO(getWorkingLoggerConfig());
		SendCommandOK();
	}
}

static TimerConfig* AssertTimerGetChannel(unsigned int argc, char **argv){
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
	TimerConfig * c = AssertTimerGetChannel(argc,argv);
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
	TimerConfig * c = AssertTimerGetChannel(argc,argv);
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

static AccelConfig * AssertAccelGetChannel(unsigned int argc, char **argv){
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
	AccelConfig * c= AssertAccelGetChannel(argc,argv);
	if (NULL != c){
		SendChannelConfig(&(c->cfg));
		SendNameInt("mode",c->mode);
		SendNameInt("channel",c->accelChannel);
		SendNameInt("zeroValue",c->zeroValue);
	}
}

void SetAccelConfig(unsigned int argc, char **argv){
	AccelConfig * c= AssertAccelGetChannel(argc,argv);
	if (NULL != c){
		SetChannelConfig(&(c->cfg),2,argc,argv);
		if (argc >= 5) c->mode = filterAccelMode(modp_atoi(argv[5]));
		if (argc >= 6) c->accelChannel = filterAccelChannel(modp_atoi(argv[6]));
		if (argc >= 7) c->zeroValue = modp_atoi(argv[7]);
		SendCommandOK();
	}
}

void CalibrateAccelZero(unsigned int argc, char **argv){
	if (getWorkingLoggerConfig()->AccelInstalled){
		calibrateAccelZero();
		SendCommandOK();
	}
	else{
		SendCommandError(ERROR_CODE_INVALID_COMMAND);
	}

}

void GetLoggerOutputConfig(unsigned int argc, char **argv){
	LoggerOutputConfig *c = &(getWorkingLoggerConfig()->LoggerOutputConfig);
	SendNameInt("sdLoggingMode",c->sdLoggingMode);
	SendNameInt("telemetryMode",c->telemetryMode);
	SendNameUint("p2pDestAddrHigh",c->p2pDestinationAddrHigh);
	SendNameUint("p2pDestAddrLow",c->p2pDestinationAddrLow);
	SendNameString("telemetryServerHost", c->telemetryServerHost);
	SendNameString("telemetryDeviceId", c->telemetryDeviceId);
}

void SetLoggerOutputConfig(unsigned int argc, char **argv){
	LoggerOutputConfig *c = &(getWorkingLoggerConfig()->LoggerOutputConfig);
	if (argc > 1) c->sdLoggingMode = filterSdLoggingMode((char)modp_atoui(argv[1]));
	if (argc > 2) c->telemetryMode = filterTelemetryMode((char)modp_atoui(argv[2]));
	if (argc > 3) c->p2pDestinationAddrHigh = modp_atoui(argv[3]);
	if (argc > 4) c->p2pDestinationAddrLow = modp_atoui(argv[4]);
	if (argc > 5) setTextField(c->telemetryServerHost, argv[5], TELEMETRY_SERVER_HOST_LENGTH);
	if (argc > 6) setTextField(c->telemetryDeviceId, argv[6], DEVICE_ID_LENGTH);
	SendCommandOK();
}


static void StartTerminal0(unsigned int baud){
	initUsart0(USART_MODE_8N1, baud);
	while (1){
		char c = ReadChar();
		if (c == 27) break;
		if (c){
			SendChar(c);
			if (c == '\r') SendChar('\n');
			usart0_putchar(c);
			if (c == '\r') usart0_putchar('\n');
		}
		c = usart0_getcharWait(0);
		if (c){
			SendChar(c);
			if (c == '\r') SendChar('\n');
		}
	}
}

static void StartTerminal1(unsigned int baud){
	initUsart0(USART_MODE_8N1, baud);
	if (baud == 460800){
		SendString("Fixing Baud\r\n");
		AT91C_BASE_US0->US_BRGR = BOARD_MCK / (8 * 460800);
	}
	while (1){
		char c = usart1_getcharWait(0);
		if (c) SendChar(c);
		c = ReadChar();
		if (c == 27) break;
		if (c) usart1_putchar(c);
	}
}

void StartTerminal(unsigned int argc, char **argv){
	if (argc < 3){
		SendCommandError(ERROR_CODE_MISSING_PARAMS);
		return;
	}

	SendString("Entering Terminal. Press ESC to exit\r\n");

	unsigned int port = modp_atoui(argv[1]);
	unsigned int baud = modp_atoui(argv[2]);

	switch(port){
		case 0:
			StartTerminal0(baud);
			break;
		case 1:
			StartTerminal1(baud);
			break;
		default:
			SendCommandError(ERROR_CODE_INVALID_PARAM);
	}
}
