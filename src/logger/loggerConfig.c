/*
 * Race Capture Firmware
 *
 * Copyright (C) 2015 Autosport Labs
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


#include "cpu.h"
#include "loggerConfig.h"
#include "memory.h"
#include "mod_string.h"
#include "modp_numtoa.h"
#include "printk.h"
#include "virtual_channel.h"

#include <stdbool.h>

#ifndef RCP_TESTING
#include "memory.h"
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

static void resetPwmClkFrequency(unsigned short *pwmClkFreq)
{
    *pwmClkFreq = DEFAULT_PWM_CLOCK_FREQUENCY;
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
    cfg[7] = (ADCConfig) BATTERY_ADC_CONFIG;
}

static void resetPwmConfig(PWMConfig cfg[])
{
    for (size_t i = 0; i < CONFIG_PWM_CHANNELS; ++i) {
        PWMConfig *c = cfg + i;
        *c = (PWMConfig) DEFAULT_PWM_CONFIG;
        sPrintStrInt(c->cfg.label, "PWM", i + 1);
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

static void resetTimerConfig(TimerConfig cfg[])
{
    for (size_t i = 0; i < CONFIG_TIMER_CHANNELS; ++i) {
        TimerConfig *c = cfg + i;
        *c = (TimerConfig) DEFAULT_FREQUENCY_CONFIG;
        sPrintStrInt(c->cfg.label, "RPM", i + 1);
    }

    // Make Channel 1 the default RPM config.
    cfg[0].cfg = (ChannelConfig) DEFAULT_RPM_CHANNEL_CONFIG;
}

static void resetImuConfig(ImuConfig cfg[])
{
    const char *imu_names[] = {"AccelX", "AccelY", "AccelZ", "Yaw", "Pitch", "Roll"};

    for (size_t i = 0; i < 6; ++i) {
        ImuConfig *c = cfg + i;
        if (i < IMU_CHANNEL_YAW) {
            *c = (ImuConfig)DEFAULT_IMU_CONFIG;
        } else {
            *c = (ImuConfig)DEFAULT_GYRO_CONFIG;
        }
        strcpy(c->cfg.label, imu_names[i]);

        // Channels go X, Y, Z.  Works perfectly with our counter.
        c->physicalChannel = i;
    }
}

static void resetCanConfig(CANConfig *cfg)
{
    cfg->enabled = CONFIG_FEATURE_INSTALLED;
    for (size_t i = 0; i < CONFIG_CAN_CHANNELS; i++) {
        cfg->baud[i] = DEFAULT_CAN_BAUD_RATE;
    }
}

static void resetOBD2Config(OBD2Config *cfg)
{
    memset(cfg, 0, sizeof(OBD2Config));

    for (int i = 0; i < OBD2_CHANNELS; ++i) {
        PidConfig *c = &cfg->pids[i];
        memset(c, 0, sizeof(PidConfig));
        sPrintStrInt(c->cfg.label, "OBD2 Pid ", i + 1);
    }
}

static void resetGPSConfig(GPSConfig *cfg)
{
    *cfg = (GPSConfig) DEFAULT_GPS_CONFIG;
}

static void resetLapConfig(LapConfig *cfg)
{
    *cfg = (LapConfig) DEFAULT_LAP_CONFIG;
}

static void resetTrackConfig(TrackConfig *cfg)
{
    memset(cfg, 0, sizeof(TrackConfig));
    cfg->radius = DEFAULT_TRACK_TARGET_RADIUS;
    cfg->auto_detect = DEFAULT_TRACK_AUTO_DETECT;
}

static void resetBluetoothConfig(BluetoothConfig *cfg)
{
        *cfg = (BluetoothConfig) {
                .btEnabled = DEFAULT_BT_ENABLED,
                .baudRate = DEFAULT_BT_BAUD,
        };
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
    strncpy(cfg->telemetryServerHost, DEFAULT_TELEMETRY_SERVER_HOST,
            sizeof(cfg->telemetryServerHost));
    cfg->telemetry_port = DEFAULT_TELEMETRY_SERVER_PORT;
}

static void resetConnectivityConfig(ConnectivityConfig *cfg)
{
    resetBluetoothConfig(&cfg->bluetoothConfig);
    resetCellularConfig(&cfg->cellularConfig);
    resetTelemetryConfig(&cfg->telemetryConfig);
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

int flash_default_logger_config(void)
{
    LoggerConfig *lc = &g_workingLoggerConfig;

    resetVersionInfo(&lc->RcpVersionInfo);
    resetPwmClkFrequency(&lc->PWMClockFrequency);
    resetTimeConfig(lc->TimeConfigs);
    resetAdcConfig(lc->ADCConfigs);
    resetPwmConfig(lc->PWMConfigs);
    resetGpioConfig(lc->GPIOConfigs);
    resetTimerConfig(lc->TimerConfigs);
    resetImuConfig(lc->ImuConfigs);
    resetCanConfig(&lc->CanConfig);
    resetOBD2Config(&lc->OBD2Configs);
    resetGPSConfig(&lc->GPSConfigs);
    resetLapConfig(&lc->LapConfigs);
    resetTrackConfig(&lc->TrackConfigs);
    resetConnectivityConfig(&lc->ConnectivityConfigs);
    strcpy(lc->padding_data, "");

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

static bool checkFlashDefaultConfig(void)
{
    bool changed = versionChanged(&g_savedLoggerConfig.RcpVersionInfo);
    if (changed) {
        pr_info("major/minor version changed\r\n");
        flash_default_logger_config();
        return true;
    } else {
        return false;
    }
}

static void loadWorkingLoggerConfig(void)
{
    memcpy((void *) &g_workingLoggerConfig,
           (void *) &g_savedLoggerConfig, sizeof(LoggerConfig));
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

unsigned char filterBgStreamingMode(unsigned char mode)
{
    return mode == 0 ? 0 : 1;
}

unsigned char filterSdLoggingMode(unsigned char mode)
{
    switch (mode) {
    case SD_LOGGING_MODE_CSV:
        return SD_LOGGING_MODE_CSV;
    default:
    case SD_LOGGING_MODE_DISABLED:
        return SD_LOGGING_MODE_DISABLED;
    }
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

unsigned char filterPulsePerRevolution(unsigned char pulsePerRev)
{
    return pulsePerRev == 0 ? 1 : pulsePerRev;
}

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

int filterImuChannel(int channel)
{
    return (channel < CONFIG_IMU_CHANNELS ? channel : CONFIG_IMU_CHANNELS - 1);
}

int filterImuRawValue(int imuRawValue)
{
    if (imuRawValue > MAX_IMU_RAW) {
        imuRawValue = MAX_IMU_RAW;
    } else if (imuRawValue < MIN_IMU_RAW) {
        imuRawValue = MIN_IMU_RAW;
    }
    return imuRawValue;
}

int filterImuMode(int mode)
{
    switch (mode) {
    case MODE_IMU_DISABLED:
        return MODE_IMU_DISABLED;
    case MODE_IMU_INVERTED:
        return MODE_IMU_INVERTED;
    default:
    case MODE_IMU_NORMAL:
        return MODE_IMU_NORMAL;
    }
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

PWMConfig * getPwmConfigChannel(int channel)
{
    PWMConfig * c = NULL;
    if (channel >= 0 && channel < CONFIG_PWM_CHANNELS) {
        c = &(getWorkingLoggerConfig()->PWMConfigs[channel]);
    }
    return c;
}

TimerConfig * getTimerConfigChannel(int channel)
{
    TimerConfig * c = NULL;
    if (channel >=0 && channel < CONFIG_TIMER_CHANNELS) {
        c = &(getWorkingLoggerConfig()->TimerConfigs[channel]);
    }
    return c;
}

ADCConfig * getADCConfigChannel(int channel)
{
    ADCConfig *c = NULL;
    if (channel >=0 && channel < CONFIG_ADC_CHANNELS) {
        c = &(getWorkingLoggerConfig()->ADCConfigs[channel]);
    }
    return c;
}

GPIOConfig * getGPIOConfigChannel(int channel)
{
    GPIOConfig *c = NULL;
    if (channel >=0 && channel < CONFIG_GPIO_CHANNELS) {
        c = &(getWorkingLoggerConfig()->GPIOConfigs[channel]);
    }
    return c;
}

ImuConfig * getImuConfigChannel(int channel)
{
    ImuConfig * c = NULL;
    if (channel >= 0 && channel < CONFIG_IMU_CHANNELS) {
        c = &(getWorkingLoggerConfig()->ImuConfigs[channel]);
    }
    return c;
}

unsigned int getHighestSampleRate(LoggerConfig *config)
{
    int s = SAMPLE_DISABLED;
    int sr;

    /*
     * Bypass Interval and Utc here since they will always be logging
     * at the highest rate based on the results of this very method
     */

    for (int i = 0; i < CONFIG_ADC_CHANNELS; i++) {
        sr = config->ADCConfigs[i].cfg.sampleRate;
        s = getHigherSampleRate(sr, s);
    }

    for (int i = 0; i < CONFIG_PWM_CHANNELS; i++) {
        sr = config->PWMConfigs[i].cfg.sampleRate;
        s = getHigherSampleRate(sr, s);
    }

    for (int i = 0; i < CONFIG_GPIO_CHANNELS; i++) {
        sr = config->GPIOConfigs[i].cfg.sampleRate;
        s = getHigherSampleRate(sr, s);
    }

    for (int i = 0; i < CONFIG_TIMER_CHANNELS; i++) {
        sr = config->TimerConfigs[i].cfg.sampleRate;
        s = getHigherSampleRate(sr, s);
    }

    for (int i = 0; i < CONFIG_IMU_CHANNELS; i++) {
        sr = config->ImuConfigs[i].cfg.sampleRate;
        s = getHigherSampleRate(sr, s);
    }


    GPSConfig *gpsConfig = &(config->GPSConfigs);
    sr = gpsConfig->latitude.sampleRate;
    s = getHigherSampleRate(sr, s);

    sr = gpsConfig->longitude.sampleRate;
    s = getHigherSampleRate(sr, s);

    sr = gpsConfig->speed.sampleRate;
    s = getHigherSampleRate(sr, s);

    sr = gpsConfig->distance.sampleRate;
    s = getHigherSampleRate(sr, s);

    sr = gpsConfig->altitude.sampleRate;
    s = getHigherSampleRate(sr, s);

    sr = gpsConfig->satellites.sampleRate;
    s = getHigherSampleRate(sr, s);

    sr = gpsConfig->quality.sampleRate;
    s = getHigherSampleRate(sr, s);

    sr = gpsConfig->DOP.sampleRate;
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

    return s;
}

