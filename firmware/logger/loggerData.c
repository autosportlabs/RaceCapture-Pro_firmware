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


static void writeAccelerometer(SampleRecord *sampleRecord, portTickType currentTicks, LoggerConfig *config){

	unsigned int accelValues[CONFIG_ACCEL_CHANNELS];

	for (unsigned int i=0; i < CONFIG_ACCEL_CHANNELS;i++){
		AccelConfig *ac = &(config->AccelConfigs[i]);
		portTickType sr = ac->cfg.sampleRate;
		if (sr != SAMPLE_DISABLED && (currentTicks % sr) == 0){
			accelValues[i] = readAccelChannel(ac->accelChannel);
		}
	}

	for (unsigned int i=0; i < CONFIG_ACCEL_CHANNELS;i++){
		AccelConfig *ac = &(config->AccelConfigs[i]);
		portTickType sr = ac->cfg.sampleRate;
		if (sr != SAMPLE_DISABLED){
			if ((currentTicks % sr) == 0){
				float value = (i == ACCEL_CHANNEL_ZT ? convertYawRawToDegreesPerSec(accelValues[i],ac->zeroValue) : convertAccelRawToG(accelValues[i],ac->zeroValue));
				sampleRecord->AccelSamples[i].floatValue = value;
			}
		}
	}
}

static void writeADC(SampleRecord *sampleRecord, portTickType currentTicks, LoggerConfig *config){

	unsigned int adc[CONFIG_ADC_CHANNELS];
	readAllADC(&adc[0],&adc[1],&adc[2],&adc[3],&adc[4],&adc[5],&adc[6],&adc[7]);

	for (unsigned int i=0; i < CONFIG_ADC_CHANNELS;i++){
		ADCConfig *ac = &(config->ADCConfigs[i]);
		portTickType sr = ac->cfg.sampleRate;
		if (sr != SAMPLE_DISABLED){
			if ((currentTicks % sr) == 0){
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
				sampleRecord->ADCSamples[i].floatValue = analogValue;
			}
		}
	}
}

static void writeGPSChannels(SampleRecord *sampleRecord, portTickType currentTicks, GPSConfig *config){

	{
		portTickType sr = config->timeCfg.sampleRate;
		if (sr != SAMPLE_DISABLED){
			if ((currentTicks % sr) == 0) sampleRecord->GPS_TimeSample.floatValue = getUTCTime();
		}
	}
	{
		//latitude sample rate is a stand-in for position sample rate
		portTickType sr = config->latitudeCfg.sampleRate;
		if (sr != SAMPLE_DISABLED){
			if ((currentTicks % sr) == 0){
				sampleRecord->GPS_LatitueSample.floatValue = getLatitude();
				sampleRecord->GPS_LongitudeSample.floatValue = getLongitude();
			}
		}
	}

	{
		portTickType sr = config->splitTimeCfg.sampleRate;
		if (sr != SAMPLE_DISABLED){
			if ((currentTicks % sr) == 0) sampleRecord->GPS_SplitTimeSample.floatValue = getLastSplitTime();
		}
	}

	{
		portTickType sr = config->satellitesCfg.sampleRate;
		if (sr != SAMPLE_DISABLED){
			if ((currentTicks % sr) == 0) sampleRecord->GPS_SatellitesSample.intValue = getSatellitesUsedForPosition();
		}
	}

	{
		portTickType sr = config->speedCfg.sampleRate;
		if (sr != SAMPLE_DISABLED){
			if ((currentTicks % sr) == 0) sampleRecord->GPS_SpeedSample.floatValue = getGPSSpeed();
		}
	}
	{
		portTickType sr = config->lapCountCfg.sampleRate;
		if (sr != SAMPLE_DISABLED){
			if ((currentTicks % sr) == 0) sampleRecord->GPS_LapCountSample.intValue = getLapCount();
		}
	}
	{
		portTickType sr = config->lapTimeCfg.sampleRate;
		if (sr != SAMPLE_DISABLED){
			if ((currentTicks % sr) == 0) sampleRecord->GPS_LapTimeSample.floatValue = getLastLapTime();
		}
	}
}

