#include "dateTime.h"
#include "loggerSampleData.h"
#include "loggerHardware.h"
#include "loggerConfig.h"
#include "loggerData.h"
#include "virtual_channel.h"
#include "imu.h"
#include "ADC.h"
#include "timer.h"
#include "PWM.h"
#include "GPIO.h"
#include "OBD2.h"
#include "sampleRecord.h"
#include "gps.h"
#include "geopoint.h"
#include "predictive_timer_2.h"
#include "linear_interpolate.h"
#include "printk.h"


int get_uptime_sample(int i);
long long get_utc_sample(int i);


int populate_sample_buffer(ChannelSample * samples,  size_t count, size_t currentTicks) {
   unsigned short highestRate = SAMPLE_DISABLED;

   for (size_t i = 0; i < count; i++, samples++) {
      const unsigned short sampleRate = samples->sampleRate;

      // Zero out the sample for sanity
      samples->valueLongLong = 0ll;

      if (currentTicks % sampleRate != 0)
         continue;

      highestRate = HIGHER_SAMPLE_RATE(sampleRate, highestRate);
      size_t channelIndex = samples->channelIndex;

      switch(samples->sampleData) {
      case SampleData_Int:
         samples->valueInt = samples->get_int_sample(channelIndex);
         break;
      case SampleData_Float:
         samples->valueFloat = samples->get_float_sample(channelIndex);

         // XXX: Kept for legacy purposes.  Should remove in the future.
         if (get_channel(samples->channelId)->precision == 0)
            samples->valueInt = (int) samples->valueFloat;

         break;
      case SampleData_LongLong:
         samples->valueLongLong = samples->get_ll_sample(channelIndex);
         break;
      default:
         pr_warning("Got into supposedly unreachable area in populate_sample_buffer");
      }
   }

   return highestRate;
}

static void setFloatGetter(ChannelSample *s, float (*float_getter)(int)) {
   s->sampleData = SampleData_Float;
   s->get_float_sample = float_getter;
}


