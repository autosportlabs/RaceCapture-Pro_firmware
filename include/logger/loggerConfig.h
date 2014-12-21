#ifndef LOGGERCONFIG_H_
#define LOGGERCONFIG_H_

#include <stddef.h>
#include "geopoint.h"
#include "tracks.h"
#include "capabilities.h"

#define FLASH_PAGE_SIZE						((unsigned int) 256) // Internal FLASH Page Size: 256 bytes

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
#define FAST_LINK_MAX_TELEMETRY_SAMPLE_RATE SAMPLE_10Hz


//standard sample rates based on OS timer ticks
#define SAMPLE_1000Hz						(TICK_RATE_HZ / 1000)
#define SAMPLE_500Hz						(TICK_RATE_HZ / 500)
#define SAMPLE_200Hz						(TICK_RATE_HZ / 200)
#define SAMPLE_100Hz 						(TICK_RATE_HZ / 100)
#define SAMPLE_50Hz 						(TICK_RATE_HZ / 50)
#define SAMPLE_25Hz 						(TICK_RATE_HZ / 25)
#define SAMPLE_10Hz 						(TICK_RATE_HZ / 10)
#define SAMPLE_5Hz 							(TICK_RATE_HZ / 5)
#define SAMPLE_1Hz 							(TICK_RATE_HZ / 1)
#define SAMPLE_DISABLED 					0

#define ANALOG_SCALING_BINS					5

#define SCALING_MODE_RAW					0
#define SCALING_MODE_LINEAR					1
#define SCALING_MODE_MAP					2
#define DEFAULT_SCALING_MODE				SCALING_MODE_LINEAR
#define LINEAR_SCALING_PRECISION			7
#define FILTER_ALPHA_PRECISION				2
#define SCALING_MAP_BIN_PRECISION			2

#define DEFAULT_GPS_POSITION_PRECISION 		6
#define DEFAULT_GPS_RADIUS_PRECISION 		5
#define DEFAULT_VOLTAGE_SCALING_PRECISION	2
#define DEFAULT_ANALOG_SCALING_PRECISION	2

typedef struct _VersionInfo{
	unsigned int major;
	unsigned int minor;
	unsigned int bugfix;
} VersionInfo;

#define DEFAULT_VERSION_INFO {MAJOR_REV, MINOR_REV, BUGFIX_REV}

#define DEFAULT_LABEL_LENGTH					12
#define DEFAULT_UNITS_LENGTH					8

#define DEFAULT_CHANNEL_LOGGING_PRECISION		2
#define DEFAULT_CHANNEL_UNITS					""
#define DEFAULT_CHANNEL_MIN						0
#define DEFAULT_CHANNEL_MAX						1000

/*
 * These Flags dictate special behavior within the ChannelConfig struct.
 */
#define ALWAYS_SAMPLED 1 << 0

typedef struct _ChannelConfig{
	char label[DEFAULT_LABEL_LENGTH];
	char units[DEFAULT_UNITS_LENGTH];
	float min;
	float max;
	unsigned short sampleRate;
	unsigned char precision;
	unsigned char flags;
} ChannelConfig;

typedef struct _ScalingMap{
	float rawValues[ANALOG_SCALING_BINS];
	float scaledValues[ANALOG_SCALING_BINS];
} ScalingMap;

enum TimeType {
   TimeType_Uptime,
   TimeType_UtcMillis,
};

struct TimeConfig {
   ChannelConfig cfg;
   enum TimeType tt;
};

#define EMPTY_CHANNEL_CONFIG {"","", 0.0f, 0.0f, SAMPLE_DISABLED, 0}

// Default to lowest active sample rate.  This will change in code later.
#define DEFAULT_UPTIME_CONFIG {"Interval", "ms", 0, 0, SAMPLE_1Hz, 0, ALWAYS_SAMPLED}
#define DEFAULT_UTC_MILLIS_CONFIG {"Utc", "ms", 0, 0, SAMPLE_1Hz, 0, ALWAYS_SAMPLED}

#define DEFAULT_UPTIME_TIME_CONFIG {DEFAULT_UPTIME_CONFIG, TimeType_Uptime}
#define DEFAULT_UTC_MILLIS_TIME_CONFIG {DEFAULT_UTC_MILLIS_CONFIG, TimeType_UtcMillis}


