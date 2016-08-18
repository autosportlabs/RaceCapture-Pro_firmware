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

#ifndef LOGGERCONFIG_H_
#define LOGGERCONFIG_H_

#include "auto_logger.h"
#include "capabilities.h"
#include "channel_config.h"
#include "cpp_guard.h"
#include "geopoint.h"
#include "serial_device.h"
#include "timer_config.h"
#include "tracks.h"
#include "versionInfo.h"
#include "wifi.h"

#include <stdbool.h>
#include <stddef.h>

CPP_GUARD_BEGIN

/* Internal FLASH Page Size: 256 bytes */
#define FLASH_PAGE_SIZE	((unsigned int) 256)

/**
 * Standard sample rates based on OS timer ticks.  Note that every
 * sample rate must be evenly divisible by all sample rates below
 * it.  Otherwise our sampling will readings.  In example, the 100Hz
 * sample rate is evenly divisible by the 50, 25, 10, 5 and 1 Hz sample
 * rates.
 */
#define SAMPLE_1000Hz                       (TICK_RATE_HZ / 1000)
#define SAMPLE_500Hz                        (TICK_RATE_HZ / 500)
#define SAMPLE_200Hz                        (TICK_RATE_HZ / 200)
#define SAMPLE_100Hz                        (TICK_RATE_HZ / 100)
#define SAMPLE_50Hz                         (TICK_RATE_HZ / 50)
#define SAMPLE_25Hz                         (TICK_RATE_HZ / 25)
#define SAMPLE_10Hz                         (TICK_RATE_HZ / 10)
#define SAMPLE_5Hz                          (TICK_RATE_HZ / 5)
#define SAMPLE_1Hz                          (TICK_RATE_HZ / 1)
#define SAMPLE_DISABLED                     0

#define CONFIG_FEATURE_INSTALLED			1
#define CONFIG_FEATURE_NOT_INSTALLED		0

//Number of channels per feature
#define CONFIG_TIME_CHANNELS                2
#define CONFIG_ADC_CHANNELS					ANALOG_CHANNELS
#define CONFIG_IMU_CHANNELS					IMU_CHANNELS
#define CONFIG_GPIO_CHANNELS				GPIO_CHANNELS
#define CONFIG_PWM_CHANNELS					PWM_CHANNELS
#define CONFIG_TIMER_CHANNELS				TIMER_CHANNELS
#define CONFIG_CAN_CHANNELS                 CAN_CHANNELS

#define SLOW_LINK_MAX_TELEMETRY_SAMPLE_RATE SAMPLE_10Hz
#define FAST_LINK_MAX_TELEMETRY_SAMPLE_RATE SAMPLE_50Hz

#define DEFAULT_GPS_POSITION_PRECISION 		6
#define DEFAULT_GPS_RADIUS_PRECISION 		5

enum TimeType {
    TimeType_Uptime,
    TimeType_UtcMillis,
};

struct TimeConfig {
    ChannelConfig cfg;
    enum TimeType tt;
};

#define EMPTY_CHANNEL_CONFIG {"","", 0.0f, 0.0f, SAMPLE_DISABLED, 0}

/*
 * Default to lowest active sample rate.  This will change in code later.
 * NOTE: This should never be below SAMPLE_1Hz, else you could potentially
 *       starve the telemtry task if a user somehow disables all channels.
 */
#define DEFAULT_UPTIME_CONFIG {"Interval", "ms", 0, 0, SAMPLE_1Hz, 0, ALWAYS_SAMPLED}
#define DEFAULT_UTC_MILLIS_CONFIG {"Utc", "ms", 0, 0, SAMPLE_1Hz, 0, ALWAYS_SAMPLED}

#define DEFAULT_UPTIME_TIME_CONFIG {DEFAULT_UPTIME_CONFIG, TimeType_Uptime}
#define DEFAULT_UTC_MILLIS_TIME_CONFIG {DEFAULT_UTC_MILLIS_CONFIG, TimeType_UtcMillis}

