/*
 * loggerCommands.c
 *
 *  Created on: Jul 24, 2011
 *      Author: brent
 */
#include <string.h>
#include "loggerCommands.h"
#include "usb_comm.h"
#include "modp_numtoa.h"
#include "modp_atonum.h"
#include "loggerConfig.h"

static struct cmd_t flashLoggerConfigCmd ={NULL, "flashLoggerCfg", "Write the active Logger Configuration to flash.","", FlashLoggerConfig};

static struct cmd_t setAnalogLabelCmd ={NULL, "setAnalogLabel", "Sets the label for the specified analog channel.","<channel> <label> <units>", SetAnalogLabel};
static struct cmd_t getAnalogLabelCmd ={NULL, "getAnalogLabel", "Gets the label for the specified analog channel.","<channel>", GetAnalogLabel};

static struct cmd_t getAnalogSampleRateCmd ={NULL, "getAnalogSampleRate", "Gets the sample rate for the specified analog channel.","<channel>", GetAnalogSampleRate};
static struct cmd_t setAnalogSampleRateCmd ={NULL, "setAnalogSampleRate", "Sets the sample rate for the specified analog channel.","<channel> <sampleRate>", SetAnalogSampleRate};

static struct cmd_t setAnalogScalingCmd ={NULL, "setAnalogScaling", "Sets the scaling factor for the specified analog channel.","<channel> <scaling>", SetAnalogScaling};
static struct cmd_t getAnalogScalingCmd ={NULL, "getAnalogScaling", "Gets the scaling factor for the specified analog channel.","<channel>", GetAnalogScaling};

static struct cmd_t setPwmClockFreqCmd ={NULL, "setPwmClockFreq", "Sets the PWM clock frequency for the specified channel.","<channel> <frequency>", SetPwmClockFreq};
static struct cmd_t getPwmClockFreqCmd ={NULL, "getPwmClockFreq", "Gets the PWM clock frequency for the specified channel.","<channel>", GetPwmClockFreq};

static struct cmd_t setPwmLabelCmd ={NULL, "setPwmLabel", "Sets the label for the specified PWM channel.","<channel> <label>", SetPwmLabel};
static struct cmd_t getPwmLabelCmd ={NULL, "getPwmLabel", "Gets the label for the specified PWM channel.","<channel>", GetPwmLabel};

static struct cmd_t setPwmSampleRateCmd ={NULL, "setPwmSampelRate", "Sets the sample rate for the specified PWM channel.","<channel> <sampleRate>", SetPwmSampleRate};
static struct cmd_t getPwmSampleRateCmd ={NULL, "getPwmSampleRate", "Gets the sample rate for the specified PWM channel.","<channel>", GetPwmSampleRate};

static struct cmd_t setPwmOutputCfgCmd ={NULL, "setPwmOutputCfg", "Sets the output configuration for the specified PWM channel.","<channel> <config>", SetPwmOutputCfg};
static struct cmd_t getPwmOutputCfgCmd ={NULL, "getPwmOutputCfg", "Gets the output configuration for the specified PWM channel.","<channel>", GetPwmOutputCfg};

static struct cmd_t setPwmLoggingCfgCmd ={NULL, "setPwmLoggingCfg", "Sets the logging configuration for the specified PWM channel.","<channel> <config>", SetPwmLoggingCfg};
static struct cmd_t getPwmLoggingCfgCmd ={NULL, "getPwmLoggingCfg", "Sets the logging configuration for the specified PWM channel.","<channel>", GetPwmLoggingCfg};

static struct cmd_t setPwmStartupDutyCycleCmd ={NULL, "setPwmStartupDutyCycle", "Sets the startup duty cycle for the specified PWM channel.","<channel> <dutyCycle>", SetPwmStartupDutyCycle};
static struct cmd_t getPwmStartupDutyCycleCmd ={NULL, "getPwmStartupDutyCycle", "Gets the startup duty cycle for the specified PWM channel.","<channel>", GetPwmStartupDutyCycle};

static struct cmd_t setPwmStartupPeriodCmd ={NULL, "setPwmStartupPeriod", "Sets the startup period for the specified PWM channel.","<channel> <period>", SetPWMStartupPeriod};
static struct cmd_t getPwmStartupPeriodCmd ={NULL, "getPwmStartupPeriod", "Gets the startup period for the specified PWM channel.","<channel>", GetPwmStartupPeriod};

