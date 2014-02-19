/*
 * loggerData.c
 *
 *  Created on: Jun 1, 2012
 *      Author: brent
 */

#include "loggerData.h"
#include "loggerHardware.h"
#include "accelerometer.h"
#include "gps.h"
#include "linear_interpolate.h"
#include "predictive_timer.h"
#include "filter.h"

//Channel Filters
#define ACCEL_ALPHA 0.1
static Filter g_accel_filter[CONFIG_ACCEL_CHANNELS];

static void init_filters(){
	for (size_t i = 0; i < CONFIG_ACCEL_CHANNELS; i++){
		init_filter(&g_accel_filter[i], ACCEL_ALPHA);
	}
}

void init_logger_data(){
	init_filters();
}

void doBackgroundSampling(){
	for (size_t i = ACCELEROMETER_CHANNEL_MIN; i <= ACCELEROMETER_CHANNEL_MAX; i++){
		update_filter(&g_accel_filter[i], readAccelChannel(i));
	}
}

float getAccelerometerValue(unsigned char accelChannel, AccelConfig *ac){
	size_t physicalChannel = ac->accelChannel;
	unsigned int raw = g_accel_filter[physicalChannel].current_value;
	float accelG = (accelChannel == ACCEL_CHANNEL_ZT ? YAW_RAW_TO_DEGREES_PER_SEC(raw ,ac->zeroValue) : ACCEL_RAW_TO_GFORCE(raw ,ac->zeroValue));

	//invert physical channel to match industry-standard accelerometer mappings
	switch(physicalChannel){
		case ACCEL_CHANNEL_X:
		case ACCEL_CHANNEL_Y:
		case ACCEL_CHANNEL_ZT:
			accelG = -accelG;
			break;
		default:
			break;
	}

	//now invert based on configuration
	switch (ac->mode){
	case MODE_ACCEL_NORMAL:
		break;
	case MODE_ACCEL_INVERTED:
		accelG = -accelG;
		break;
	case MODE_ACCEL_DISABLED:
	default:
		accelG = 0;
		break;
	}
	return accelG;
}

static void flushAccelBuffer(size_t physicalChannel){
	for (size_t i = 0; i < 1000; i++){
		readAccelChannel(physicalChannel);
	}
}

void calibrateAccelZero(){
	for (int i = ACCELEROMETER_CHANNEL_MIN; i <= ACCELEROMETER_CHANNEL_MAX; i++){
		AccelConfig * c = getAccelConfigChannel(i);
		size_t physicalChannel = c->accelChannel;
		flushAccelBuffer(physicalChannel);
		unsigned int zeroValue = g_accel_filter[physicalChannel].current_value;
		//adjust for gravity
		if (c->accelChannel == ACCEL_CHANNEL_Z) zeroValue-= (ACCEL_COUNTS_PER_G * (c->mode != MODE_ACCEL_INVERTED ? 1 : -1));
		c->zeroValue = zeroValue;
	}
}

static int writeAccelerometer(SampleRecord *sampleRecord, size_t currentTicks, LoggerConfig *config){
	int rate = SAMPLE_DISABLED;
	for (unsigned int i=0; i < CONFIG_ACCEL_CHANNELS;i++){
		AccelConfig *ac = &(config->AccelConfigs[i]);
		size_t sr = ac->cfg.sampleRate;
		if (sr != SAMPLE_DISABLED && (currentTicks % sr) == 0){
			rate = HIGHER_SAMPLE_RATE(sr, rate);
			sampleRecord->AccelSamples[i].floatValue = getAccelerometerValue(i, ac);
		}
	}
	return rate;
}

static int writeADC(SampleRecord *sampleRecord, size_t currentTicks, LoggerConfig *config){
	int rate = SAMPLE_DISABLED;
	unsigned int adc[CONFIG_ADC_CHANNELS];
	int adcRead = 0;

	for (unsigned int i=0; i < CONFIG_ADC_CHANNELS;i++){
		ADCConfig *ac = &(config->ADCConfigs[i]);
		size_t sr = ac->cfg.sampleRate;
		if (sr != SAMPLE_DISABLED){
			if ((currentTicks % sr) == 0){
				rate = HIGHER_SAMPLE_RATE(sr,rate);
				if (!adcRead){
					readAllADC(&adc[0],&adc[1],&adc[2],&adc[3],&adc[4],&adc[5],&adc[6],&adc[7]);
					adcRead = 1;
				}
				float analogValue = 0;
				switch(ac->scalingMode){
				case SCALING_MODE_RAW:
					analogValue = adc[i];
					break;
				case SCALING_MODE_LINEAR:
					analogValue = (ac->linearScaling * (float)adc[i]);
					break;
				case SCALING_MODE_MAP:
					analogValue = GetMappedValue((float)adc[i],&(ac->scalingMap));
					break;
				}
				if (ac->loggingPrecision == 0){
					sampleRecord->ADCSamples[i].intValue = analogValue;
				}else{
					sampleRecord->ADCSamples[i].floatValue = analogValue;
				}
			}
		}
	}
	return rate;
}