void init_channel_sample_buffer(LoggerConfig *loggerConfig, ChannelSample * samples, size_t channelCount){
   ChannelSample *sample = samples;
   const unsigned int highSampleRate = getHighestSampleRate(loggerConfig);

   /*
    * This sets up immutable channels.  These channels are channels that are always
    * present.  They are always the first 2 channels, with the most reliable (Interval)
    * being the first channel.  Utc only works when we get synchronization from an external
    * clock.
    */
   struct TimeConfig *tc = &(loggerConfig->TimeConfigs[0]);
   ChannelConfig *cc = &(tc->cfg);
   sample->channelId = cc->channeId;
   sample->channelIndex = 0;
   sample->get_int_sample = get_uptime_sample;
   sample->sampleData = SampleData_Int;
   //Always use the highestSampleRate for our Time values.
   sample->sampleRate = highSampleRate;
   ++sample;

   tc = &(loggerConfig->TimeConfigs[1]);
   cc = &(tc->cfg);
   sample->channelId = cc->channeId;
   sample->channelIndex = 1;
   sample->sampleData = SampleData_LongLong;
   sample->get_ll_sample = get_utc_sample;
   //Always use the highestSampleRate for our Time values.
   sample->sampleRate = highSampleRate;
   ++sample;

	for (int i=0; i < CONFIG_ADC_CHANNELS; i++){
		ADCConfig *config = &(loggerConfig->ADCConfigs[i]);
		if (config->cfg.sampleRate != SAMPLE_DISABLED){
			cc = &(config->cfg);
			sample->channelId = cc->channeId;
			sample->sampleRate = cc->sampleRate;
			sample->valueInt = NIL_SAMPLE;
			sample->channelIndex = i;
                        setFloatGetter(sample, get_analog_sample);
			sample++;
		}
	}

	for (int i = 0; i < CONFIG_IMU_CHANNELS; i++){
		ImuConfig *config = &(loggerConfig->ImuConfigs[i]);
		if (config->cfg.sampleRate != SAMPLE_DISABLED){
			cc = &(config->cfg);
			sample->channelId = cc->channeId;
			sample->sampleRate = cc->sampleRate;
			sample->valueInt = NIL_SAMPLE;
			sample->channelIndex = i;
                        setFloatGetter(sample, get_imu_sample);
			sample++;
		}
	}

	for (int i=0; i < CONFIG_TIMER_CHANNELS; i++){
		TimerConfig *config = &(loggerConfig->TimerConfigs[i]);
		if (config->cfg.sampleRate != SAMPLE_DISABLED){
			cc = &(config->cfg);
			sample->channelId = cc->channeId;
			sample->sampleRate = cc->sampleRate;
			sample->valueInt = NIL_SAMPLE;
			sample->channelIndex = i;
                        setFloatGetter(sample, get_timer_sample); // here
			sample++;
		}
	}

	for (int i=0; i < CONFIG_GPIO_CHANNELS; i++){
		GPIOConfig *config = &(loggerConfig->GPIOConfigs[i]);
		if (config->cfg.sampleRate != SAMPLE_DISABLED){
			cc = &(config->cfg);
			sample->channelId = cc->channeId;
			sample->sampleRate = cc->sampleRate;
			sample->valueInt = NIL_SAMPLE;
			sample->channelIndex = i;
                        setFloatGetter(sample, get_gpio_sample);
			sample++;
		}
	}

	for (int i=0; i < CONFIG_PWM_CHANNELS; i++){
		PWMConfig *config = &(loggerConfig->PWMConfigs[i]);
		if (config->cfg.sampleRate != SAMPLE_DISABLED){
			cc = &(config->cfg);
			sample->channelId = cc->channeId;
			sample->sampleRate = cc->sampleRate;
			sample->valueInt = NIL_SAMPLE;
			sample->channelIndex = i;
                        setFloatGetter(sample, get_pwm_sample);
			sample++;
		}
	}

	{
		OBD2Config *obd2Config = &(loggerConfig->OBD2Configs);
		size_t enabledPids = obd2Config->enabledPids;
		for (size_t i = 0; i < enabledPids; i++){
			cc = &obd2Config->pids[i].cfg;
			sample->channelId = cc->channeId;
			sample->sampleRate = cc->sampleRate;
			sample->valueInt = NIL_SAMPLE;
			sample->channelIndex = i;
                        setFloatGetter(sample, get_obd2_sample);
			sample++;
		}
	}
	{
		size_t virtualChannelCount = get_virtual_channel_count();
		for (size_t i = 0; i < virtualChannelCount; i++){
			VirtualChannel *vc = get_virtual_channel(i);
			cc = &vc->config;
			sample->channelId = cc->channeId;
			sample->sampleRate = cc->sampleRate;
			sample->valueInt = NIL_SAMPLE;
			sample->channelIndex = i;
                        setFloatGetter(sample, get_virtual_channel_value);
			sample++;
		}
	}
	{
		GPSConfig *gpsConfig = &(loggerConfig->GPSConfigs);
		unsigned short gpsSampleRate = gpsConfig->sampleRate;
		if (gpsSampleRate != SAMPLE_DISABLED){
			if (gpsConfig->positionEnabled){
				sample->channelId = CHANNEL_Latitude;
				sample->sampleRate = gpsSampleRate;
				sample->valueInt = NIL_SAMPLE;
				sample->channelIndex = gps_channel_latitude;
                                setFloatGetter(sample, get_gps_sample);
				sample++;

				sample->channelId = CHANNEL_Longitude;
				sample->sampleRate = gpsSampleRate;
				sample->valueInt = NIL_SAMPLE;
				sample->channelIndex = gps_channel_longitude;
                                setFloatGetter(sample, get_gps_sample);
				sample++;
			}

			if (gpsConfig->speedEnabled){
				sample->channelId = CHANNEL_Speed;
				sample->sampleRate = gpsSampleRate;
				sample->valueInt = NIL_SAMPLE;
				sample->channelIndex = gps_channel_speed;
                                setFloatGetter(sample, get_gps_sample);
				sample++;
			}

			if (gpsConfig->timeEnabled){
				sample->channelId = CHANNEL_Time;
				sample->sampleRate = gpsSampleRate;
				sample->valueInt = NIL_SAMPLE;
				sample->channelIndex = gps_channel_time;
                                setFloatGetter(sample, get_gps_sample);
				sample++;
			}

			if (gpsConfig->satellitesEnabled){
				sample->channelId = CHANNEL_GPSSats;
				sample->sampleRate = gpsSampleRate;
				sample->valueInt = NIL_SAMPLE;
				sample->channelIndex = gps_channel_satellites;
                                setFloatGetter(sample, get_gps_sample);
				sample++;
			}

			if (gpsConfig->distanceEnabled){
				sample->channelId = CHANNEL_Distance;
				sample->sampleRate = gpsSampleRate;
				sample->valueInt = NIL_SAMPLE;
				sample->channelIndex = gps_channel_distance;
                                setFloatGetter(sample, get_gps_sample);
				sample++;
			}
		}
	}

	{
		LapConfig *trackConfig = &(loggerConfig->LapConfigs);
		if (trackConfig->lapCountCfg.sampleRate != SAMPLE_DISABLED){
			cc = &(trackConfig->lapCountCfg);
			sample->channelId = cc->channeId;
			sample->sampleRate = cc->sampleRate;
			sample->valueInt = NIL_SAMPLE;
			sample->channelIndex = lap_stat_channel_lapcount;
                        setFloatGetter(sample, get_lap_stat_sample);
			sample++;
		}

		if (trackConfig->lapTimeCfg.sampleRate != SAMPLE_DISABLED){
			cc = &(trackConfig->lapTimeCfg);
			sample->channelId = cc->channeId;
			sample->sampleRate = cc->sampleRate;
			sample->valueInt = NIL_SAMPLE;
			sample->channelIndex = lap_stat_channel_laptime;
                        setFloatGetter(sample, get_lap_stat_sample);
			sample++;
		}

		if (trackConfig->sectorCfg.sampleRate != SAMPLE_DISABLED){
			cc = &(trackConfig->sectorCfg);
			sample->channelId = cc->channeId;
			sample->sampleRate = cc->sampleRate;
			sample->valueInt = NIL_SAMPLE;
			sample->channelIndex = lap_stat_channel_sector;
                        setFloatGetter(sample, get_lap_stat_sample);
			sample++;
		}

		if (trackConfig->sectorTimeCfg.sampleRate != SAMPLE_DISABLED){
			cc = &(trackConfig->sectorTimeCfg);
			sample->channelId = cc->channeId;
			sample->sampleRate = cc->sampleRate;
			sample->valueInt = NIL_SAMPLE;
			sample->channelIndex = lap_stat_channel_sectortime;
                        setFloatGetter(sample, get_lap_stat_sample);
			sample++;
		}

		if (trackConfig->predTimeCfg.sampleRate != SAMPLE_DISABLED){
			cc = &(trackConfig->predTimeCfg);
			sample->channelId = cc->channeId;
			sample->sampleRate = cc->sampleRate;
			sample->valueInt = NIL_SAMPLE;
			sample->channelIndex = lap_stat_channel_predtime;
                        setFloatGetter(sample, get_lap_stat_sample);
			sample++;
		}
	}
}