static void writeGPIOs(SampleRecord *sampleRecord, portTickType currentTicks, LoggerConfig *loggerConfig){

	unsigned int gpioMasks[CONFIG_GPIO_CHANNELS];

	gpioMasks[0] = GPIO_1;
	gpioMasks[1] = GPIO_2;
	gpioMasks[2] = GPIO_3;

	unsigned int gpioStates = AT91F_PIO_GetInput(AT91C_BASE_PIOA);
	for (int i = 0; i < CONFIG_GPIO_CHANNELS; i++){
		GPIOConfig *c = &(loggerConfig->GPIOConfigs[i]);
		portTickType sr = c->cfg.sampleRate;
		if (sr != SAMPLE_DISABLED){
			if ((currentTicks % sr) == 0) sampleRecord->GPIOSamples[i].intValue = ((gpioStates & gpioMasks[i]) != 0);
		}
	}
}

static void writeTimerChannels(SampleRecord *sampleRecord, portTickType currentTicks, LoggerConfig *loggerConfig){

	unsigned int timers[CONFIG_TIMER_CHANNELS];
	getAllTimerPeriods(timers,timers + 1,timers + 2);

	for (int i = 0; i < CONFIG_TIMER_CHANNELS; i++){
		TimerConfig *c = &(loggerConfig->TimerConfigs[i]);
		portTickType sr = c->cfg.sampleRate;
		if (sr != SAMPLE_DISABLED){
			if ((currentTicks % sr) == 0){
				int value = 0;
				int scaling = c->calculatedScaling;
				unsigned int timerValue = timers[i];
				switch (c->mode){
					case MODE_LOGGING_TIMER_RPM:
						value = calculateRPM(timerValue,scaling);
						break;
					case MODE_LOGGING_TIMER_FREQUENCY:
						value = calculateFrequencyHz(timerValue,scaling);
						break;
					case MODE_LOGGING_TIMER_PERIOD_MS:
						value = calculatePeriodMs(timerValue,scaling);
						break;
					case MODE_LOGGING_TIMER_PERIOD_USEC:
						value = calculatePeriodUsec(timerValue,scaling);
						break;
					default:
						value = -1;
						break;
				}
				sampleRecord->TimerSamples[i].intValue = value;
			}
		}
	}
}

static void writePWMChannels(SampleRecord *sampleRecord, portTickType currentTicks, LoggerConfig *loggerConfig){

	for (int i = 0; i < CONFIG_PWM_CHANNELS; i++){
		PWMConfig *c = &(loggerConfig->PWMConfigs[i]);
		portTickType sr = c->cfg.sampleRate;
		if (sr != SAMPLE_DISABLED){
			if ((currentTicks % sr) == 0){
				switch (c->loggingMode){
					case MODE_LOGGING_PWM_PERIOD:
						sampleRecord->PWMSamples[i].intValue = PWM_GetPeriod(i);
						break;
					case MODE_LOGGING_PWM_DUTY:
						sampleRecord->PWMSamples[i].intValue = PWM_GetDutyCycle(i);
						break;
					case MODE_LOGGING_PWM_VOLTS:
						sampleRecord->PWMSamples[i].floatValue =  PWM_GetDutyCycle(i) * c->voltageScaling;
						break;
					default:
						break;
				}
			}
		}
	}
}


void populateSampleRecord(SampleRecord *sr, portTickType currentTicks, LoggerConfig *config){

	//perform logging tasks
	unsigned int gpsInstalled = (unsigned int)config->GPSConfig.GPSInstalled;
	unsigned int accelInstalled = (unsigned int)config->AccelInstalled;

	//Write ADC channels
	writeADC(sr, currentTicks, config);
	//Write GPIO channels
	writeGPIOs(sr,currentTicks, config);
	//Write Timer channels
	writeTimerChannels(sr,currentTicks, config);
	//Write PWM channels
	writePWMChannels(sr,currentTicks, config);

	//Optional hardware
	//Write Accelerometer
	if (accelInstalled) writeAccelerometer(sr,currentTicks, config);
	//Write GPS
	if (gpsInstalled) writeGPSChannels(sr,currentTicks, &(config->GPSConfig));

}


//linear interpolation routine
//            (y2 - y1)
//  y = y1 +  --------- * (x - x1)
//            (x2 - x1)
static float LinearInterpolate(float x, float x1, float y1, float x2, float y2){
	return y1 + (((y2 - y1))  / (x2 - x1)) * (x - x1);
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
		nextBin = 0;
	}
	else{
		nextBin = bin;
		if (bin < ANALOG_SCALING_BINS - 1) nextBin++;
	}
	float x1 = (float)scalingMap->rawValues[bin];
	float y1 = scalingMap->scaledValues[bin];
	float x2 = (float)scalingMap->rawValues[nextBin];
	float y2 = scalingMap->scaledValues[nextBin];
	float scaled = LinearInterpolate(value,x1,y1,x2,y2);
	return scaled;

}