typedef struct _ADCConfig{
	ChannelConfig cfg;
	float linearScaling;
	float linearOffset;
	float filterAlpha;
	float calibration;
	unsigned char scalingMode;
	ScalingMap scalingMap;
} ADCConfig;

#define DEFAULT_SCALING (1)
#define DEFAULT_FILTER_ALPHA (1.0f)
#define DEFAULT_CALIBRATION (1.0f)
#define DEFAULT_SCALING_MAP {{0,1.25,2.5,3.75,5.0},{0,1.25,2.5,3.75,5.0}}

#define DEFAULT_ADC_CHANNEL_CONFIG {"", "Volts", 0, 5, SAMPLE_DISABLED, 2, 0}
// Define channel config for battery
#define DEFAULT_ADC_BATTERY_CONFIG {"Battery", "Volts", 0, 20, SAMPLE_1Hz, 2, 0}

#define BATTERY_ADC_CONFIG                      \
   {                                            \
      DEFAULT_ADC_BATTERY_CONFIG,               \
         DEFAULT_SCALING,                       \
         0,                                     \
         DEFAULT_FILTER_ALPHA,                  \
         DEFAULT_CALIBRATION,					\
         DEFAULT_SCALING_MODE,                  \
         DEFAULT_SCALING_MAP                    \
         }

#define DEFAULT_ADC_CONFIG                      \
   {                                            \
      DEFAULT_ADC_CHANNEL_CONFIG,               \
         DEFAULT_SCALING,                       \
         0,                                     \
         DEFAULT_FILTER_ALPHA,                  \
         DEFAULT_CALIBRATION,					\
         DEFAULT_SCALING_MODE,                  \
         DEFAULT_SCALING_MAP                    \
         }

typedef struct _TimerConfig{
	ChannelConfig cfg;
	char slowTimerEnabled;
	unsigned char mode;
	float filterAlpha;
	unsigned char pulsePerRevolution;
	unsigned short timerSpeed;
} TimerConfig;


#define MODE_LOGGING_TIMER_RPM		        0
#define MODE_LOGGING_TIMER_FREQUENCY		1
#define MODE_LOGGING_TIMER_PERIOD_MS		2
#define MODE_LOGGING_TIMER_PERIOD_USEC		3


#define TIMER_SLOW			0
#define TIMER_MEDIUM		1
#define TIMER_FAST			2

#define DEFAULT_TIMER_FILTER_ALPHA (1.0f)
#define DEFAULT_TIMER_PPR 1
#define DEFAULT_TIMER_DIVIDER TIMER_MEDIUM
#define DEFAULT_TIMER_SCALING 375428

#define DEFAULT_RPM_CHANNEL_CONFIG {"RPM", "", 0, 10000, SAMPLE_DISABLED, 0, 0}
#define DEFAULT_FREQUENCY_CHANNEL_CONFIG {"", "", 0, 1000, SAMPLE_DISABLED, 0, 0}

#define DEFAULT_FREQUENCY_CONFIG {              \
      DEFAULT_FREQUENCY_CHANNEL_CONFIG,         \
         0,                                     \
         MODE_LOGGING_TIMER_RPM,                \
         1.0F,                                  \
         1,                                     \
         TIMER_MEDIUM                           \
         }

typedef struct _GPIOConfig{
	ChannelConfig cfg;
	unsigned char mode;
} GPIOConfig;

#define CONFIG_GPIO_IN  					0
#define CONFIG_GPIO_OUT  					1

#define DEFAULT_GPIO_MODE CONFIG_GPIO_IN
#define DEFAULT_GPIO_CHANNEL_CONFIG {"", "", 0, 1, SAMPLE_DISABLED, 1, 0}
#define DEFAULT_GPIO_CONFIG {DEFAULT_GPIO_CHANNEL_CONFIG, CONFIG_GPIO_IN}


typedef struct _ImuConfig{
	ChannelConfig cfg;
	unsigned char mode;
	unsigned char physicalChannel;
	signed short zeroValue;
	float filterAlpha;
} ImuConfig;

