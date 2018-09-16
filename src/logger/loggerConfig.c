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

#include "capabilities.h"
#include "channel_config.h"
#include "cpu.h"
#include "loggerConfig.h"
#include "memory.h"
#include "modp_numtoa.h"
#include "printk.h"
#include "str_util.h"
#include "timer_config.h"
#include "units.h"
#include "virtual_channel.h"
#include "stdutil.h"
#include <stdbool.h>
#include <string.h>

#define _LOG_PFX "[LoggerConfig] "

#ifndef RCP_TESTING
static const volatile LoggerConfig g_savedLoggerConfig  __attribute__((section(".config\n\t#")));
#else
static LoggerConfig g_savedLoggerConfig;
#endif

static LoggerConfig g_workingLoggerConfig;

static void resetVersionInfo(VersionInfo *vi)
{
    vi->major = MAJOR_REV;
    vi->minor = MINOR_REV;
    vi->bugfix = BUGFIX_REV;
}

/**
 * Prints a string prefix and add an int suffix to dest buffer.
 */
static void sPrintStrInt(char *dest, const char *str, const unsigned int i)
{
    char iStr[3];
    const int idx = strlen(str);

    modp_itoa10(i, iStr);
    strcpy(dest, str);
    strcpy(dest + idx, iStr);
}

static void resetTimeConfig(struct TimeConfig *tc)
{
    tc[0] = (struct TimeConfig) DEFAULT_UPTIME_TIME_CONFIG;
    tc[1] = (struct TimeConfig) DEFAULT_UTC_MILLIS_TIME_CONFIG;
}


#if PWM_CHANNELS > 0
PWMConfig * getPwmConfigChannel(int channel)
{
    PWMConfig * c = NULL;
    if (channel >= 0 && channel < CONFIG_PWM_CHANNELS) {
        c = &(getWorkingLoggerConfig()->PWMConfigs[channel]);
    }
    return c;
}

unsigned short filterPwmDutyCycle(int dutyCycle)
{
    if (dutyCycle > MAX_PWM_DUTY_CYCLE) {
        dutyCycle = MAX_PWM_DUTY_CYCLE;
    } else if (dutyCycle < MIN_PWM_DUTY_CYCLE) {
        dutyCycle = MIN_PWM_DUTY_CYCLE;
    }
    return dutyCycle;
}

unsigned short filterPwmPeriod(int period)
{
    if (period > MAX_PWM_PERIOD) {
        period = MAX_PWM_PERIOD;
    } else if (period < MIN_PWM_PERIOD) {
        period = MIN_PWM_PERIOD;
    }
    return period;
}

uint16_t filterPwmClockFrequency(uint16_t freq)
{
    if (freq > MAX_PWM_CLOCK_FREQUENCY) {
        freq = MAX_PWM_CLOCK_FREQUENCY;
    } else if (freq < MIN_PWM_CLOCK_FREQUENCY) {
        freq = MIN_PWM_CLOCK_FREQUENCY;
    }
    return freq;
}

static void resetPwmClkFrequency(unsigned short *pwmClkFreq)
{
    *pwmClkFreq = DEFAULT_PWM_CLOCK_FREQUENCY;
}

static void resetPwmConfig(PWMConfig cfg[])
{
    for (size_t i = 0; i < CONFIG_PWM_CHANNELS; ++i) {
        PWMConfig *c = cfg + i;
        *c = (PWMConfig) DEFAULT_PWM_CONFIG;
        sPrintStrInt(c->cfg.label, "PWM", i + 1);
    }
}

char filterPwmOutputMode(int value)
{
    switch(value) {
    case MODE_PWM_ANALOG:
        return MODE_PWM_ANALOG;
    case MODE_PWM_FREQUENCY:
    default:
        return MODE_PWM_FREQUENCY;
    }
}

