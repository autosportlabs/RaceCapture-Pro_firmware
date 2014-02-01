#include "loggerSampleData.h"
#include "loggerHardware.h"
#include "loggerConfig.h"
#include "loggerData.h"
#include "accelerometer.h"
#include "gps.h"


static float get_mapped_value(float value, ScalingMap *scalingMap){
	unsigned short *bins;
	unsigned int bin, nextBin;

	bins = scalingMap->rawValues + ANALOG_SCALING_BINS - 1;
	bin = nextBin = ANALOG_SCALING_BINS - 1;

	while (value < *bins && bin > 0){
		bins--;
		bin--;
	}
	if (bin == 0 && value < *bins){
		return scalingMap->scaledValues[0];
	}
	else{
		nextBin = bin;
		if (bin < ANALOG_SCALING_BINS - 1){
			nextBin++;
		}
		else{
			return scalingMap->scaledValues[ANALOG_SCALING_BINS - 1];
		}
	}
	float x1 = (float)scalingMap->rawValues[bin];
	float y1 = scalingMap->scaledValues[bin];
	float x2 = (float)scalingMap->rawValues[nextBin];
	float y2 = scalingMap->scaledValues[nextBin];
	float scaled = LinearInterpolate(value,x1,y1,x2,y2);
	return scaled;
}

float get_analog_sample(int channelId){
	LoggerConfig * config = getWorkingLoggerConfig();
	ADCConfig *ac = &(config->ADCConfigs[channelId]);
	unsigned int value = readADC(channelId);
	float analogValue = 0;
	switch(ac->scalingMode){
		case SCALING_MODE_RAW:
			analogValue = value;
			break;
		case SCALING_MODE_LINEAR:
			analogValue = (ac->linearScaling * (float)value);
			break;
		case SCALING_MODE_MAP:
			analogValue = get_mapped_value((float)value,&(ac->scalingMap));
			break;
		default:
			analogValue = -1;
			break;
	}
	return analogValue;
}

float get_timer_sample(int channelId){
	LoggerConfig *config = getWorkingLoggerConfig();
	TimerConfig *c = &(config->TimerConfigs[i]);
	size_t sr = c->cfg.sampleRate;
	unsigned int value = getTimerPeriod(channelId);
	float timerValue = 0;
	switch (c->mode){
		case MODE_LOGGING_TIMER_RPM:
			timerValue = TIMER_PERIOD_TO_RPM(value, scaling);
			break;
		case MODE_LOGGING_TIMER_FREQUENCY:
			timerValue = TIMER_PERIOD_TO_HZ(value, scaling);
			break;
		case MODE_LOGGING_TIMER_PERIOD_MS:
			timerValue = TIMER_PERIOD_TO_MS(value, scaling);
			break;
		case MODE_LOGGING_TIMER_PERIOD_USEC:
			timerValue = TIMER_PERIOD_TO_USEC(value, scaling);
			break;
		default:
			timerValue = -1;
			break;
	}
	return timerValue;
}

float get_pwm_sample(int channelId){
	LoggerConfig *config = getWorkingLoggerConfig();
	PWMConfig *pc = &(loggerConfig->PWMConfigs[i]);
	float pwmValue = 0;
	switch (c->loggingMode){
		case MODE_LOGGING_PWM_PERIOD:
			pwmValue = PWM_GetPeriod(channelId);
			break;
		case MODE_LOGGING_PWM_DUTY:
			pwmValue = PWM_GetDutyCycle(channelId);
			break;
		case MODE_LOGGING_PWM_VOLTS:
			pwmValue = PWM_GetDutyCycle(channelId) * c->voltageScaling;
			break;
		default:
			pwmValue = -1;
			break;
	}
	return pwmValue;
}

float get_gpio_sample(int channelId){
	int gpioValue = readGpio(channelId);
	return (float)gpioValue;
}

float get_accel_sample(int channelId){
	LoggerConfig *config = getWorkingLoggerConfig();
	AccelConfig *ac = &(config->AccelConfigs[channelId]);
	float value = accelerometer_read_value(channelId, ac);
	return value;
}

float get_gps_sample(int channelId){
	float value = 0;
	switch(channelId){
		gps_channel_latitude:
			value = getLatitude();
			break;
		gps_channel_longitude:
			value = getLongitude();
			break;
		gps_channel_speed:
			value = getGPSSpeed() *  0.621371192; //convert to MPH
			break;
		gps_channel_time:
			value - getUTCTime();
			break;
		gps_channel_satellites:
			value = getSatellitesUsedForPosition();
			break;
		default:
			value = -1;
			break;
	}
	return value;
}

float get_lap_stat_sample(int channelId){
	float value = 0;
	switch(channelId){
		lap_stat_channel_lapcount:
			value = getLapCount();
			break;
		lap_stat_channel_laptime:
			value = getLastLapTime();
			break;
		lap_stat_channel_splittime:
			value = getLastSplitTime();
			break;
		lap_stat_channel_distance:
			value = getDistance();
			break;
		lap_stat_channel_predtime:
			value = get_predicted_time();
			break;
		default:
			value = -1;
			break;
	}
	return value;
}

