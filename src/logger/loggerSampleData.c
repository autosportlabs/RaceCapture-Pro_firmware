/*
 * Race Capture Firmware
 *
 * Copyright (C) 2016 Autosport Labs
 *
 * This file is part of the Race Capture firmware suite
 *
 * This is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details. You should
 * have received a copy of the GNU General Public License along with
 * this code. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ADC.h"
#include "FreeRTOS.h"
#include "GPIO.h"
#include "OBD2.h"
#include "PWM.h"
#include "channel_config.h"
#include "dateTime.h"
#include "geopoint.h"
#include "gps.h"
#include "imu.h"
#include "lap_stats.h"
#include "linear_interpolate.h"
#include "loggerConfig.h"
#include "loggerData.h"
#include "loggerHardware.h"
#include "loggerSampleData.h"
#include "macros.h"
#include "predictive_timer_2.h"
#include "printk.h"
#include "sampleRecord.h"
#include "taskUtil.h"
#include "timer.h"
#include "units.h"
#include "virtual_channel.h"
#include <stdbool.h>

#define SAMPLE_CB_REGISTRY_SIZE	8

struct sample_cb_registry {
        logger_sample_cb_t* cb;
        void* data;
        int rate;
} sample_cb_registry[SAMPLE_CB_REGISTRY_SIZE];

static ChannelSample* processChannelSampleWithFloatGetter(ChannelSample *s,
        ChannelConfig *cfg,
        const size_t index,
        float (*getter)(int))
{
    if (cfg->sampleRate == SAMPLE_DISABLED)
        return s;

    s->cfg = cfg;
    s->channelIndex = index;
    s->sampleData = SampleData_Float;
    s->get_float_sample = getter;

    return ++s;
}

static ChannelSample* processChannelSampleWithIntGetter(ChannelSample *s,
        ChannelConfig *cfg,
        const size_t index,
        int (*getter)(int))
{
    if (cfg->sampleRate == SAMPLE_DISABLED)
        return s;

    s->cfg = cfg;
    s->channelIndex = index;
    s->sampleData = SampleData_Int;
    s->get_int_sample = getter;

    return ++s;
}

static ChannelSample* processChannelSampleWithFloatGetterNoarg(ChannelSample *s,
        ChannelConfig *cfg,
        float (*getter)())
{
    if (cfg->sampleRate == SAMPLE_DISABLED )
        return s;

    s->cfg = cfg;
    s->sampleData = SampleData_Float_Noarg;
    s->get_float_sample_noarg = getter;

    return ++s;
}

static ChannelSample* processChannelSampleWithIntGetterNoarg(ChannelSample *s,
        ChannelConfig *cfg,
        int (*getter)())
{
    if (cfg->sampleRate == SAMPLE_DISABLED )
        return s;

    s->cfg = cfg;
    s->sampleData = SampleData_Int_Noarg;
    s->get_int_sample_noarg = getter;

    return ++s;
}

static ChannelSample* processChannelSampleWithLongLongGetterNoarg(ChannelSample *s,
        ChannelConfig *cfg,
        long long (*getter)())
{
    if (cfg->sampleRate == SAMPLE_DISABLED )
        return s;

    s->cfg = cfg;
    s->sampleData = SampleData_LongLong_Noarg;
    s->get_longlong_sample_noarg = getter;

    return ++s;
}

/* XXX Implement custom data getters here XXX */

float get_mapped_value(float value, ScalingMap *scalingMap)
{
    float *bins;
    unsigned int bin, nextBin;

    bins = scalingMap->rawValues + ANALOG_SCALING_BINS - 1;
    bin = nextBin = ANALOG_SCALING_BINS - 1;

    while (value < *bins && bin > 0) {
        bins--;
        bin--;
    }
    if (bin == 0 && value < *bins) {
        return scalingMap->scaledValues[0];
    } else {
        nextBin = bin;
        if (bin < ANALOG_SCALING_BINS - 1) {
            nextBin++;
        } else {
            return scalingMap->scaledValues[ANALOG_SCALING_BINS - 1];
        }
    }
    float x1 = scalingMap->rawValues[bin];
    float y1 = scalingMap->scaledValues[bin];
    float x2 = scalingMap->rawValues[nextBin];
    float y2 = scalingMap->scaledValues[nextBin];
    float scaled = LinearInterpolate(value,x1,y1,x2,y2);
    return scaled;
}