/* ANALOG SENSOR SUPPORT */
#define ANALOG_SCALING_BINS                 5
#define SCALING_MODE_RAW                    0
#define SCALING_MODE_LINEAR                 1
#define SCALING_MODE_MAP                    2
#define DEFAULT_SCALING_MODE                SCALING_MODE_LINEAR
#define LINEAR_SCALING_PRECISION            7
#define FILTER_ALPHA_PRECISION              2
#define SCALING_MAP_BIN_PRECISION           2
#define DEFAULT_ANALOG_SCALING_PRECISION    2
#define DEFAULT_VOLTAGE_SCALING_PRECISION   2

typedef struct _ScalingMap {
    float rawValues[ANALOG_SCALING_BINS];
    float scaledValues[ANALOG_SCALING_BINS];
} ScalingMap;

typedef struct _ADCConfig {
    ChannelConfig cfg;
    float linearScaling;
    float linearOffset;
    float filterAlpha;
    float calibration;
    unsigned char scalingMode;
    ScalingMap scalingMap;
} ADCConfig;

#define DEFAULT_LINEAR_SCALING (1)
#define DEFAULT_LINEAR_OFFSET (0)
#define DEFAULT_FILTER_ALPHA (1.0f)
#define DEFAULT_CALIBRATION (1.0f)
#define DEFAULT_SCALING_MAP {{0,1.25,2.5,3.75,5.0},{0,1.25,2.5,3.75,5.0}}

#define DEFAULT_ADC_CHANNEL_CONFIG {"", "Volts", 0, 5, SAMPLE_DISABLED, 2, 0}
// Define channel config for battery
#define DEFAULT_ADC_BATTERY_CONFIG {"Battery", "Volts", 0, 20, SAMPLE_1Hz, 2, 0}

#define BATTERY_ADC_CONFIG                      \
   {                                            \
      DEFAULT_ADC_BATTERY_CONFIG,               \
         DEFAULT_LINEAR_SCALING,                \
         DEFAULT_LINEAR_OFFSET,                 \
         DEFAULT_FILTER_ALPHA,                  \
         DEFAULT_CALIBRATION,					\
         DEFAULT_SCALING_MODE,                  \
         DEFAULT_SCALING_MAP                    \
         }

#define DEFAULT_ADC_CONFIG                      \
   {                                            \
      DEFAULT_ADC_CHANNEL_CONFIG,               \
         DEFAULT_LINEAR_SCALING,                \
         DEFAULT_LINEAR_OFFSET,                 \
         DEFAULT_FILTER_ALPHA,                  \
         DEFAULT_CALIBRATION,					\
         DEFAULT_SCALING_MODE,                  \
         DEFAULT_SCALING_MAP                    \
         }

typedef struct _GPIOConfig {
    ChannelConfig cfg;
    unsigned char mode;
} GPIOConfig;

#define CONFIG_GPIO_IN  					0
#define CONFIG_GPIO_OUT  					1

#define DEFAULT_GPIO_MODE CONFIG_GPIO_IN
#define DEFAULT_GPIO_CHANNEL_CONFIG {"", "", 0, 1, SAMPLE_DISABLED, 1, 0}
#define DEFAULT_GPIO_CONFIG {DEFAULT_GPIO_CHANNEL_CONFIG, CONFIG_GPIO_IN}

/*
 * FIXME: These two enum types have to live in this file instead of
 * something more proper like imu.h.  Otherwise we will get a chicken
 * and egg problem in our headers.
 */
enum imu_mode {
        IMU_MODE_DISABLED = 0,
        IMU_MODE_NORMAL = 1,
        IMU_MODE_INVERTED = 2,
};

enum imu_channel {
        IMU_CHANNEL_X = 0,
        IMU_CHANNEL_Y = 1,
        IMU_CHANNEL_Z = 2,
        IMU_CHANNEL_YAW = 3,
        IMU_CHANNEL_PITCH = 4,
        IMU_CHANNEL_ROLL = 5,
};

typedef struct _ImuConfig {
    ChannelConfig cfg;
    enum imu_mode mode;
    enum imu_channel physicalChannel;
    signed short zeroValue;
    float filterAlpha;
} ImuConfig;

/*
 * On InvenSense IMU 9150/9250 raw values are returned in 2's
 * compliment.  Hence they are already formed the way we want,
 * so the zeroValue is actually 0.  Just have to divide them
 * by their units per element rate (defined by HAL).
 */