static struct cmd_t setPwmVoltageScalingCmd ={NULL, "setPwmVoltageScaling", "Sets the voltage scaling for the specified PWM channel.","<channel> <scaling>", SetPwmVoltageScaling};
static struct cmd_t getPwmVoltageScalingCmd ={NULL, "getPwmVoltageScaling", "Gets the voltage scaling for the specified PWM channel.","<channel>", getPwmVoltageScaling};

static struct cmd_t setGpsInstalledCmd ={NULL, "setGpsInstalled", "Sets the installed state of the GPS module.","<installed>", SetGpsInstalled};
static struct cmd_t getGpsInstalledCmd ={NULL, "getGpsInstalled", "Gets the installed state of the GPS module.","", getGpsInstalled};

static struct cmd_t setGpsQualityLabelCmd ={NULL, "setGpsQualityLabel", "Sets the GPS quality label.","<label>", SetGpsQualityLabel};
static struct cmd_t getGpsQualityLabelCmd ={NULL, "getGpsQualityLabel", "Gets the GPS quality label.","", GetGpsQualityLabel};

static struct cmd_t setGpsStatsLabelCmd ={NULL, "setGpsStatsLabel", "Sets the GPS stats label.","<label>", SetGpsStatsLabel};
static struct cmd_t getGpsStatsLabelCmd ={NULL, "getGpsStatsLabel", "Gets the GPS stats label.","", GetGpsStatsLabel};

static struct cmd_t setGpsLatitudeLabelCmd ={NULL, "setGpsLatitudeLabel", "Sets the GPS latitude label.","<label>", SetGpsLatitudeLabel};
static struct cmd_t getGpsLatitudeLabelCmd ={NULL, "getGpsLatitudeLabel", "Gets the GPS latitude label.","", GetGpsLatitudeLabel};

static struct cmd_t setGpsLongitudeLabelCmd ={NULL, "setGpsLongitudeLabel", "Sets the GPS longitude label.","<label>", SetGpsLongitudeLabel};
static struct cmd_t getGpsLongitudeLabelCmd ={NULL, "getGpsLongitudeLabel", "Gets the GPS longitude label.","", GetGpsLongitudeLabel};

static struct cmd_t setGpsTimeLabelCmd ={NULL, "setGpsTimeLabel", "Sets the GPS time label.","<label>", SetGpsTimeLabel};
static struct cmd_t getGpsTimeLabelCmd ={NULL, "getGpsTimeLabel", "Gets the GPS time label.","", GetGpsTimeLabel};

static struct cmd_t setGpsVelocityLabelCmd ={NULL, "setGpsVelocityLabel", "Sets the GPS velocity label.","<label>", SetGpsVelocityLabel};
static struct cmd_t getGpsVelocityLabelCmd ={NULL, "getGpsVeloictyLabel", "Gets the GPS velocity label.","", GetGpsVelocityLabel};

static struct cmd_t setGpsPositionSampleRateCmd ={NULL, "setGpsPositionSampleRate", "Sets the GPS position sample rate.","<sampleRate>", SetGpsPositionSampleRate};
static struct cmd_t getGpsPositionSampleRateCmd ={NULL, "getGpsPositionSampleRate", "Gets the GPS position sample rate.","", GetGpsPositionSampleRate};

static struct cmd_t setGpsVelocitySampleRateCmd ={NULL, "setGpsVelocitySampleRate", "Sets the GPS velocity sample rate.","<sampleRate>", SetGpsVelocitySampleRate};
static struct cmd_t getGpsVelocitySampleRateCmd ={NULL, "getGpsVelocitySampleRate", "Gets the GPS velocity sample rate.","", GetGpsVelocitySampleRate};

static struct cmd_t setGpsTimeSampleRateCmd ={NULL, "setGpsTimeSampleRate", "Sets the GPS time sample rate.","<sampleRate>", SetGpsTimeSampleRate};
static struct cmd_t getGpsTimeSampleRateCmd ={NULL, "getGpsTimeSampleRate", "Gets the GPS time sample rate.","", GetGpsTimeSampleRate};