#if ANALOG_CHANNELS > 0
float get_analog_sample(int channelId)
{
    LoggerConfig * loggerConfig = getWorkingLoggerConfig();
    ADCConfig *ac = &(loggerConfig->ADCConfigs[channelId]);
    float value = ADC_read(channelId);
    float analogValue = 0;
    switch(ac->scalingMode) {
    case SCALING_MODE_RAW:
        analogValue = value;
        break;
    case SCALING_MODE_LINEAR:
        analogValue = (ac->linearScaling * (float)value) + ac->linearOffset;
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
#endif

#if PWM_CHANNELS > 0
float get_pwm_sample(int channelId)
{
    LoggerConfig *loggerConfig = getWorkingLoggerConfig();
    PWMConfig *c = &(loggerConfig->PWMConfigs[channelId]);
    float pwmValue = 0;
    switch (c->loggingMode) {
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
#endif

#if IMU_CHANNELS > 0
float get_imu_sample(int channelId)
{
    LoggerConfig *config = getWorkingLoggerConfig();
    ImuConfig *c = &(config->ImuConfigs[channelId]);
    float value = imu_read_value(channelId, c);
    return value;
}
#endif

static void* get_altitude_getter(const ChannelConfig *cc)
{
	return UNIT_LENGTH_METERS == units_get_unit(cc->units) ?
		gps_get_altitude_meters : getAltitude;
}

static void* get_distance_getter(const ChannelConfig *cc)
{
	return UNIT_LENGTH_KILOMETERS == units_get_unit(cc->units) ?
		getLapDistance : getLapDistanceInMiles;
}

static void* get_speed_getter(const ChannelConfig *cc)
{
	return UNIT_SPEED_KILOMETERS_HOUR == units_get_unit(cc->units) ?
		getGPSSpeed : getGpsSpeedInMph;
}

void init_channel_sample_buffer(LoggerConfig *loggerConfig, struct sample *buff)
{
        buff->ticks = 0;
        ChannelSample *sample = buff->channel_samples;
        ChannelConfig *chanCfg;

    /*
     * This sets up immutable channels.  These channels are channels that are always
     * present.  They are always the first 2 channels, with the most reliable (Interval)
     * being the first channel.  Utc only works when we get synchronization from an external
     * clock.
     */
    struct TimeConfig *tc = &(loggerConfig->TimeConfigs[0]);
    chanCfg = &(tc->cfg);
    chanCfg->flags = ALWAYS_SAMPLED; // Set always sampled flag here so we always take samples
    sample = processChannelSampleWithIntGetterNoarg(sample, chanCfg, getUptimeAsInt);

    tc = &(loggerConfig->TimeConfigs[1]);
    chanCfg = &(tc->cfg);
    chanCfg->flags = ALWAYS_SAMPLED; // Set always sampled flag here so we always take samples
    sample = processChannelSampleWithLongLongGetterNoarg(sample, chanCfg, getMillisSinceEpochAsLongLong);

#if ANALOG_CHANNELS > 0
    for (int i=0; i < CONFIG_ADC_CHANNELS; i++) {
        ADCConfig *config = &(loggerConfig->ADCConfigs[i]);
        chanCfg = &(config->cfg);
        sample = processChannelSampleWithFloatGetter(sample, chanCfg, i, get_analog_sample);
    }
#endif

#if IMU_CHANNELS > 0
    for (int i = 0; i < CONFIG_IMU_CHANNELS; i++) {
        ImuConfig *config = &(loggerConfig->ImuConfigs[i]);
        chanCfg = &(config->cfg);
        sample = processChannelSampleWithFloatGetter(sample, chanCfg, i, get_imu_sample);
    }
#endif

#if TIMER_CHANNELS > 0
    for (int i=0; i < CONFIG_TIMER_CHANNELS; i++) {
        TimerConfig *config = &(loggerConfig->TimerConfigs[i]);
        chanCfg = &(config->cfg);
        sample = processChannelSampleWithFloatGetter(sample, chanCfg, i, timer_get_sample);
    }
#endif

#if GPIO_CHANNELS > 0
    for (int i=0; i < CONFIG_GPIO_CHANNELS; i++) {
        GPIOConfig *config = &(loggerConfig->GPIOConfigs[i]);
        chanCfg = &(config->cfg);
        sample = processChannelSampleWithIntGetter(sample, chanCfg, i, GPIO_get);
    }
#endif

#if PWM_CHANNELS > 0
    for (int i=0; i < CONFIG_PWM_CHANNELS; i++) {
        PWMConfig *config = &(loggerConfig->PWMConfigs[i]);
        chanCfg = &(config->cfg);
        sample = processChannelSampleWithFloatGetter(sample, chanCfg, i, get_pwm_sample);
    }
#endif

    OBD2Config *obd2Config = &(loggerConfig->OBD2Configs);
    const unsigned char enabled = loggerConfig->OBD2Configs.enabled;
    for (size_t i = 0; i < obd2Config->enabledPids && enabled; i++) {
        chanCfg = &(obd2Config->pids[i].cfg);
        sample = processChannelSampleWithIntGetter(sample, chanCfg, i,
                                                   OBD2_get_current_PID_value);
    }

#if VIRTUAL_CHANNEL_SUPPORT
    const size_t virtualChannelCount = get_virtual_channel_count();
    for (size_t i = 0; i < virtualChannelCount; i++) {
        VirtualChannel *vc = get_virtual_channel(i);
        chanCfg = &(vc->config);
        sample = processChannelSampleWithFloatGetter(sample, chanCfg, i, get_virtual_channel_value);
    }
#endif /* VIRTUAL_CHANNEL_SUPPORT */

    GPSConfig *gpsConfig = &(loggerConfig->GPSConfigs);
    chanCfg = &(gpsConfig->latitude);
    sample = processChannelSampleWithFloatGetterNoarg(sample, chanCfg, GPS_getLatitude);
    chanCfg = &(gpsConfig->longitude);
    sample = processChannelSampleWithFloatGetterNoarg(sample, chanCfg, GPS_getLongitude);
    chanCfg = &(gpsConfig->speed);
    sample = processChannelSampleWithFloatGetterNoarg(sample, chanCfg,
						      get_speed_getter(chanCfg));
    chanCfg = &(gpsConfig->distance);
    sample = processChannelSampleWithFloatGetterNoarg(sample, chanCfg,
						      get_distance_getter(chanCfg));
    chanCfg = &(gpsConfig->altitude);
    sample = processChannelSampleWithFloatGetterNoarg(sample, chanCfg,
						      get_altitude_getter(chanCfg));
    chanCfg = &(gpsConfig->satellites);
    sample = processChannelSampleWithIntGetterNoarg(sample, chanCfg, GPS_getSatellitesUsedForPosition);
    chanCfg = &(gpsConfig->quality);
    sample = processChannelSampleWithIntGetterNoarg(sample, chanCfg, GPS_getQuality);
    chanCfg = &(gpsConfig->DOP);
    sample = processChannelSampleWithFloatGetterNoarg(sample, chanCfg, GPS_getDOP);


    LapConfig *trackConfig = &(loggerConfig->LapConfigs);
    chanCfg = &(trackConfig->lapCountCfg);
    sample = processChannelSampleWithIntGetterNoarg(sample, chanCfg, getLapCount);
    chanCfg = &(trackConfig->lapTimeCfg);
    sample = processChannelSampleWithFloatGetterNoarg(sample, chanCfg, getLastLapTimeInMinutes);
    chanCfg = &(trackConfig->sectorCfg);
    sample = processChannelSampleWithIntGetterNoarg(sample, chanCfg, getSector);
    chanCfg = &(trackConfig->sectorTimeCfg);
    sample = processChannelSampleWithFloatGetterNoarg(sample, chanCfg, getLastSectorTimeInMinutes);
    chanCfg = &(trackConfig->predTimeCfg);
    sample = processChannelSampleWithFloatGetterNoarg(sample, chanCfg,
             getPredictedTimeInMinutes);
    chanCfg = &(trackConfig->elapsed_time_cfg);
    sample = processChannelSampleWithFloatGetterNoarg(sample, chanCfg,
             lapstats_elapsed_time_minutes);
    chanCfg = &(trackConfig->current_lap_cfg);
    sample = processChannelSampleWithIntGetterNoarg(sample, chanCfg,
             lapstats_current_lap);
}

static void populate_channel_sample(ChannelSample *sample)
{
    size_t channelIndex = sample->channelIndex;

    switch(sample->sampleData) {
    case SampleData_Int_Noarg:
        sample->valueInt = sample->get_int_sample_noarg();
        break;
    case SampleData_Int:
        sample->valueInt = sample->get_int_sample(channelIndex);
        break;
    case SampleData_LongLong_Noarg:
        sample->valueLongLong = sample->get_longlong_sample_noarg();
        break;
    case SampleData_LongLong:
        sample->valueLongLong = sample->get_longlong_sample(channelIndex);
        break;
    case SampleData_Float_Noarg:
        sample->valueFloat = sample->get_float_sample_noarg();
        break;
    case SampleData_Float:
        sample->valueFloat = sample->get_float_sample(channelIndex);
        break;
    case SampleData_Double_Noarg:
        sample->valueDouble = sample->get_double_sample_noarg();
        break;
    case SampleData_Double:
        sample->valueDouble = sample->get_double_sample(channelIndex);
        break;
    default:
        pr_warning("populate channel sample: unknown sample type");
        sample->valueLongLong = -1;
        break;
    }
}

int populate_sample_buffer(struct sample *s, size_t logTick)
{
    unsigned short highestRate = SAMPLE_DISABLED;
    ChannelSample *samples = s->channel_samples;
    const size_t count = s->channel_count;
    s->ticks = logTick;

    for (size_t i = 0; i < count; i++, samples++) {
        const unsigned short sampleRate = samples->cfg->sampleRate;

        if (logTick % sampleRate != 0) {
            samples->populated = false;
            continue;
        }

        highestRate = getHigherSampleRate(sampleRate, highestRate);
        samples->populated = true;
        populate_channel_sample(samples);
    }

    // Check if we got a sample.  If not, then bypass the rest as we are done.
    if (highestRate == SAMPLE_DISABLED)
        return SAMPLE_DISABLED;

    // If there was a sample taken, now we fill in the always sampled fields.
    samples = s->channel_samples;
    for (size_t i = 0; i < count; i++, samples++) {
        const int isAlwaysSampled = samples->cfg->flags & ALWAYS_SAMPLED;
        if (!isAlwaysSampled)
            continue;

        samples->populated = true;
        populate_channel_sample(samples);
    }

    return highestRate;
}


static bool is_valid_registry_index(const int idx)
{
        return (size_t) idx < ARRAY_LEN(sample_cb_registry);
}

void logger_sample_process_callbacks(const int ticks,
                                     const struct sample* sample)
{
        for (int i = 0; is_valid_registry_index(i); ++i) {
                struct sample_cb_registry* slot = sample_cb_registry + i;
                if (slot->cb && should_sample(ticks, slot->rate))
                        slot->cb(sample, ticks, slot->data);
        }
}

/**
 * Sets up a callback for logger samples at the specified rate.  An optinal
 * user defined argument allows the caller to include context when they get
 * the callback so that they may send out data appropriately.
 * @param cb The method to call back.
 * @param rate The sample rate that is desired.
 * @param data User defined data that will be provided to the callback.
 * @return A handle that references this callback's registration id; else -1
 * if there was an error.
 */
int logger_sample_create_callback(logger_sample_cb_t* cb, const int rate,
				  void* data)
{
        for (int i = 0; cb && is_valid_registry_index(i); ++i) {
                struct sample_cb_registry* slot = sample_cb_registry + i;
                if (slot->cb)
			continue;

		/* If here then we found a slot */
		slot->cb = cb;
		slot->data = data;
		slot->rate = encodeSampleRate(rate);
		return i;
        }

        return -1;
}

/**
 * Destroys a logger callback created by the #logger_sample_create_callback method.
 * Requires the handle returned by the creation process to destroy it.
 */
bool logger_sample_destroy_callback(const int handle)
{
	if (!is_valid_registry_index(handle))
		return false;

	struct sample_cb_registry* slot = sample_cb_registry + handle;
	memset(slot, 0, sizeof(struct sample_cb_registry));
	return true;
}