#define DEFAULT_ACCEL_ZERO	0
#define DEFAULT_GYRO_ZERO	0

#define IMU_ACCEL_CH_CONFIG(name) {name, "G", -3, 3, SAMPLE_25Hz, 2, 0}
#define IMU_GYRO_CH_CONFIG(name) {name, "Deg/Sec", -120, 120, SAMPLE_25Hz, 0, 0}
#define IMU_ACCEL_CONFIG(name, mode, chan) {    \
                IMU_ACCEL_CH_CONFIG(name),      \
                        mode,                   \
                        chan,                   \
                        DEFAULT_ACCEL_ZERO,     \
                        0.1F                    \
                        }

#define IMU_GYRO_CONFIG(name, mode, chan) {     \
                IMU_GYRO_CH_CONFIG(name),       \
                        mode,                   \
                        chan,                   \
                        DEFAULT_GYRO_ZERO,      \
                        0.1F                    \
                        }

#define IMU_CONFIG_DEFAULTS {                                                 \
                IMU_ACCEL_CONFIG("AccelX", IMU_MODE_NORMAL, IMU_CHANNEL_X), \
                IMU_ACCEL_CONFIG("AccelY", IMU_MODE_NORMAL, IMU_CHANNEL_Y), \
                IMU_ACCEL_CONFIG("AccelZ", IMU_MODE_NORMAL, IMU_CHANNEL_Z),   \
                IMU_GYRO_CONFIG("Yaw", IMU_MODE_NORMAL, IMU_CHANNEL_YAW),   \
                IMU_GYRO_CONFIG("Pitch", IMU_MODE_NORMAL, IMU_CHANNEL_PITCH), \
                IMU_GYRO_CONFIG("Roll", IMU_MODE_NORMAL, IMU_CHANNEL_ROLL),   \
                }

typedef struct _PWMConfig {
    ChannelConfig cfg;
    unsigned char outputMode;
    unsigned char loggingMode;
    unsigned char startupDutyCycle;
    unsigned short startupPeriod;
} PWMConfig;

/// PWM frequency in Hz.
#define MAX_PWM_CLOCK_FREQUENCY             65000
#define MIN_PWM_CLOCK_FREQUENCY	 			10
#define DEFAULT_PWM_CLOCK_FREQUENCY			20000

/// Maximum duty cycle value.
#define MAX_PWM_DUTY_CYCLE              	100
#define MIN_PWM_DUTY_CYCLE          		1

//TODO: FIX PERIOD/FREQ MIN-MAX
#define MIN_PWM_PERIOD						1
#define MAX_PWM_PERIOD						2000

#define MODE_PWM_ANALOG  					0
#define	MODE_PWM_FREQUENCY  				1

#define MODE_LOGGING_PWM_PERIOD				0
#define MODE_LOGGING_PWM_DUTY				1
#define	MODE_LOGGING_PWM_VOLTS				2

#define PWM_VOLTAGE_SCALING			0.05

#define DEFAULT_PWM_DUTY_CYCLE (50)
#define DEFAULT_PWM_PERIOD (100)

#define DEFAULT_PWM_CHANNEL_CONFIG {"PWM1", "", 0, 100, SAMPLE_DISABLED, 0, 0}
#define DEFAULT_PWM_CONFIG {DEFAULT_PWM_CHANNEL_CONFIG, MODE_PWM_FREQUENCY, MODE_LOGGING_PWM_DUTY, DEFAULT_PWM_DUTY_CYCLE, DEFAULT_PWM_PERIOD}


#define OBD2_CHANNELS 20

typedef struct _PidConfig {
    ChannelConfig cfg;
    unsigned short pid;
} PidConfig;


typedef struct _OBD2Config {
    unsigned char enabled;
    unsigned short enabledPids;
    PidConfig pids[OBD2_CHANNELS];
} OBD2Config;