static int writeGPSChannels(SampleRecord *sampleRecord, size_t currentTicks, GPSConfig *config){
	int rate = SAMPLE_DISABLED;
	{
		size_t sr = config->timeCfg.sampleRate;
		if (sr != SAMPLE_DISABLED){
			if ((currentTicks % sr) == 0){
				rate = HIGHER_SAMPLE_RATE(sr, rate);
				sampleRecord->GPS_TimeSample.floatValue = getUTCTime();
			}
		}
	}
	{
		//latitude sample rate is a stand-in for position sample rate
		size_t sr = config->latitudeCfg.sampleRate;
		if (sr != SAMPLE_DISABLED){
			if ((currentTicks % sr) == 0){
				rate = HIGHER_SAMPLE_RATE(sr, rate);
				sampleRecord->GPS_LatitueSample.floatValue = getLatitude();
				sampleRecord->GPS_LongitudeSample.floatValue = getLongitude();
			}
		}
	}

	{
		size_t sr = config->satellitesCfg.sampleRate;
		if (sr != SAMPLE_DISABLED){
			if ((currentTicks % sr) == 0){
				rate = HIGHER_SAMPLE_RATE(sr, rate);
				sampleRecord->GPS_SatellitesSample.intValue = getSatellitesUsedForPosition();
			}
		}
	}

	{
		size_t sr = config->speedCfg.sampleRate;
		if (sr != SAMPLE_DISABLED){
			if ((currentTicks % sr) == 0){
				rate = HIGHER_SAMPLE_RATE(sr, rate);
				sampleRecord->GPS_SpeedSample.floatValue = getGPSSpeed() *  0.621371192; //convert to MPH
			}
		}
	}
	return rate;
}

static int writeTrackChannels(SampleRecord *sampleRecord, size_t currentTicks, TrackConfig *config){
	int rate = SAMPLE_DISABLED;
	{
		size_t sr = config->lapCountCfg.sampleRate;
		if (sr != SAMPLE_DISABLED){
			if ((currentTicks % sr) == 0){
				rate = HIGHER_SAMPLE_RATE(sr, rate);
				sampleRecord->Track_LapCountSample.intValue = getLapCount();
			}
		}
	}
	{
		size_t sr = config->splitTimeCfg.sampleRate;
		if (sr != SAMPLE_DISABLED){
			if ((currentTicks % sr) == 0){
				rate = HIGHER_SAMPLE_RATE(sr, rate);
				sampleRecord->Track_SplitTimeSample.floatValue = getLastSplitTime();
			}
		}
	}
	{
		size_t sr = config->lapTimeCfg.sampleRate;
		if (sr != SAMPLE_DISABLED){
			if ((currentTicks % sr) == 0){
				rate = HIGHER_SAMPLE_RATE(sr, rate);
				sampleRecord->Track_LapTimeSample.floatValue = getLastLapTime();
			}
		}
	}
	{
		size_t sr = config->distanceCfg.sampleRate;
		if (sr != SAMPLE_DISABLED){
			if ((currentTicks % sr) == 0){
				rate = HIGHER_SAMPLE_RATE(sr, rate);
				sampleRecord->Track_DistanceSample.floatValue = getDistance() * 0.621371192; //convert to miles
			}
		}
	}
	{
		size_t sr = config->predTimeCfg.sampleRate;
		if (sr != SAMPLE_DISABLED){
			if ((currentTicks % sr) == 0){
				rate = HIGHER_SAMPLE_RATE(sr, rate);
				sampleRecord->Track_PredTimeSample.floatValue = get_predicted_time(getGPSSpeed());
			}
		}
	}
	return rate;
}

static int writeGPIOs(SampleRecord *sampleRecord, size_t currentTicks, LoggerConfig *loggerConfig){
	int rate = SAMPLE_DISABLED;
	unsigned int gpio[CONFIG_GPIO_CHANNELS];
	readGpios(&gpio[0], &gpio[1], &gpio[2]);

	for (int i = 0; i < CONFIG_GPIO_CHANNELS; i++){
		GPIOConfig *c = &(loggerConfig->GPIOConfigs[i]);
		size_t sr = c->cfg.sampleRate;
		if (sr != SAMPLE_DISABLED){
			if ((currentTicks % sr) == 0){
				rate = HIGHER_SAMPLE_RATE(sr, rate);
				sampleRecord->GPIOSamples[i].intValue = gpio[i];
			}
		}
	}
	return rate;
}