char filterPwmLoggingMode(int config)
{
    switch (config) {
    case MODE_LOGGING_PWM_PERIOD:
        return MODE_LOGGING_PWM_PERIOD;
    case MODE_LOGGING_PWM_DUTY:
        return MODE_LOGGING_PWM_DUTY;
    case MODE_LOGGING_PWM_VOLTS:
    default:
        return MODE_LOGGING_PWM_VOLTS;
    }
}
#endif

#if GPIO_CHANNELS > 1
GPIOConfig * getGPIOConfigChannel(int channel)
{
    GPIOConfig *c = NULL;
    if (channel >=0 && channel < CONFIG_GPIO_CHANNELS) {
        c = &(getWorkingLoggerConfig()->GPIOConfigs[channel]);
    }
    return c;
}

char filterGpioMode(int value)
{
    switch(value) {
    case CONFIG_GPIO_OUT:
        return CONFIG_GPIO_OUT;
    case CONFIG_GPIO_IN:
    default:
        return CONFIG_GPIO_IN;
    }
}

static void resetGpioConfig(GPIOConfig cfg[])
{
    for (size_t i = 0; i < CONFIG_GPIO_CHANNELS; ++i) {
        GPIOConfig *c = cfg + i;
        *c = (GPIOConfig) DEFAULT_GPIO_CONFIG;
        sPrintStrInt(c->cfg.label, "GPIO", i + 1);
    }
}
#endif

#if IMU_CHANNELS > 0
static void resetImuConfig(ImuConfig cfg[])
{
        static const ImuConfig defaults[] = IMU_CONFIG_DEFAULTS;
        memcpy(cfg, defaults, sizeof(defaults));
}
#endif

static void resetCanConfig(CANConfig *cfg)
{
    cfg->enabled = CONFIG_FEATURE_INSTALLED;
    for (size_t i = 0; i < CONFIG_CAN_CHANNELS; i++) {
        cfg->baud[i] = DEFAULT_CAN_BAUD_RATE;
#if CAN_SW_TERMINATION == true
        cfg->termination[i] = true;
#endif
    }
}

uint8_t filter_can_bus_channel(uint8_t value)
{
        return MIN(value, CONFIG_CAN_CHANNELS - 1);
}

enum CANMappingType filter_can_mapping_type(enum CANMappingType mapping_type){
    if (mapping_type >= CANMappingType_ENUM_COUNT)
            mapping_type = CANMappingType_unsigned;
    return mapping_type;
}

static void _reset_can_mapping_config(CANChannelConfig *cfg)
{
    memset(cfg, 0, sizeof(CANChannelConfig));
    return;
}

static void resetOBD2Config(OBD2Config *cfg)
{
    memset(cfg, 0, sizeof(OBD2Config));

    for (int i = 0; i < OBD2_CHANNELS; ++i) {
        PidConfig *c = &cfg->pids[i];
        memset(c, 0, sizeof(PidConfig));
    }

    /* TODO BAP - hacked in some defaults */
    /* STIEG: This somehow fucks up the unit tests.
              How/why it does this... I have no freaking clue.
              Is it perhaps the label because "RPM" is the default label
              for the 0 timer pid.  Signs point to yes but I don't
              understand (yet) how this could possibly do this.
    cfg->enabled = 1;
    cfg->enabledPids = 1;
    cfg->pids[0].pid = 12;
    strcpy(cfg->pids[0].cfg.label, "RPM");
    cfg->pids[0].cfg.sampleRate = encodeSampleRate(10);
    cfg->pids[0].cfg.min = 0;
    cfg->pids[0].cfg.max = 10000;
    cfg->pids[0].cfg.precision = 0;
    */
}

void logger_config_reset_gps_config(GPSConfig *cfg)
{
	*cfg = (GPSConfig) DEFAULT_GPS_CONFIG;

#if GPS_HARDWARE_SUPPORT
	/* Setting here b/c this now uses units.h labels */
	strcpy(cfg->altitude.units, units_get_label(UNIT_LENGTH_FEET));
	strcpy(cfg->speed.units, units_get_label(UNIT_SPEED_MILES_HOUR));
#endif
 strcpy(cfg->distance.units, units_get_label(UNIT_LENGTH_MILES));
}