static struct cmd_t setGpioLabelCmd ={NULL, "setGpioLabel", "Sets the label for the specified GPIO port.","<port> <label>", SetGpioLabel};
static struct cmd_t getGpioLabelCmd ={NULL, "getGpioLabel", "Gets the label for the specified GPIO port.","<port>", GetGpioLabel};

static struct cmd_t setGpioSampleRateCmd ={NULL, "setGpioSampleRate", "Sets the sample rate for the specified GPIO port.","<port> <sampleRate>", SetGpioSampleRate};
static struct cmd_t getGpioSampleRateCmd ={NULL, "getGpioSampleRate", "Gets the sample rate for the specified GPIO port.","<port>", GetGpioSampleRate};

static struct cmd_t setGpioCfgCmd ={NULL, "setGpioCfg", "Sets the configuration for the specified GPIO port.","<port> <configuration>", SetGpioConfig};
static struct cmd_t getGpioCfgCmd ={NULL, "getGpioCfg", "Gets the configuration for the specified GPIO port.","<port>", GetGpioConfig};

static struct cmd_t setTimerLabelCmd ={NULL, "setTimerLabel", "Sets the label for the specified timer channel.","<channel> <label>", SetTimerLabel};
static struct cmd_t getTimerLabelCmd ={NULL, "getTimerLabel", "Gets the label for the specified timer channel.","<channel>", GetTimerLabel};

static struct cmd_t setTimerSampleRateCmd ={NULL, "setTimerSampleRate", "Sets the sample rate for the specified timer channel.","<channel> <sampleRate>", SetTimerSampleRate};
static struct cmd_t getTimerSampleRateCmd ={NULL, "getTimerSampleRate", "Gets the sample rate for the specified timer channel.","<channel>", GetTimerSampleRate};

static struct cmd_t setTimerCfgCmd ={NULL, "setTimerCfg", "Sets the configuration for the specified timer channel.","<channel> <configuration>", SetTimerCfg};
static struct cmd_t getTimerCfgCmd ={NULL, "getTimerCfg", "Gets the configuration for the specified timer channel.","<channel>", GetTimerCfg};

static struct cmd_t setTimerPulsePerRevCmd ={NULL, "setTimerPulsePerRev", "Sets the pulse per revolution for the specified timer channel.","<channel> <pulses>", SetTimerPulsePerRev};
static struct cmd_t getTimerPulsePerRevCmd ={NULL, "getTimerPulsePerRev", "Gets the pulse per revolution for the specified timer channel.","<channel>", GetTimerPulsePerRev};

static struct cmd_t setTimerDividerCmd ={NULL, "setTimerDivider", "Sets the divider for the specified timer channel.","<channel> <divider>", SetTimerDivider};
static struct cmd_t getTimerDividerCmd ={NULL, "getTimerDivider", "Gets the divider for the specified timer channel.","<channel>", GetTimerDivider};

static struct cmd_t calculateTimerScalingCmd ={NULL, "calculateTimerScaling", "Calculates and updates the scaling for the specified timer channel.","<channel>", CalculateTimerScaling};
static struct cmd_t getTimerScalingCmd ={NULL, "getTimerScaling", "Gets the scaling for the specified timer channel.","<channel>", GetTimerScaling};

static struct cmd_t setAccelInstalledCmd ={NULL, "setAccelInstalled", "Sets the installed state of the accelerometer.","", SetAccelInstalled};
static struct cmd_t getAccelInstalledCmd ={NULL, "getAccelInstalled", "Gets the installed state of the accelerometer.","", GetAceelInstalled};

static struct cmd_t setAccelLabelCmd ={NULL, "setAccelLabel", "Sets the label for the specified accelerometer channel.","<channel> <label>", SetAccelLabel};
static struct cmd_t getAccelLabelCmd ={NULL, "getAccelLabel", "Gets the label for the specified accelerometer channel.","<channel>", GetAccelLabel};

static struct cmd_t setAccelSampleRateCmd ={NULL, "setAccelSampleRate", "Sets the sample rate for the specified accelerometer channel.","<channel> <sampleRate>", SetAccelSampleRate};
static struct cmd_t getAccelSampleRateCmd ={NULL, "getAccelSampleRate", "Gets the sample rate for the specified accelerometer channel.","<channel>", GetAccelSampleRate};