#define MIN_IMU_RAW							0
#define MAX_IMU_RAW							4097

#define MODE_IMU_DISABLED  					0
#define MODE_IMU_NORMAL  					1
#define MODE_IMU_INVERTED  					2

#define IMU_CHANNEL_X						0
#define IMU_CHANNEL_Y						1
#define IMU_CHANNEL_Z						2
#define IMU_CHANNEL_YAW						3
#define IMU_CHANNEL_PITCH					4
#define IMU_CHANNEL_ROLL					5

#define DEFAULT_ACCEL_ZERO					2048
#define DEFAULT_GYRO_ZERO					1862 //LY330ALH zero state voltage output is 1.5v

#define DEFAULT_ACCEL_X_CONFIG    {"AccelX",   "G", -3, 3, SAMPLE_25Hz, 2, 0}
#define DEFAULT_ACCEL_Y_CONFIG    {"AccelY",   "G", -3, 3, SAMPLE_25Hz, 2, 0}
#define DEFAULT_ACCEL_Z_CONFIG    {"AccelZ",   "G", -3, 3, SAMPLE_25Hz, 2, 0}
#define DEFAULT_GYRO_YAW_CONFIG   {"Yaw",      "Deg/Sec", -300, 300, SAMPLE_25Hz, 1, 0}
#define DEFAULT_GYRO_PITCH_CONFIG {"Pitch",    "Deg/Sec", -300, 300, SAMPLE_25Hz, 1, 0}
#define DEFAULT_GYRO_ROLL_CONFIG  {"Roll",     "Deg/Sec", -300, 300, SAMPLE_25Hz, 1, 0}

#define DEFAULT_IMU_CHANNEL_CONFIG  {"", "G", -3, 3, SAMPLE_25Hz, 2, 0}
#define DEFAULT_GYRO_CHANNEL_CONFIG {"", "Deg/Sec", -300, 300, SAMPLE_25Hz, 1, 0}

#define DEFAULT_IMU_CONFIG                      \
   {                                            \
      DEFAULT_IMU_CHANNEL_CONFIG,               \
         MODE_IMU_NORMAL,                       \
         0,                                     \
         DEFAULT_ACCEL_ZERO,                    \
         0.1F}

#define DEFAULT_GYRO_CONFIG {                   \
      DEFAULT_GYRO_CHANNEL_CONFIG,              \
         MODE_IMU_NORMAL,                       \
         IMU_CHANNEL_YAW,                       \
         DEFAULT_GYRO_ZERO,                     \
         0.1F                                   \
         }



typedef struct _PWMConfig{
	ChannelConfig cfg;
	unsigned char outputMode;
	unsigned char loggingMode;
	unsigned char startupDutyCycle;
	unsigned short startupPeriod;
} PWMConfig;

/// PWM frequency in Hz.
#define MAX_PWM_CLOCK_FREQUENCY             200000
#define MIN_PWM_CLOCK_FREQUENCY	 	10
#define DEFAULT_PWM_CLOCK_FREQUENCY			10000

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

typedef struct _PidConfig{
	ChannelConfig cfg;
	unsigned short pid;
} PidConfig;


typedef struct _OBD2Config{
	unsigned char enabled;
	unsigned char obd2SampleRate;
	unsigned short enabledPids;
	PidConfig pids[OBD2_CHANNELS];
} OBD2Config;

#define DEFAULT_ENABLED_PIDS 1
#define DEFAULT_OBD2_SAMPLE_RATE SAMPLE_10Hz