#define DEFAULT_ENABLED_PIDS 1
#define DEFAULT_OBD2_CONFIG \
{ \
	CONFIG_FEATURE_NOT_INSTALLED, \
	DEFAULT_ENABLED_PIDS, \
	{ \
		{{"RPM", "", 0, 10000, SAMPLE_10Hz, 0, 0}, 12}, \
		{EMPTY_CHANNEL_CONFIG, 0}, \
		{EMPTY_CHANNEL_CONFIG, 0}, \
		{EMPTY_CHANNEL_CONFIG, 0}, \
		{EMPTY_CHANNEL_CONFIG, 0}, \
		{EMPTY_CHANNEL_CONFIG, 0}, \
		{EMPTY_CHANNEL_CONFIG, 0}, \
		{EMPTY_CHANNEL_CONFIG, 0}, \
		{EMPTY_CHANNEL_CONFIG, 0}, \
		{EMPTY_CHANNEL_CONFIG, 0}, \
		{EMPTY_CHANNEL_CONFIG, 0}, \
		{EMPTY_CHANNEL_CONFIG, 0}, \
		{EMPTY_CHANNEL_CONFIG, 0}, \
		{EMPTY_CHANNEL_CONFIG, 0}, \
		{EMPTY_CHANNEL_CONFIG, 0}, \
		{EMPTY_CHANNEL_CONFIG, 0}, \
		{EMPTY_CHANNEL_CONFIG, 0}, \
		{EMPTY_CHANNEL_CONFIG, 0}, \
		{EMPTY_CHANNEL_CONFIG, 0}, \
		{EMPTY_CHANNEL_CONFIG, 0}  \
	} \
}

typedef struct _CANConfig {
    unsigned char enabled;
    int baud[CONFIG_CAN_CHANNELS];
} CANConfig;

#define DEFAULT_CAN_BAUD_RATE 500000

typedef struct _GPSConfig {
    ChannelConfig latitude;
    ChannelConfig longitude;
    ChannelConfig speed;
    ChannelConfig distance;
    ChannelConfig altitude;
    ChannelConfig satellites;
    ChannelConfig quality;
    ChannelConfig DOP;
} GPSConfig;



#define DEFAULT_GPS_SAMPLE_RATE SAMPLE_10Hz

#define DEFAULT_GPS_LATITUDE_CONFIG {"Latitude", "Degrees", -180, 180, DEFAULT_GPS_SAMPLE_RATE, 6, 0}
#define DEFAULT_GPS_LONGITUDE_CONFIG {"Longitude", "Degrees", -180, 180, DEFAULT_GPS_SAMPLE_RATE, 6, 0}
#define DEFAULT_GPS_SPEED_CONFIG {"Speed", "", 0, 150, DEFAULT_GPS_SAMPLE_RATE, 2, 0}
#define DEFAULT_GPS_DISTANCE_CONFIG {"Distance", "", 0, 0, DEFAULT_GPS_SAMPLE_RATE, 3, 0}
#define DEFAULT_GPS_ALTITUDE_CONFIG {"Altitude", "", 0, 4000, DEFAULT_GPS_SAMPLE_RATE, 1, 0}
#define DEFAULT_GPS_SATELLITE_CONFIG {"GPSSats", "", 0, 20, DEFAULT_GPS_SAMPLE_RATE, 0, 0}
#define DEFAULT_GPS_QUALITY_CONFIG {"GPSQual", "", 0, 5, DEFAULT_GPS_SAMPLE_RATE, 0, 0}
#define DEFAULT_GPS_DOP_CONFIG {"GPSDOP", "", 0, 20, DEFAULT_GPS_SAMPLE_RATE, 1, 0}

#define DEFAULT_GPS_CONFIG {                   \
		DEFAULT_GPS_LATITUDE_CONFIG,           \
		DEFAULT_GPS_LONGITUDE_CONFIG,          \
		DEFAULT_GPS_SPEED_CONFIG,              \
		DEFAULT_GPS_DISTANCE_CONFIG,           \
		DEFAULT_GPS_ALTITUDE_CONFIG,           \
		DEFAULT_GPS_SATELLITE_CONFIG,          \
		DEFAULT_GPS_QUALITY_CONFIG,            \
		DEFAULT_GPS_DOP_CONFIG                 \
         }