uint16_t logger_config_get_gps_sample_rate(void)
{
#if GPS_HARDWARE_SUPPORT
        return getWorkingLoggerConfig()->GPSConfigs.speed.sampleRate;
#else
        return 0;
#endif
}
static void resetLapConfig(LapConfig *cfg)
{
    *cfg = (LapConfig) DEFAULT_LAP_CONFIG;
}

static void resetTrackConfig(TrackConfig *cfg)
{
	memset(cfg, 0, sizeof(TrackConfig));
	cfg->radius = DEFAULT_TRACK_TARGET_RADIUS;
	cfg->auto_detect = true;
}

static void resetBluetoothConfig(BluetoothConfig *cfg)
{
        memset(cfg, 0, sizeof(BluetoothConfig));
        cfg->btEnabled = DEFAULT_BT_ENABLED;
}

static void resetCellularConfig(CellularConfig *cfg)
{
    memset(cfg, 0, sizeof(CellularConfig));
    cfg->cellEnabled = DEFAULT_CELL_ENABLED;
    strcpy(cfg->apnHost, DEFAULT_APN_HOST);
    strcpy(cfg->dns1, DEFAULT_DNS1);
    strcpy(cfg->dns2, DEFAULT_DNS2);
}

static void resetTelemetryConfig(TelemetryConfig *cfg)
{
	memset(cfg, 0, sizeof(TelemetryConfig));
	cfg->backgroundStreaming = BACKGROUND_STREAMING_ENABLED;
	strntcpy(cfg->telemetryServerHost, DEFAULT_TELEMETRY_SERVER_HOST,
		 sizeof(cfg->telemetryServerHost));
	cfg->telemetry_port = DEFAULT_TELEMETRY_SERVER_PORT;
}

static void resetConnectivityConfig(ConnectivityConfig *cfg)
{
        resetBluetoothConfig(&cfg->bluetoothConfig);
        resetCellularConfig(&cfg->cellularConfig);
        resetTelemetryConfig(&cfg->telemetryConfig);
        wifi_reset_config(&cfg->wifi);
}

static void reset_logging_config(struct logging_config *lc)
{
        memset(lc, 0, sizeof(struct logging_config));
}

bool isHigherSampleRate(const int contender, const int champ)
{
    // Contender can't win here.  Ever.
    if (contender == SAMPLE_DISABLED)
        return false;

    // Champ defaults in this case.  Contender need only show up.
    if (champ == SAMPLE_DISABLED)
        return contender != SAMPLE_DISABLED;

    return contender < champ;
}

int getHigherSampleRate(const int a, const int b)
{
    return isHigherSampleRate(a, b) ? a : b;
}

int getConnectivitySampleRateLimit()
{
    ConnectivityConfig *connConfig = &getWorkingLoggerConfig()->ConnectivityConfigs;
    int sampleRateLimit = connConfig->cellularConfig.cellEnabled ? SLOW_LINK_MAX_TELEMETRY_SAMPLE_RATE : FAST_LINK_MAX_TELEMETRY_SAMPLE_RATE;
    return sampleRateLimit;
}

/* Filter sample rates to only allow rates we support */
int encodeSampleRate(int sampleRate)
{
    if (sampleRate > MAX_SENSOR_SAMPLE_RATE){
        return SAMPLE_DISABLED;
    }
    switch(sampleRate) {
    case 1000:
        return SAMPLE_1000Hz;
    case 500:
        return SAMPLE_500Hz;
    case 200:
        return SAMPLE_200Hz;
    case 100:
        return SAMPLE_100Hz;
    case 50:
        return SAMPLE_50Hz;
    case 25:
        return SAMPLE_25Hz;
    case 10:
        return SAMPLE_10Hz;
    case 5:
        return SAMPLE_5Hz;
    case 1:
        return SAMPLE_1Hz;
    case 0:
    default:
        return SAMPLE_DISABLED;
    }
}

