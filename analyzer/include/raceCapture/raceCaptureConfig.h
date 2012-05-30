/*
 * raceCaptureConfig.h
 *
 *  Created on: Apr 23, 2012
 *      Author: brent
 */

#ifndef RACECAPTURECONFIG_H_
#define RACECAPTURECONFIG_H_
#include "wx/wxprec.h"


//Number of channels per feature
#define CONFIG_ANALOG_CHANNELS				8
#define CONFIG_ACCEL_CHANNELS				4
#define CONFIG_GPIO_CHANNELS				3
#define CONFIG_ANALOG_PULSE_CHANNELS		4
#define CONFIG_TIMER_CHANNELS				3
#define CONFIG_GPS_CHANNELS					8

#define CONFIG_ANALOG_SCALING_BINS			5
#define MIN_ANALOG_RAW						0
#define MAX_ANALOG_RAW						4095

enum feature_installed_t {
	feature_not_installed = 0,
	feature_installed = 1,
};

enum sample_rate_t {
	sample_disabled = 0,
	sample_1Hz = 1,
	sample_5Hz = 5,
	sample_10Hz = 10,
	sample_20Hz = 20,
	sample_30Hz = 30,
	sample_50Hz = 50,
	sample_100Hz = 100
};

enum scaling_mode_t {
	scaling_mode_raw = 0,
	scaling_mode_linear,
	scaling_mode_map
};

WX_DEFINE_ARRAY_INT(sample_rate_t, SampleRates);

enum telemetry_mode_t{
	telemetry_mode_disabled = 0,
	telemetry_mode_p2p,
	telemetry_mode_cell
};

enum logging_mode_t{
	logging_mode_disabled = 0,
	logging_mode_csv,
	logging_mode_binary
};

class LoggerOutputConfig{
public:
	telemetry_mode_t telemetryMode;
	logging_mode_t loggingMode;
	unsigned int p2pDestinationAddrHigh;
	unsigned int p2pDestinationAddrLow;
};

class ChannelConfig{
public:
	ChannelConfig();
	static SampleRates GetSampleRates();
	const static int MAX_LABEL_LENGTH = 10;
	const static int MAX_UNITS_LENGTH = 10;
	const static int MAX_PRECISION = 8;
	const static int MIN_PRECISION = 0;
	wxString label;
	wxString units;
	sample_rate_t sampleRate;

private:
	static SampleRates sampleRates;

};

class ScalingMap{
public:
	unsigned short rawValues[CONFIG_ANALOG_SCALING_BINS];
	float scaledValue[CONFIG_ANALOG_SCALING_BINS];
};

class AnalogConfig{
public:
	ChannelConfig channelConfig;
	int loggingPrecision;
	float linearScaling;
	scaling_mode_t scalingMode;
	ScalingMap scalingMap;
};

enum timer_mode_t {
	timer_mode_rpm = 0,
	timer_mode_frequency = 1,
	timer_mode_period_usec = 2,
	timer_mode_period_ms = 3
};

class TimerConfig{
public:
	const static int MIN_PULSE_PER_REV = 1;
	const static int MAX_PULSE_PER_REV = 360;
	ChannelConfig channelConfig;
	int loggingPrecision;
	bool slowTimerEnabled;
	timer_mode_t mode;
	int pulsePerRev;
	int timerDivider;
	int scaling;
};

enum gpio_mode_t {
	gpio_mode_in = 0,
	gpio_mode_out = 1
};

class GpioConfig{
public:
	ChannelConfig channelConfig;
	gpio_mode_t mode;
};

enum accel_mode_t{
	accel_mode_disabled = 0,
	accel_mode_normal = 1,
	accel_mode_inverted = 2
};

enum accel_channel_t{
	accel_channel_x = 0,
	accel_channel_y = 1,
	accel_channel_z = 2,
	accel_channel_zt = 3
};

class AccelConfig{
public:
	ChannelConfig channelConfig;
	accel_mode_t mode;
	accel_channel_t channel;
	unsigned long zeroValue;
};

enum pwm_output_mode_t{
	pwm_mode_analog = 0,
	pwm_mode_frequency = 1
};

enum pwm_logging_mode_t{
	mode_logging_pwm_period = 0,
	mode_logging_pwm_duty = 1,
	mode_logging_pwm_volts = 2
};

class PwmConfig{
public:
	const static int MIN_PWM_DUTY_CYCLE = 1;
	const static int MAX_PWM_DUTY_CYCLE = 100;

	const static int MIN_PWM_PERIOD = 1;
	const static int MAX_PWM_PERIOD = 2000;

	ChannelConfig channelConfig;
	int loggingPrecision;
	pwm_output_mode_t outputMode;
	pwm_logging_mode_t loggingMode;
	unsigned short startupDutyCycle;
	unsigned short startupPeriod;
	float voltageScaling;
};

class GpsConfig{
public:
	bool gpsInstalled;
	float startFinishLatitude;
	float startFinishLongitude;
	float startFinishRadius;
	ChannelConfig lapCountCfg;
	ChannelConfig lapTimeCfg;
	ChannelConfig qualityCfg;
	ChannelConfig satellitesCfg;
	ChannelConfig latitudeCfg;
	ChannelConfig longitudeCfg;
	ChannelConfig timeCfg;
	ChannelConfig velocityCfg;
};


class RaceCaptureConfig
{
public:
	AnalogConfig analogConfigs[CONFIG_ANALOG_CHANNELS];
	unsigned short PwmClockFrequency;
	PwmConfig pwmConfigs[CONFIG_ANALOG_PULSE_CHANNELS];
	GpioConfig gpioConfigs[CONFIG_GPIO_CHANNELS];
	TimerConfig timerConfigs[CONFIG_TIMER_CHANNELS];
	feature_installed_t accelInstalled;
	AccelConfig accelConfigs[CONFIG_ACCEL_CHANNELS];
	GpsConfig gpsConfig;
	LoggerOutputConfig loggerOutputConfig;
};

#endif /* RACECAPTURECONFIG_H_ */