static struct cmd_t setAccelIdleSampleRateCmd ={NULL, "setAccelIdleSampleRate", "Sets the sample rate for the specified accelerometer channel when not logging.","<channel> <sampleRate>", SetAccelIdleSampeRate};
static struct cmd_t getAccelIdleSampleRateCmd ={NULL, "getAccelIdleSampleRate", "Gets the sample rate for the specified accelerometer channel when not logging.","<channel>", GetAccelIdleSampleRate};

static struct cmd_t setAccelConfigCmd ={NULL, "setAccelConfig", "Sets the configuration for the specified accelerometer channel.","<channel> <config>", SetAccelConfig};
static struct cmd_t getAccelConfigCmd ={NULL, "getAccelConfig", "Gets the configuration for the specified accelerometer channel.","<channel>", GetAccelConfig};

static struct cmd_t setAccelChannelCmd ={NULL, "setAccelChannel", "Sets the X/Y/Z orientation for the specified accelerometer channel.","<channel> <orientation>", SetAccelChannel};
static struct cmd_t getAccelChannelCmd ={NULL, "getAccelChannel", "Gets the X/Y/Z orientation for the specified accelerometer channel.","<channel>", GetAccelChannel};

static struct cmd_t setAccelZeroValueCmd ={NULL, "setAccelZeroValue", "Sets the zero value for the specified accelerometer channel.","<channel> <zeroValue>", SetAccelZeroValue};
static struct cmd_t getAccelZeroValueCmd ={NULL, "getAccelZeroValue", "Gets the zero value for the specified accelerometer channel.","<channel>", GetAccelZeroValue};

static struct cmd_t calibrateAccelZeroCmd ={NULL, "calibrateAccelZero", "Calibrates the current accelerometer values as the zero point.","", CalibrateAccelZero};


void InitLoggerCommands(){

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
	if (3 < argc){
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
	if (2 < argc){
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
	if (NULL != c) SendNameString("label",c->label);
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
	if (3 < argc){
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
	if (2 < argc){
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
	if (2 < argc){
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
		c->GPSConfig.positionSampleRate = modp_atoi(argv[1]);
		SendCommandOK();
	}
}

void GetGpsPositionSampleRate(unsigned int argc, char **argv){
	SendNameInt("gpsPositionSampleRate",getWorkingLoggerConfig()->GPSConfig.positionSampleRate);
}

void SetGpsVelocitySampleRate(unsigned int argc, char **argv){
	struct LoggerConfig * c = AssertSetBaseParam(argc);
	if (NULL != c){
		c->GPSConfig.velocitySampleRate = modp_atoi(argv[1]);
		SendCommandOK();
	}
}

void GetGpsVelocitySampleRate(unsigned int argc, char **argv){
	SendNameInt("gpsVelocitySampleRate",getWorkingLoggerConfig()->GPSConfig.velocitySampleRate);
}

void SetGpsTimeSampleRate(unsigned int argc, char **argv){
	struct LoggerConfig * c = AssertSetBaseParam(argc);
	if (NULL != c){
		c->GPSConfig.timeSampleRate = modp_atoi(argv[1]);
		SendCommandOK();
	}
}

void GetGpsTimeSampleRate(unsigned int argc, char **argv){
	SendNameInt("gpsTimeSampleRate",getWorkingLoggerConfig()->GPSConfig.timeSampleRate);
}

struct GPIOConfig * AssertGpioSetParam(unsigned int argc, char **argv){
	struct GPIOConfig *c = NULL;
	if (3 < argc){
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
	if (2 < argc){
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
	if (NULL != c) SendNameInt("gpioSampleRate",c->sampleRate);
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
	if (3 < argc){
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
	if (2 < argc){
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
	if (NULL != c) SendNameInt("timerSampleRate", c->sampleRate);
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
		c->timerDivider = modp_atoi(argv[2]);
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
	struct TimerConfig * c = AssertTimerSetParam(argc,argv);
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
	if (3 < argc){
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
	if (2 < argc){
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
	if (NULL != c) c->AccelInstalled = (modp_atoi(argv[1]) != 0);
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

void SetAccelIdleSampeRate(unsigned int argc, char **argv){
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
}

/*
//Logger configuration editing
lua_register(L,"setAccelZeroValue",Lua_SetAccelZeroValue);
*/