int decodeSampleRate(int rate_code)
{
    if (rate_code == 0) {
        return SAMPLE_DISABLED;
    }
    else {
        return TICK_RATE_HZ / rate_code;
    }
}

uint8_t filter_background_streaming_mode(uint8_t mode)
{
    return mode == 0 ? 0 : 1;
}

#if TIMER_CHANNELS > 0
unsigned short filterTimerDivider(unsigned short speed)
{
    switch(speed) {
    case TIMER_SLOW:
        return TIMER_SLOW;
    case TIMER_MEDIUM:
        return TIMER_MEDIUM;
    case TIMER_FAST:
        return TIMER_FAST;
    default:
        return TIMER_FAST;
    }
}
char filterTimerMode(int mode)
{
    switch (mode) {
    case MODE_LOGGING_TIMER_RPM:
        return MODE_LOGGING_TIMER_RPM;
    case MODE_LOGGING_TIMER_PERIOD_MS:
        return MODE_LOGGING_TIMER_PERIOD_MS;
    case MODE_LOGGING_TIMER_PERIOD_USEC:
        return MODE_LOGGING_TIMER_PERIOD_USEC;
    default:
    case MODE_LOGGING_TIMER_FREQUENCY:
        return MODE_LOGGING_TIMER_FREQUENCY;
    }
}
#endif

#if IMU_CHANNELS > 0
ImuConfig * getImuConfigChannel(int channel)
{
    ImuConfig * c = NULL;
    if (channel >= 0 && channel < CONFIG_IMU_CHANNELS) {
        c = &(getWorkingLoggerConfig()->ImuConfigs[channel]);
    }
    return c;
}

int filterImuChannel(int channel)
{
    return (channel < CONFIG_IMU_CHANNELS ? channel : CONFIG_IMU_CHANNELS - 1);
}

int filterImuMode(int mode)
{
        switch (mode) {
        case IMU_MODE_DISABLED:
                return IMU_MODE_DISABLED;
        case IMU_MODE_INVERTED:
                return IMU_MODE_INVERTED;
        case IMU_MODE_NORMAL:
        default:
                return IMU_MODE_NORMAL;
        }
}
#endif


#if ANALOG_CHANNELS > 0
static void resetAdcConfig(ADCConfig cfg[])
{
    // All but the last one are zeroed out.
    for (size_t i = 0; i < CONFIG_ADC_CHANNELS; ++i) {
        ADCConfig *c = cfg + i;
        *c = (ADCConfig) DEFAULT_ADC_CONFIG;
        sPrintStrInt(c->cfg.label, "Analog", i + 1);
        strcpy(c->cfg.units, "Volts");
    }

    // Now update the battery config
    cfg[CONFIG_ADC_CHANNELS - 1] = (ADCConfig) BATTERY_ADC_CONFIG;
}

ADCConfig * getADCConfigChannel(int channel)
{
    ADCConfig *c = NULL;
    if (channel >=0 && channel < CONFIG_ADC_CHANNELS) {
        c = &(getWorkingLoggerConfig()->ADCConfigs[channel]);
    }
    return c;
}

unsigned char filterAnalogScalingMode(unsigned char mode)
{
    switch(mode) {
    case SCALING_MODE_LINEAR:
        return SCALING_MODE_LINEAR;
    case SCALING_MODE_MAP:
        return SCALING_MODE_MAP;
    default:
    case SCALING_MODE_RAW:
        return SCALING_MODE_RAW;
    }
}
#endif