typedef struct _LapConfig {
    ChannelConfig lapCountCfg;
    ChannelConfig lapTimeCfg;
    ChannelConfig sectorCfg;
    ChannelConfig sectorTimeCfg;
    ChannelConfig predTimeCfg;
    ChannelConfig elapsed_time_cfg;
    ChannelConfig current_lap_cfg;
} LapConfig;

#define DEFAULT_LAPSTATS_SAMPLE_RATE SAMPLE_10Hz

#define DEFAULT_LAP_COUNT_CONFIG {"LapCount", "", 0, 0, DEFAULT_LAPSTATS_SAMPLE_RATE, 0, 0}
#define DEFAULT_LAP_TIME_CONFIG {"LapTime", "Min", 0, 0, DEFAULT_LAPSTATS_SAMPLE_RATE, 4, 0}
#define DEFAULT_SECTOR_CONFIG {"Sector", "", 0, 0, DEFAULT_LAPSTATS_SAMPLE_RATE, 0, 0}
#define DEFAULT_SECTOR_TIME_CONFIG {"SectorTime", "Min", 0, 0, DEFAULT_LAPSTATS_SAMPLE_RATE, 4, 0}
#define DEFAULT_PRED_TIME_CONFIG {"PredTime", "Min", 0, 0, SAMPLE_5Hz, 4, 0}
#define DEFAULT_ELAPSED_LAP_TIME_CONFIG {"ElapsedTime", "Min", 0, 0, DEFAULT_LAPSTATS_SAMPLE_RATE, 4, 0}
#define DEFAULT_CURRENT_LAP_CONFIG {"CurrentLap", "", 0, 0, DEFAULT_LAPSTATS_SAMPLE_RATE, 0, 0}

#define DEFAULT_LAP_CONFIG {                                    \
                DEFAULT_LAP_COUNT_CONFIG,                       \
                        DEFAULT_LAP_TIME_CONFIG,                \
                        DEFAULT_SECTOR_CONFIG,                  \
                        DEFAULT_SECTOR_TIME_CONFIG,             \
                        DEFAULT_PRED_TIME_CONFIG,               \
                        DEFAULT_ELAPSED_LAP_TIME_CONFIG,        \
                        DEFAULT_CURRENT_LAP_CONFIG              \
                        }

typedef struct _TrackConfig {
    float radius;
    unsigned char auto_detect;
    Track track;
} TrackConfig;

#define BT_DEVICE_NAME_LENGTH 21
#define BT_PASSCODE_LENGTH 5
#define DEFAULT_BT_ENABLED 1

typedef struct _BluetoothConfig {
        unsigned char btEnabled;
        char new_name [BT_DEVICE_NAME_LENGTH];
        char new_pin [BT_PASSCODE_LENGTH];
} BluetoothConfig;

#define CELL_APN_HOST_LENGTH 30
#define CELL_APN_USER_LENGTH 30
#define CELL_APN_PASS_LENGTH 30
#define DNS_ADDR_LEN 16
#define DEFAULT_APN_HOST "epc.tmobile.com"
#define DEFAULT_APN_USER ""
#define DEFAULT_APN_PASS ""
#define DEFAULT_CELL_ENABLED 0
#define DEFAULT_DNS1	"8.8.8.8"
#define DEFAULT_DNS2	"8.8.4.4"

typedef struct _CellularConfig {
        unsigned char cellEnabled;
        char apnHost [CELL_APN_HOST_LENGTH + 1];
        char apnUser [CELL_APN_USER_LENGTH + 1];
        char apnPass [CELL_APN_PASS_LENGTH + 1];
        char dns1[DNS_ADDR_LEN];
        char dns2[DNS_ADDR_LEN];
} CellularConfig;


#define DEVICE_ID_LENGTH 35
#define TELEMETRY_SERVER_HOST_LENGTH 95

#define DEFAULT_DEVICE_ID ""
#define DEFAULT_TELEMETRY_SERVER_HOST "telemetry.race-capture.com"
#define DEFAULT_TELEMETRY_SERVER_PORT 8080

#define BACKGROUND_STREAMING_ENABLED				1
#define BACKGROUND_STREAMING_DISABLED				0