#define DEFAULT_OBD2_CONFIG \
{ \
	CONFIG_FEATURE_NOT_INSTALLED, \
	DEFAULT_OBD2_SAMPLE_RATE, \
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

typedef struct _CANConfig{
	unsigned char enabled;
	int baud[CONFIG_CAN_CHANNELS];
} CANConfig;

#define DEFAULT_CAN_BAUD_RATE 500000

typedef struct _GPSConfig{
   ChannelConfig latitude;
   ChannelConfig longitude;
   ChannelConfig speed;
   ChannelConfig distance;
   ChannelConfig satellites;
} GPSConfig;

//HACK: FIX ME for MARK3
#if (MAX_GPS_SAMPLE_RATE == 10)
#define MAX_GPS_SAMPLE_HZ SAMPLE_10Hz
#else
#define MAX_GPS_SAMPLE_HZ SAMPLE_50Hz
#endif

#define DEFAULT_GPS_LATITUDE_CONFIG {"Latitude", "Degrees", -180, 180, MAX_GPS_SAMPLE_HZ, 6, 0}
#define DEFAULT_GPS_LONGITUDE_CONFIG {"Longitude", "Degrees", -180, 180, MAX_GPS_SAMPLE_HZ, 6, 0}
#define DEFAULT_GPS_SPEED_CONFIG {"Speed", "MPH", 0, 150, MAX_GPS_SAMPLE_HZ, 2, 0}
#define DEFAULT_GPS_DISTANCE_CONFIG {"Distance", "Miles", 0, 0, MAX_GPS_SAMPLE_HZ, 3, 0}
#define DEFAULT_GPS_SATELLITE_CONFIG {"GPSSats", "", 0, 100, MAX_GPS_SAMPLE_HZ, 0, 0}

#define DEFAULT_GPS_CONFIG {                    \
      DEFAULT_GPS_LATITUDE_CONFIG,              \
         DEFAULT_GPS_LONGITUDE_CONFIG,          \
         DEFAULT_GPS_SPEED_CONFIG,              \
         DEFAULT_GPS_DISTANCE_CONFIG,           \
         DEFAULT_GPS_SATELLITE_CONFIG           \
         }

typedef struct _LapConfig{
	ChannelConfig lapCountCfg;
	ChannelConfig lapTimeCfg;
	ChannelConfig sectorCfg;
	ChannelConfig sectorTimeCfg;
	ChannelConfig predTimeCfg;
} LapConfig;

#define DEFAULT_LAP_COUNT_CONFIG {"LapCount", "", 0, 0, SAMPLE_1Hz, 0, 0}
#define DEFAULT_LAP_TIME_CONFIG {"LapTime", "Min", 0, 0, SAMPLE_1Hz, 4, 0}
#define DEFAULT_SECTOR_CONFIG {"Sector", "", 0, 0, SAMPLE_1Hz, 4, 0}
#define DEFAULT_SECTOR_TIME_CONFIG {"SectorTime", "Min", 0, 0, SAMPLE_1Hz, 4, 0}
#define DEFAULT_PRED_TIME_CONFIG {"PredTime", "Min", 0, 0, SAMPLE_1Hz, 4, 0}

#define DEFAULT_LAP_CONFIG {                    \
      DEFAULT_LAP_COUNT_CONFIG,                 \
         DEFAULT_LAP_TIME_CONFIG,               \
         DEFAULT_SECTOR_CONFIG,                 \
         DEFAULT_SECTOR_TIME_CONFIG,            \
         DEFAULT_PRED_TIME_CONFIG               \
         }

typedef struct _TrackConfig{
	float radius;
	unsigned char auto_detect;
	Track track;
} TrackConfig;

#define DEFAULT_TRACK_AUTO_DETECT 1

#define BLUETOOTH_ENABLED				1
#define BLUETOOTH_DISABLED				0

#define BT_DEVICE_NAME_LENGTH 21
#define BT_PASSCODE_LENGTH 5
#define DEFAULT_BT_DEVICE_NAME "RaceCapturePro"
#define DEFAULT_BT_PASSCODE "1010"
#define DEFAULT_BT_BAUD 115200
#define DEFAULT_BT_ENABLED BLUETOOTH_ENABLED

typedef struct _BluetoothConfig{
	unsigned char btEnabled;
	char deviceName [BT_DEVICE_NAME_LENGTH];
	char passcode [BT_PASSCODE_LENGTH];
	unsigned int baudRate;
} BluetoothConfig;

#define DEFAULT_BT_CONFIG { \
	DEFAULT_BT_ENABLED, \
	DEFAULT_BT_DEVICE_NAME, \
	DEFAULT_BT_PASSCODE, \
	DEFAULT_BT_BAUD \
}

#define CELL_ENABLED				1
#define CELL_DISABLED				0