unsigned int getHighestSampleRate(LoggerConfig *config)
{
    int s = SAMPLE_DISABLED;
    int sr;

    for (int i = 0; i < CONFIG_TIME_CHANNELS; i++) {
            sr = config->TimeConfigs[i].cfg.sampleRate;
            s = getHigherSampleRate(sr, s);
    }

#if ANALOG_CHANNELS > 0
    for (int i = 0; i < CONFIG_ADC_CHANNELS; i++) {
        sr = config->ADCConfigs[i].cfg.sampleRate;
        s = getHigherSampleRate(sr, s);
    }
#endif

#if PWM_CHANNELS > 0
    for (int i = 0; i < CONFIG_PWM_CHANNELS; i++) {
        sr = config->PWMConfigs[i].cfg.sampleRate;
        s = getHigherSampleRate(sr, s);
    }
#endif

#if GPIO_CHANNELS > 1
    for (int i = 0; i < CONFIG_GPIO_CHANNELS; i++) {
        sr = config->GPIOConfigs[i].cfg.sampleRate;
        s = getHigherSampleRate(sr, s);
    }
#endif

#if TIMER_CHANNELS > 0
    for (int i = 0; i < CONFIG_TIMER_CHANNELS; i++) {
        sr = config->TimerConfigs[i].cfg.sampleRate;
        s = getHigherSampleRate(sr, s);
    }
#endif

#if IMU_CHANNELS > 0
    for (int i = 0; i < CONFIG_IMU_CHANNELS; i++) {
        sr = config->ImuConfigs[i].cfg.sampleRate;
        s = getHigherSampleRate(sr, s);
    }
#endif

    {
            OBD2Config *obd2_config = &(config->OBD2Configs);
            const size_t enabled_channels = obd2_config->enabledPids;
            bool enabled = obd2_config->enabled;
            for (size_t i = 0; i < enabled_channels && enabled; i++) {
					sr = config->OBD2Configs.pids[i].mapping.channel_cfg.sampleRate;
					s = getHigherSampleRate(sr, s);
            }
    }
    {
            CANChannelConfig *ccc = &(config->can_channel_cfg);
            const size_t enabled_can_channels = ccc->enabled_mappings;
            bool enabled = ccc->enabled;
            for (size_t i = 0; i < enabled_can_channels && enabled; i++) {
					sr = config->can_channel_cfg.can_channels[i].mapping.channel_cfg.sampleRate;
					s = getHigherSampleRate(sr, s);
            }
    }

    GPSConfig *gpsConfig = &(config->GPSConfigs);
#if GPS_HARDWARE_SUPPORT
    sr = gpsConfig->latitude.sampleRate;
    s = getHigherSampleRate(sr, s);

    sr = gpsConfig->longitude.sampleRate;
    s = getHigherSampleRate(sr, s);

    sr = gpsConfig->speed.sampleRate;
    s = getHigherSampleRate(sr, s);

    sr = gpsConfig->altitude.sampleRate;
    s = getHigherSampleRate(sr, s);

    sr = gpsConfig->satellites.sampleRate;
    s = getHigherSampleRate(sr, s);

    sr = gpsConfig->quality.sampleRate;
    s = getHigherSampleRate(sr, s);

    sr = gpsConfig->DOP.sampleRate;
    s = getHigherSampleRate(sr, s);
#endif
    sr = gpsConfig->distance.sampleRate;
    s = getHigherSampleRate(sr, s);

    LapConfig *trackCfg = &(config->LapConfigs);
    sr = trackCfg->lapCountCfg.sampleRate;
    s = getHigherSampleRate(sr, s);

    sr = trackCfg->lapTimeCfg.sampleRate;
    s = getHigherSampleRate(sr, s);

    sr = trackCfg->sectorCfg.sampleRate;
    s = getHigherSampleRate(sr, s);

    sr = trackCfg->sectorTimeCfg.sampleRate;
    s = getHigherSampleRate(sr, s);

    sr = trackCfg->predTimeCfg.sampleRate;
    s = getHigherSampleRate(sr, s);

    sr = trackCfg->elapsed_time_cfg.sampleRate;
    s = getHigherSampleRate(sr, s);

    sr = trackCfg->current_lap_cfg.sampleRate;
    s = getHigherSampleRate(sr, s);

    /* Now check our Virtual Channels */
#if VIRTUAL_CHANNEL_SUPPORT
    sr = get_virtual_channel_high_sample_rate();
    s = getHigherSampleRate(s, sr);
#endif /* VIRTUAL_CHANNEL_SUPPORT */
    return s;
}

