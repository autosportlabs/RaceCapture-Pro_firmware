#include "loggerSampleData.h"
#include "loggerHardware.h"
#include "loggerConfig.h"
#include "loggerData.h"
#include "accelerometer.h"
#include "sampleRecord.h"
#include "gps.h"
#include "predictive_timer.h"
#include "linear_interpolate.h"

void populate_sample_buffer(ChannelSample ** samples,  size_t count, size_t currentTicks){
	for (size_t i = 0; i < count; i++){
		ChannelSample *sample = samples[i];
		if (currentTicks % sample->channelConfig->sampleRate == 0){
			size_t channelIndex = sample->channelIndex;
			float value = sample->get_sample(channelIndex); //polymorphic behavior
			if (sample->precision == 0){
				sample->intValue = (int)value;
			}
			else{
				sample->floatValue = value;
			}
		}
		else{
			sample->intValue = NIL_SAMPLE;
		}
	}
}

void init_channel_sample_buffer(LoggerConfig *loggerConfig, ChannelSample * samples, size_t channelCount){
	ChannelSample *sample = samples;

	for (int i = 0; i < CONFIG_ACCEL_CHANNELS; i++){
		AccelConfig *config = &(loggerConfig->AccelConfigs[i]);
		if (config->cfg.sampleRate != SAMPLE_DISABLED){
			sample->precision = DEFAULT_ACCEL_LOGGING_PRECISION;
			sample->channelConfig = &(config->cfg);
			sample->intValue = NIL_SAMPLE;
			sample->channelIndex = i;
			sample->get_sample = get_accel_sample;
			sample++;
		}
	}

	for (int i=0; i < CONFIG_ADC_CHANNELS; i++){
		ADCConfig *config = &(loggerConfig->ADCConfigs[i]);
		if (config->cfg.sampleRate != SAMPLE_DISABLED){
			sample->precision = config->loggingPrecision;
			sample->channelConfig = &(config->cfg);
			sample->intValue = NIL_SAMPLE;
			sample->channelIndex = i;
			sample->get_sample = get_analog_sample;
			sample++;
		}
	}

	for (int i=0; i < CONFIG_TIMER_CHANNELS; i++){
		TimerConfig *config = &(loggerConfig->TimerConfigs[i]);
		if (config->cfg.sampleRate != SAMPLE_DISABLED){
			sample->precision = config->loggingPrecision;
			sample->channelConfig = &(config->cfg);
			sample->intValue = NIL_SAMPLE;
			sample->channelIndex = i;
			sample->get_sample = get_timer_sample;
			sample++;
		}
	}

	for (int i=0; i < CONFIG_GPIO_CHANNELS; i++){
		GPIOConfig *config = &(loggerConfig->GPIOConfigs[i]);
		if (config->cfg.sampleRate != SAMPLE_DISABLED){
			sample->precision = DEFAULT_GPIO_LOGGING_PRECISION;
			sample->channelConfig = &(config->cfg);
			sample->intValue = NIL_SAMPLE;
			sample->channelIndex = i;
			sample->get_sample = get_gpio_sample;
			sample++;
		}
	}

	for (int i=0; i < CONFIG_PWM_CHANNELS; i++){
		PWMConfig *config = &(loggerConfig->PWMConfigs[i]);
		if (config->cfg.sampleRate != SAMPLE_DISABLED){
			sample->precision = config->loggingPrecision;
			sample->channelConfig = &(config->cfg);
			sample->intValue = NIL_SAMPLE;
			sample->channelIndex = i;
			sample->get_sample = get_pwm_sample;
			sample++;
		}
	}

	{
		GPSConfig *gpsConfig = &(loggerConfig->GPSConfigs);
		if (gpsConfig->latitudeCfg.sampleRate != SAMPLE_DISABLED){
			sample->precision = DEFAULT_GPS_POSITION_LOGGING_PRECISION;
			sample->channelConfig = &(gpsConfig->latitudeCfg);
			sample->intValue = NIL_SAMPLE;
			sample->channelIndex = gps_channel_latitude;
			sample->get_sample = get_gps_sample;
			sample++;
		}

		if (gpsConfig->longitudeCfg.sampleRate != SAMPLE_DISABLED){
			sample->precision = DEFAULT_GPS_POSITION_LOGGING_PRECISION;
			sample->channelConfig = &(gpsConfig->longitudeCfg);
			sample->intValue = NIL_SAMPLE;
			sample->channelIndex = gps_channel_longitude;
			sample->get_sample = get_gps_sample;
			sample++;
		}

		if (gpsConfig->speedCfg.sampleRate != SAMPLE_DISABLED){
			sample->precision = DEFAULT_GPS_SPEED_LOGGING_PRECISION;
			sample->channelConfig = &(gpsConfig->speedCfg);
			sample->intValue = NIL_SAMPLE;
			sample->channelIndex = gps_channel_speed;
			sample->get_sample = get_gps_sample;
			sample++;
		}

		if (gpsConfig->timeCfg.sampleRate != SAMPLE_DISABLED){
			sample->precision = DEFAULT_GPS_TIME_LOGGING_PRECISION;
			sample->channelConfig = &(gpsConfig->timeCfg);
			sample->intValue = NIL_SAMPLE;
			sample->channelIndex = gps_channel_time;
			sample->get_sample = get_gps_sample;
			sample++;
		}

		if (gpsConfig->satellitesCfg.sampleRate != SAMPLE_DISABLED){
			sample->precision = DEFAULT_GPS_SATELLITES_LOGGING_PRECISION;
			sample->channelConfig = &(gpsConfig->satellitesCfg);
			sample->intValue = NIL_SAMPLE;
			sample->channelIndex = gps_channel_satellites;
			sample->get_sample = get_gps_sample;
			sample++;
		}
	}

	{
		TrackConfig *trackConfig = &(loggerConfig->TrackConfigs);
		if (trackConfig->lapCountCfg.sampleRate != SAMPLE_DISABLED){
			sample->precision = DEFAULT_LAP_COUNT_LOGGING_PRECISION;
			sample->channelConfig = &(trackConfig->lapCountCfg);
			sample->intValue = NIL_SAMPLE;
			sample->channelIndex = lap_stat_channel_lapcount;
			sample->get_sample = get_lap_stat_sample;
			sample++;
		}

		if (trackConfig->lapTimeCfg.sampleRate != SAMPLE_DISABLED){
			sample->precision = DEFAULT_LAP_TIME_LOGGING_PRECISION;
			sample->channelConfig = &(trackConfig->lapTimeCfg);
			sample->intValue = NIL_SAMPLE;
			sample->channelIndex = lap_stat_channel_laptime;
			sample->get_sample = get_lap_stat_sample;
			sample++;
		}

		if (trackConfig->splitTimeCfg.sampleRate != SAMPLE_DISABLED){
			sample->precision = DEFAULT_LAP_TIME_LOGGING_PRECISION;
			sample->channelConfig = &(trackConfig->splitTimeCfg);
			sample->intValue = NIL_SAMPLE;
			sample->channelIndex = lap_stat_channel_splittime;
			sample->get_sample = get_lap_stat_sample;
			sample++;
		}

		if (trackConfig->distanceCfg.sampleRate != SAMPLE_DISABLED){
			sample->precision = DEFAULT_DISTANCE_LOGGING_PRECISION;
			sample->channelConfig = &(trackConfig->distanceCfg);
			sample->intValue = NIL_SAMPLE;
			sample->channelIndex = lap_stat_channel_distance;
			sample->get_sample = get_lap_stat_sample;
			sample++;
		}

		if (trackConfig->predTimeCfg.sampleRate != SAMPLE_DISABLED){
			sample->precision = DEFAULT_LAP_TIME_LOGGING_PRECISION;
			sample->channelConfig = &(trackConfig->predTimeCfg);
			sample->intValue = NIL_SAMPLE;
			sample->channelIndex = lap_stat_channel_predtime;
			sample->get_sample = get_lap_stat_sample;
			sample++;
		}
	}
}


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
	LoggerConfig * loggerConfig = getWorkingLoggerConfig();
	ADCConfig *ac = &(loggerConfig->ADCConfigs[channelId]);
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
	LoggerConfig *loggerConfig = getWorkingLoggerConfig();
	TimerConfig *c = &(loggerConfig->TimerConfigs[channelId]);
	unsigned int value = getTimerPeriod(channelId);
	float timerValue = 0;
	unsigned int scaling = c->calculatedScaling;
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
	LoggerConfig *loggerConfig = getWorkingLoggerConfig();
	PWMConfig *c = &(loggerConfig->PWMConfigs[channelId]);
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
		case gps_channel_latitude:
			value = getLatitude();
			break;
		case gps_channel_longitude:
			value = getLongitude();
			break;
		case gps_channel_speed:
			value = getGPSSpeed() *  0.621371192; //convert to MPH
			break;
		case gps_channel_time:
			value - getUTCTime();
			break;
		case gps_channel_satellites:
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
		case lap_stat_channel_lapcount:
			value = getLapCount();
			break;
		case lap_stat_channel_laptime:
			value = getLastLapTime();
			break;
		case lap_stat_channel_splittime:
			value = getLastSplitTime();
			break;
		case lap_stat_channel_distance:
			value = getDistance();
			break;
		case lap_stat_channel_predtime:
			value = get_predicted_time(getGPSSpeed());
			break;
		default:
			value = -1;
			break;
	}
	return value;
}