#define CELL_APN_HOST_LENGTH 30
#define CELL_APN_USER_LENGTH 30
#define CELL_APN_PASS_LENGTH 30
#define DEFAULT_APN_HOST "epc.tmobile.com"
#define DEFAULT_APN_USER ""
#define DEFAULT_APN_PASS ""
#define DEFAULT_CELL_ENABLED CELL_DISABLED

typedef struct _CellularConfig{
	unsigned char cellEnabled;
	char apnHost [CELL_APN_HOST_LENGTH + 1];
	char apnUser [CELL_APN_USER_LENGTH + 1];
	char apnPass [CELL_APN_PASS_LENGTH + 1];
} CellularConfig;


#define DEVICE_ID_LENGTH 36
#define TELEMETRY_SERVER_HOST_LENGTH 100

#define DEFAULT_DEVICE_ID ""
#define DEFAULT_TELEMETRY_SERVER_HOST "race-capture.com"

#define BACKGROUND_STREAMING_ENABLED				1
#define BACKGROUND_STREAMING_DISABLED				0

typedef struct _TelemetryConfig {
	unsigned char backgroundStreaming;
	char telemetryDeviceId[DEVICE_ID_LENGTH + 1];
	char telemetryServerHost[TELEMETRY_SERVER_HOST_LENGTH + 1];
} TelemetryConfig;


typedef struct _ConnectivityConfig {
	BluetoothConfig bluetoothConfig;
	CellularConfig cellularConfig;
	TelemetryConfig telemetryConfig;
} ConnectivityConfig;

#define SD_LOGGING_MODE_DISABLED					0
#define SD_LOGGING_MODE_CSV							1


typedef struct _LoggerConfig {
   VersionInfo RcpVersionInfo;

   //PWM/Analog out configurations
   unsigned short PWMClockFrequency;

   // Time Config
   struct TimeConfig TimeConfigs[CONFIG_TIME_CHANNELS];

   //ADC Calibrations
   ADCConfig ADCConfigs[CONFIG_ADC_CHANNELS];

   PWMConfig PWMConfigs[CONFIG_PWM_CHANNELS];

   //GPIO configurations
   GPIOConfig GPIOConfigs[CONFIG_GPIO_CHANNELS];

   //Timer Configurations
   TimerConfig TimerConfigs[CONFIG_TIMER_CHANNELS];

   //IMU Configurations
   ImuConfig ImuConfigs[CONFIG_IMU_CHANNELS];

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

   //Padding data to accommodate flash routine
   char padding_data[FLASH_PAGE_SIZE];
} LoggerConfig;


void initialize_logger_config();

const LoggerConfig * getSavedLoggerConfig();
LoggerConfig * getWorkingLoggerConfig();

int getConnectivitySampleRateLimit();
int encodeSampleRate(int sampleRate);
int decodeSampleRate(int sampleRateCode);

unsigned char filterAnalogScalingMode(unsigned char mode);
unsigned char filterBgStreamingMode(unsigned char mode);
unsigned char filterSdLoggingMode(unsigned char mode);
char filterGpioMode(int config);
char filterPwmOutputMode(int config);
char filterPwmLoggingMode(int config);
unsigned short filterPwmDutyCycle(int dutyCycle);
unsigned short filterPwmPeriod(int period);
int filterImuRawValue(int accelRawValue);
int filterPwmClockFrequency(int frequency);
char filterTimerMode(int config);
unsigned char filterPulsePerRevolution(unsigned char pulsePerRev);
unsigned short filterTimerDivider(unsigned short divider);
int filterImuMode(int mode);
int filterImuChannel(int channel);

TimerConfig * getTimerConfigChannel(int channel);
ADCConfig * getADCConfigChannel(int channel);
PWMConfig * getPwmConfigChannel(int channel);
GPIOConfig * getGPIOConfigChannel(int channel);
ImuConfig * getImuConfigChannel(int channel);

unsigned int getHighestSampleRate(LoggerConfig *config);
size_t get_enabled_channel_count(LoggerConfig *loggerConfig);

int flashLoggerConfig(void);
int flash_default_logger_config(void);

bool isHigherSampleRate(const int contender, const int champ);
int getHigherSampleRate(const int a, const int b);

#endif /*LOGGERCONFIG_H_*/