size_t get_enabled_channel_count(LoggerConfig *loggerConfig)
{
    size_t channels = 0;

    for (size_t i=0; i < CONFIG_TIME_CHANNELS; i++)
        if (loggerConfig->TimeConfigs[i].cfg.sampleRate != SAMPLE_DISABLED)
            ++channels;
#if IMU_CHANNELS > 0
    for (size_t i=0; i < CONFIG_IMU_CHANNELS; i++)
        if (loggerConfig->ImuConfigs[i].cfg.sampleRate != SAMPLE_DISABLED)
            ++channels;
#endif

#if ANALOG_CHANNELS > 0
    for (size_t i=0; i < CONFIG_ADC_CHANNELS; i++)
        if (loggerConfig->ADCConfigs[i].cfg.sampleRate != SAMPLE_DISABLED)
            ++channels;
#endif

#if TIMER_CHANNELS > 0
    for (size_t i=0; i < CONFIG_TIMER_CHANNELS; i++)
        if (loggerConfig->TimerConfigs[i].cfg.sampleRate != SAMPLE_DISABLED)
            ++channels;
#endif

#if GPIO_CHANNELS > 1
    for (size_t i=0; i < CONFIG_GPIO_CHANNELS; i++)
        if (loggerConfig->GPIOConfigs[i].cfg.sampleRate != SAMPLE_DISABLED)
            ++channels;
#endif

#if PWM_CHANNELS > 0
    for (size_t i=0; i < CONFIG_PWM_CHANNELS; i++)
        if (loggerConfig->PWMConfigs[i].cfg.sampleRate != SAMPLE_DISABLED)
            ++channels;
#endif

    {
            OBD2Config *obd2_config = &(loggerConfig->OBD2Configs);
            const size_t enabled_channels = obd2_config->enabledPids;
            bool enabled = obd2_config->enabled;
            for (size_t i=0; i < enabled_channels && enabled; i++) {
                if (loggerConfig->OBD2Configs.pids[i].mapping.channel_cfg.sampleRate != SAMPLE_DISABLED)
                    ++channels;
            }
    }
    {
            CANChannelConfig *ccc = &(loggerConfig->can_channel_cfg);
            const size_t enabled_can_channels = ccc->enabled_mappings;
            for (size_t i=0; i < enabled_can_channels && ccc->enabled; i++) {
                if (ccc->can_channels[i].mapping.channel_cfg.sampleRate != SAMPLE_DISABLED)
                    ++channels;
            }
    }

    GPSConfig *gpsConfigs = &loggerConfig->GPSConfigs;
#if GPS_HARDWARE_SUPPORT
    if (gpsConfigs->latitude.sampleRate != SAMPLE_DISABLED) channels++;
    if (gpsConfigs->longitude.sampleRate != SAMPLE_DISABLED) channels++;
    if (gpsConfigs->speed.sampleRate != SAMPLE_DISABLED) channels++;
    if (gpsConfigs->altitude.sampleRate != SAMPLE_DISABLED) channels++;
    if (gpsConfigs->satellites.sampleRate != SAMPLE_DISABLED) channels++;
    if (gpsConfigs->quality.sampleRate != SAMPLE_DISABLED) channels++;
    if (gpsConfigs->DOP.sampleRate != SAMPLE_DISABLED) channels++;
#endif
    if (gpsConfigs->distance.sampleRate != SAMPLE_DISABLED) channels++;

    LapConfig *lapConfig = &loggerConfig->LapConfigs;
    if (lapConfig->lapCountCfg.sampleRate != SAMPLE_DISABLED) channels++;
    if (lapConfig->lapTimeCfg.sampleRate != SAMPLE_DISABLED) channels++;
    if (lapConfig->sectorCfg.sampleRate != SAMPLE_DISABLED) channels++;
    if (lapConfig->sectorTimeCfg.sampleRate != SAMPLE_DISABLED) channels++;
    if (lapConfig->predTimeCfg.sampleRate != SAMPLE_DISABLED) channels++;
    if (lapConfig->elapsed_time_cfg.sampleRate != SAMPLE_DISABLED) channels++;
    if (lapConfig->current_lap_cfg.sampleRate != SAMPLE_DISABLED) channels++;

#if VIRTUAL_CHANNEL_SUPPORT
    channels += get_virtual_channel_count();
#endif /* VIRTUAL_CHANNEL_SUPPORT */

    return channels;
}