typedef struct _TelemetryConfig {
        unsigned char backgroundStreaming;
        char telemetryDeviceId[DEVICE_ID_LENGTH + 1];
        char telemetryServerHost[TELEMETRY_SERVER_HOST_LENGTH + 1];
        int telemetry_port;
} TelemetryConfig;

typedef struct _ConnectivityConfig {
        BluetoothConfig bluetoothConfig;
        CellularConfig cellularConfig;
        TelemetryConfig telemetryConfig;
        struct wifi_cfg wifi;
} ConnectivityConfig;

#define SD_LOGGING_MODE_DISABLED					0
#define SD_LOGGING_MODE_CSV							1

/**
 * Configurations specific to our logging infrastructure.
 */
struct logging_config {
        enum serial_log_type serial[__SERIAL_COUNT];
};

typedef struct _LoggerConfig {
    VersionInfo RcpVersionInfo;

    //PWM/Analog out configurations
    unsigned short PWMClockFrequency;

    // Time Config
    struct TimeConfig TimeConfigs[CONFIG_TIME_CHANNELS];

    //ADC Calibrations
#if ANALOG_CHANNELS > 0
    ADCConfig ADCConfigs[CONFIG_ADC_CHANNELS];
#endif

#if PWM_CHANNELS > 0
    //PWM configuration
    PWMConfig PWMConfigs[CONFIG_PWM_CHANNELS];
#endif

#if GPIO_CHANNELS > 0
    //GPIO configurations
    GPIOConfig GPIOConfigs[CONFIG_GPIO_CHANNELS];
#endif

#if TIMER_CHANNELS > 0
    //Timer Configurations
    TimerConfig TimerConfigs[CONFIG_TIMER_CHANNELS];
#endif

#if IMU_CHANNELS > 0
    //IMU Configurations
    ImuConfig ImuConfigs[CONFIG_IMU_CHANNELS];
#endif

    //CAN Configuration
    CANConfig CanConfig;

    //OBD2 Config
    OBD2Config OBD2Configs;

    //GPS Configuration
    GPSConfig GPSConfigs;

    //Lap Configuration
    LapConfig LapConfigs;

    //Track configuration
    TrackConfig TrackConfigs;

    //Connectivity Configuration
    ConnectivityConfig ConnectivityConfigs;

        struct logging_config logging_cfg;
        struct auto_logger_config auto_logger_cfg;
    //Padding data to accommodate flash routine
    char padding_data[FLASH_PAGE_SIZE];
} LoggerConfig;


void initialize_logger_config();

const LoggerConfig * getSavedLoggerConfig();
LoggerConfig * getWorkingLoggerConfig();

int getConnectivitySampleRateLimit();
int encodeSampleRate(int sampleRate);
int decodeSampleRate(int sampleRateCode);

unsigned char filterBgStreamingMode(unsigned char mode);
unsigned char filterSdLoggingMode(unsigned char mode);

PWMConfig * getPwmConfigChannel(int channel);
char filterPwmOutputMode(int config);
char filterPwmLoggingMode(int config);
unsigned short filterPwmDutyCycle(int dutyCycle);
unsigned short filterPwmPeriod(int period);
uint16_t filterPwmClockFrequency(uint16_t frequency);

unsigned short filterTimerDivider(unsigned short divider);
char filterTimerMode(int config);

ADCConfig * getADCConfigChannel(int channel);
unsigned char filterAnalogScalingMode(unsigned char mode);

GPIOConfig * getGPIOConfigChannel(int channel);
char filterGpioMode(int config);

ImuConfig * getImuConfigChannel(int channel);
int filterImuMode(int mode);
int filterImuChannel(int channel);

unsigned int getHighestSampleRate(LoggerConfig *config);
size_t get_enabled_channel_count(LoggerConfig *loggerConfig);

bool isHigherSampleRate(const int contender, const int champ);
bool should_sample(const int sample_rate, const int max_rate);
int getHigherSampleRate(const int a, const int b);

int flashLoggerConfig(void);
int flash_default_logger_config(void);

void logger_config_reset_gps_config(GPSConfig *cfg);

CPP_GUARD_END

#endif /*LOGGERCONFIG_H_*/