size_t get_enabled_channel_count(LoggerConfig *loggerConfig)
{
    size_t channels = 0;

    for (size_t i=0; i < CONFIG_TIME_CHANNELS; i++)
        if (loggerConfig->TimeConfigs[i].cfg.sampleRate != SAMPLE_DISABLED)
            ++channels;

    for (size_t i=0; i < CONFIG_IMU_CHANNELS; i++)
        if (loggerConfig->ImuConfigs[i].cfg.sampleRate != SAMPLE_DISABLED)
            ++channels;

    for (size_t i=0; i < CONFIG_ADC_CHANNELS; i++)
        if (loggerConfig->ADCConfigs[i].cfg.sampleRate != SAMPLE_DISABLED)
            ++channels;

    for (size_t i=0; i < CONFIG_TIMER_CHANNELS; i++)
        if (loggerConfig->TimerConfigs[i].cfg.sampleRate != SAMPLE_DISABLED)
            ++channels;

    for (size_t i=0; i < CONFIG_GPIO_CHANNELS; i++)
        if (loggerConfig->GPIOConfigs[i].cfg.sampleRate != SAMPLE_DISABLED)
            ++channels;

    for (size_t i=0; i < CONFIG_PWM_CHANNELS; i++)
        if (loggerConfig->PWMConfigs[i].cfg.sampleRate != SAMPLE_DISABLED)
            ++channels;

    const size_t enabled_obd2_pids = loggerConfig->OBD2Configs.enabledPids;
    const unsigned char enabled = loggerConfig->OBD2Configs.enabled;
    for (size_t i=0; i < enabled_obd2_pids && enabled; i++) {
        if (loggerConfig->OBD2Configs.pids[i].cfg.sampleRate != SAMPLE_DISABLED)
            ++channels;
    }

    GPSConfig *gpsConfigs = &loggerConfig->GPSConfigs;
    if (gpsConfigs->latitude.sampleRate != SAMPLE_DISABLED) channels++;
    if (gpsConfigs->longitude.sampleRate != SAMPLE_DISABLED) channels++;
    if (gpsConfigs->speed.sampleRate != SAMPLE_DISABLED) channels++;
    if (gpsConfigs->distance.sampleRate != SAMPLE_DISABLED) channels++;
    if (gpsConfigs->altitude.sampleRate != SAMPLE_DISABLED) channels++;
    if (gpsConfigs->satellites.sampleRate != SAMPLE_DISABLED) channels++;
    if (gpsConfigs->quality.sampleRate != SAMPLE_DISABLED) channels++;
    if (gpsConfigs->DOP.sampleRate != SAMPLE_DISABLED) channels++;

    LapConfig *lapConfig = &loggerConfig->LapConfigs;
    if (lapConfig->lapCountCfg.sampleRate != SAMPLE_DISABLED) channels++;
    if (lapConfig->lapTimeCfg.sampleRate != SAMPLE_DISABLED) channels++;
    if (lapConfig->sectorCfg.sampleRate != SAMPLE_DISABLED) channels++;
    if (lapConfig->sectorTimeCfg.sampleRate != SAMPLE_DISABLED) channels++;
    if (lapConfig->predTimeCfg.sampleRate != SAMPLE_DISABLED) channels++;
    if (lapConfig->elapsed_time_cfg.sampleRate != SAMPLE_DISABLED) channels++;
    if (lapConfig->current_lap_cfg.sampleRate != SAMPLE_DISABLED) channels++;

    channels += get_virtual_channel_count();
    return channels;
}