void reset_logger_config(void)
{
    LoggerConfig *lc = &g_workingLoggerConfig;

    lc->config_size = sizeof(LoggerConfig);

    resetVersionInfo(&lc->RcpVersionInfo);
    resetTimeConfig(lc->TimeConfigs);

#if PWM_CHANNELS > 0
    resetPwmClkFrequency(&lc->PWMClockFrequency);
#endif

#if ANALOG_CHANNELS > 0
    resetAdcConfig(lc->ADCConfigs);
#endif

#if PWM_CHANNELS > 0
    resetPwmConfig(lc->PWMConfigs);
#endif

#if GPIO_CHANNELS > 1
    resetGpioConfig(lc->GPIOConfigs);
#endif

#if TIMER_CHANNELS > 0
    set_default_timer_config(lc->TimerConfigs, CONFIG_TIMER_CHANNELS);
#endif

#if IMU_CHANNELS > 0
    resetImuConfig(lc->ImuConfigs);
#endif

    resetCanConfig(&lc->CanConfig);
    _reset_can_mapping_config(&lc->can_channel_cfg);
    resetOBD2Config(&lc->OBD2Configs);

    logger_config_reset_gps_config(&lc->GPSConfigs);

    resetLapConfig(&lc->LapConfigs);
    resetTrackConfig(&lc->TrackConfigs);
    resetConnectivityConfig(&lc->ConnectivityConfigs);
    reset_logging_config(&lc->logging_cfg);

#if SDCARD_SUPPORT
    auto_logger_reset_config(&lc->auto_logger_cfg);
#endif

#if CAMERA_CONTROL
    camera_control_reset_config(&lc->camera_control_cfg);
#endif
    strcpy(lc->padding_data, "");
}

int flash_default_logger_config(void)
{
    reset_logger_config();
    int result = flashLoggerConfig();
    pr_info_str_msg("flashing default config: ", result == 0 ? "win" : "fail");
    return result;
}

int flashLoggerConfig(void)
{
    return memory_flash_region((void *) &g_savedLoggerConfig,
                               (void *) &g_workingLoggerConfig,
                               sizeof (LoggerConfig));
}


static bool _config_size_changed(void) {
    bool changed = false;

    if (g_savedLoggerConfig.config_size != sizeof(LoggerConfig)) {
        changed = true;
        pr_warning(_LOG_PFX "size of LoggerConfig changed\r\n");
    }
    return changed;
}

static bool checkFlashDefaultConfig(void)
{
        const VersionInfo sv = g_savedLoggerConfig.RcpVersionInfo;
        bool changed = version_check_changed(&sv) || _config_size_changed();
        if (!changed)
                return false;

        flash_default_logger_config();
        return true;
}

static void loadWorkingLoggerConfig(void)
{
    memcpy((void *) &g_workingLoggerConfig,
           (void *) &g_savedLoggerConfig, sizeof(LoggerConfig));

    pr_info_int_msg("sizeof LoggerConfig: ", sizeof(LoggerConfig));
}

void initialize_logger_config()
{
    checkFlashDefaultConfig();
    loadWorkingLoggerConfig();
}

const LoggerConfig * getSavedLoggerConfig()
{
    return (LoggerConfig *) &g_savedLoggerConfig;
}

LoggerConfig * getWorkingLoggerConfig()
{
    return &g_workingLoggerConfig;
}

bool should_sample(const int sample_rate, const int max_rate)
{
        return sample_rate == 0 ? false : sample_rate % max_rate == 0;
}