static int writeTimerChannels(SampleRecord *sampleRecord, size_t currentTicks, LoggerConfig *loggerConfig){
	int rate = SAMPLE_DISABLED;
	unsigned int timers[CONFIG_TIMER_CHANNELS];
	getAllTimerPeriods(timers,timers + 1,timers + 2);

	for (int i = 0; i < CONFIG_TIMER_CHANNELS; i++){
		TimerConfig *c = &(loggerConfig->TimerConfigs[i]);
		size_t sr = c->cfg.sampleRate;
		if (sr != SAMPLE_DISABLED){
			if ((currentTicks % sr) == 0){
				rate = HIGHER_SAMPLE_RATE(sr, rate);
				int value = 0;
				unsigned scaling = c->calculatedScaling;
				unsigned int timerValue = timers[i];
				switch (c->mode){
					case MODE_LOGGING_TIMER_RPM:
						value = TIMER_PERIOD_TO_RPM(timerValue, scaling);
						break;
					case MODE_LOGGING_TIMER_FREQUENCY:
						value = TIMER_PERIOD_TO_HZ(timerValue, scaling);
						break;
					case MODE_LOGGING_TIMER_PERIOD_MS:
						value = TIMER_PERIOD_TO_MS(timerValue, scaling);
						break;
					case MODE_LOGGING_TIMER_PERIOD_USEC:
						value = TIMER_PERIOD_TO_USEC(timerValue, scaling);
						break;
					default:
						value = -1;
						break;
				}
				if (c->loggingPrecision == 0){
					sampleRecord->TimerSamples[i].intValue = value;
				}
				else{
					sampleRecord->TimerSamples[i].floatValue = value;
				}
			}
		}
	}
	return rate;
}

static int writePWMChannels(SampleRecord *sampleRecord, size_t currentTicks, LoggerConfig *loggerConfig){
	int rate = SAMPLE_DISABLED;
	for (int i = 0; i < CONFIG_PWM_CHANNELS; i++){
		PWMConfig *c = &(loggerConfig->PWMConfigs[i]);
		size_t sr = c->cfg.sampleRate;
		if (sr != SAMPLE_DISABLED){
			if ((currentTicks % sr) == 0){
				rate = HIGHER_SAMPLE_RATE(sr, rate);
				float value = 0;
				switch (c->loggingMode){
					case MODE_LOGGING_PWM_PERIOD:
						value = PWM_GetPeriod(i);
						break;
					case MODE_LOGGING_PWM_DUTY:
						value = PWM_GetDutyCycle(i);
						break;
					case MODE_LOGGING_PWM_VOLTS:
						value =  PWM_GetDutyCycle(i) * c->voltageScaling;
						break;
					default:
						break;
				}
				if (c->loggingPrecision == 0){
					sampleRecord->PWMSamples[i].intValue = value;
				}
				else{
					sampleRecord->PWMSamples[i].floatValue = value;
				}
			}
		}
	}
	return rate;
}


int populateSampleRecord(SampleRecord *sr, size_t currentTicks, LoggerConfig *config){
	int highestRate = SAMPLE_DISABLED;

	//Write ADC channels
	{
		int rate = writeADC(sr, currentTicks, config);
		highestRate = HIGHER_SAMPLE_RATE(rate, highestRate);
	}

	//Write GPIO channels
	{
		int rate = writeGPIOs(sr,currentTicks, config);
		highestRate = HIGHER_SAMPLE_RATE(rate, highestRate);
	}

	//Write Timer channels
	{
		int rate = writeTimerChannels(sr,currentTicks, config);
		highestRate = HIGHER_SAMPLE_RATE(rate, highestRate);

	}

	//Write PWM channels
	{
		int rate = writePWMChannels(sr,currentTicks, config);
		highestRate = HIGHER_SAMPLE_RATE(rate, highestRate);
	}

	//Write Accelerometer
	{
		int rate = writeAccelerometer(sr,currentTicks, config);
		highestRate = HIGHER_SAMPLE_RATE(rate, highestRate);
	}

	//Write GPS
	if ((unsigned int)config->GPSConfigs.GPSInstalled){
		{
			int rate = writeGPSChannels(sr,currentTicks, &(config->GPSConfigs));
			highestRate = HIGHER_SAMPLE_RATE(rate, highestRate);
		}
		{
			int rate = writeTrackChannels(sr, currentTicks, &(config->TrackConfigs));
			highestRate = HIGHER_SAMPLE_RATE(rate, highestRate);
		}
	}
	return highestRate;
}


float GetMappedValue(float value, ScalingMap *scalingMap){
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