float get_mapped_value(float value, ScalingMap *scalingMap){
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

float get_time_sample(int index) {
   const LoggerConfig * loggerConfig = getWorkingLoggerConfig();
   const struct TimeConfig *tc = &(loggerConfig->TimeConfigs[index]);
   enum TimeType tt = tc->tt;

   switch(tt) {
   case TimeType_Uptime:
      // XXX: This won't work.  Temporary only.
      return (float) getUptime();
   case TimeType_UtcMillis:
      // XXX: This won't work.  Temporary only.
      return (float) getMillisSinceEpoch();
   }

   // Should never get here.
   return -1.0;
}

float get_analog_sample(int channelId){
	LoggerConfig * loggerConfig = getWorkingLoggerConfig();
	ADCConfig *ac = &(loggerConfig->ADCConfigs[channelId]);
	float value = ADC_read(channelId);
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
	unsigned int value = timer_get_period(channelId);
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
			pwmValue = PWM_channel_get_period(channelId);
			break;
		case MODE_LOGGING_PWM_DUTY:
			pwmValue = PWM_get_duty_cycle(channelId);
			break;
		case MODE_LOGGING_PWM_VOLTS:
			pwmValue = PWM_get_duty_cycle(channelId) * PWM_VOLTAGE_SCALING;
			break;
		default:
			pwmValue = -1;
			break;
	}
	return pwmValue;
}

float get_obd2_sample(int channelId){
	return (float)OBD2_get_current_PID_value(channelId);
}

float get_gpio_sample(int channelId){
	int gpioValue = GPIO_get(channelId);
	return (float)gpioValue;
}

float get_imu_sample(int channelId){
	LoggerConfig *config = getWorkingLoggerConfig();
	ImuConfig *c = &(config->ImuConfigs[channelId]);
	float value = imu_read_value(channelId, c);
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
         {
            /*
             * XXX: Hack.  Doing this until backend can take uint64_t and we
             *      can give time since epoch.  getUTCTime returned time as
             *      HHMMSS.MMM
             */
            const DateTime dt = getLastFixDateTime();
            value = ((float) dt.millisecond) / 1000;
            value += (float) dt.second;
            value += ((float) dt.minute) * 100;
            value += ((float) dt.hour) * 10000;
            break;
         }
		case gps_channel_distance:
			value = getGpsDistance();
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
      value = (float) getLapCount();
      break;
   case lap_stat_channel_laptime:
      // XXX: TIME_HACK.  Should send Millis only in future.
      value = tinyMillisToMinutes(getLastLapTime());
      break;
   case lap_stat_channel_sector:
      value = (float) getLastSector();
      break;
   case lap_stat_channel_sectortime:
      // XXX: TIME_HACK.  Should send Millis only in future.
      value = tinyMillisToMinutes(getLastSectorTime());
      break;
   case lap_stat_channel_predtime:
      {
         const GeoPoint gp = getGeoPoint();
         const tiny_millis_t millis = getMillisSinceFirstFix();
         value = tinyMillisToMinutes(getPredictedTime(gp, millis));
         break;
      }
   default:
      value = -1;
      break;
   }
   return value;
}

int get_uptime_sample(int i) {
   return (int) getUptime();
}

long long get_utc_sample(int i) {
   return (long long) getMillisSinceEpoch();
}
