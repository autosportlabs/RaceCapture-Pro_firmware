#ifndef LOGGERCONFIG_H_
#define LOGGERCONFIG_H_

#include <stddef.h>
#include "channelMeta.h"

#define FLASH_PAGE_SIZE						((unsigned int) 256) // Internal FLASH Page Size: 256 bytes

#define CONFIG_FEATURE_INSTALLED			1
#define CONFIG_FEATURE_NOT_INSTALLED		0

//Number of channels per feature
#define CONFIG_ADC_CHANNELS					8
#define CONFIG_ACCEL_CHANNELS				4
#define CONFIG_GPIO_CHANNELS				3
#define CONFIG_PWM_CHANNELS					4
#define CONFIG_TIMER_CHANNELS				3

#define SLOW_LINK_MAX_TELEMETRY_SAMPLE_RATE SAMPLE_10Hz
#define FAST_LINK_MAX_TELEMETRY_SAMPLE_RATE SAMPLE_50Hz


//standard sample rates based on OS timer ticks
#define SAMPLE_200Hz						1
#define SAMPLE_100Hz 						2
#define SAMPLE_50Hz 						4
#define SAMPLE_25Hz 						8
#define SAMPLE_10Hz 						20
#define SAMPLE_5Hz 							40
#define SAMPLE_1Hz 							200
#define SAMPLE_DISABLED 					0

#define DEFAULT_LABEL_LENGTH				11
#define DEFAULT_UNITS_LENGTH				11

#define ANALOG_SCALING_PRECISION			4

#define ANALOG_SCALING_BINS					5

#define SCALING_MODE_RAW					0
#define SCALING_MODE_LINEAR					1
#define SCALING_MODE_MAP					2
#define DEFAULT_SCALING_MODE				SCALING_MODE_LINEAR

#define HIGHER_SAMPLE(X,Y) 					((X != SAMPLE_DISABLED && X < Y))
#define HIGHER_SAMPLE_RATE(X,Y)				((X != SAMPLE_DISABLED && Y != SAMPLE_DISABLED && X < Y) || (X != SAMPLE_DISABLED && Y == SAMPLE_DISABLED) ? X : Y)
#define LOWER_SAMPLE_RATE(X,Y)				(X > Y ? X : Y)

typedef struct _ChannelConfig{
	unsigned short channeNameId;
	unsigned char sampleRate;
} ChannelConfig;

typedef struct _ScalingMap{
	unsigned short rawValues[ANALOG_SCALING_BINS];
	float scaledValues[ANALOG_SCALING_BINS];
} ScalingMap;

typedef struct _ADCConfig{
	ChannelConfig cfg;
	unsigned char loggingPrecision;
	float linearScaling;
	unsigned char scalingMode;
	ScalingMap scalingMap;
} ADCConfig;

#define DEFAULT_ADC_LOGGING_PRECISION		2
#define DEFAULT_SCALING_MAP {{0,256,512,768,1023},{0,1.25,2.5,3.75,5.0}}

#define DEFAULT_ADC0_CONFIG {{CHANNEL_Analog1, SAMPLE_DISABLED},DEFAULT_ADC_LOGGING_PRECISION,0.0048875f,DEFAULT_SCALING_MODE,DEFAULT_SCALING_MAP}
#define DEFAULT_ADC1_CONFIG {{CHANNEL_Analog2, SAMPLE_DISABLED},DEFAULT_ADC_LOGGING_PRECISION,0.0048875f,DEFAULT_SCALING_MODE,DEFAULT_SCALING_MAP}
#define DEFAULT_ADC2_CONFIG {{CHANNEL_Analog3, SAMPLE_DISABLED},DEFAULT_ADC_LOGGING_PRECISION,0.0048875f,DEFAULT_SCALING_MODE,DEFAULT_SCALING_MAP}
#define DEFAULT_ADC3_CONFIG {{CHANNEL_Analog4, SAMPLE_DISABLED},DEFAULT_ADC_LOGGING_PRECISION,0.0048875f,DEFAULT_SCALING_MODE,DEFAULT_SCALING_MAP}
#define DEFAULT_ADC4_CONFIG {{CHANNEL_Analog5, SAMPLE_DISABLED},DEFAULT_ADC_LOGGING_PRECISION,0.0048875f,DEFAULT_SCALING_MODE,DEFAULT_SCALING_MAP}
#define DEFAULT_ADC5_CONFIG {{CHANNEL_Analog6, SAMPLE_DISABLED},DEFAULT_ADC_LOGGING_PRECISION,0.0048875f,DEFAULT_SCALING_MODE,DEFAULT_SCALING_MAP}
#define DEFAULT_ADC6_CONFIG {{CHANNEL_Analog7, SAMPLE_DISABLED},DEFAULT_ADC_LOGGING_PRECISION,0.0048875f,DEFAULT_SCALING_MODE,DEFAULT_SCALING_MAP}
#define BATTERY_ADC7_CONFIG {{CHANNEL_Battery, SAMPLE_1Hz}, DEFAULT_ADC_LOGGING_PRECISION,0.0171f,DEFAULT_SCALING_MODE,DEFAULT_SCALING_MAP}
#define DEFAULT_ADC_CONFIGS \
			{ \
			DEFAULT_ADC0_CONFIG, \
			DEFAULT_ADC1_CONFIG, \
			DEFAULT_ADC2_CONFIG, \
			DEFAULT_ADC3_CONFIG, \
			DEFAULT_ADC4_CONFIG, \
			DEFAULT_ADC5_CONFIG, \
			DEFAULT_ADC6_CONFIG, \
			BATTERY_ADC7_CONFIG \
			}

typedef struct _TimerConfig{
	ChannelConfig cfg;
	unsigned char loggingPrecision;
	char slowTimerEnabled;
	unsigned char mode;
	unsigned char pulsePerRevolution;
	unsigned short timerDivider;
	unsigned int calculatedScaling;
} TimerConfig;


#define MODE_LOGGING_TIMER_RPM				0
#define MODE_LOGGING_TIMER_FREQUENCY		1
#define MODE_LOGGING_TIMER_PERIOD_MS		2
#define MODE_LOGGING_TIMER_PERIOD_USEC		3

#define DEFAULT_TIMER_LOGGING_PRECISION		0

// MCK: 48054840 Hz
// /2 = 24027420
// /8 = 6006855
// /32 = 1501713.75
// /128 = 375428.4375
// /1024 = 46928.5546875
#define TIMER_MCK_2 		2
#define TIMER_MCK_8 		8
#define TIMER_MCK_32 		32
#define TIMER_MCK_128 		128
#define TIMER_MCK_1024 		1024

#define DEFAULT_RPM_TIMER_CONFIG  {{CHANNEL_RPM, SAMPLE_DISABLED}, DEFAULT_TIMER_LOGGING_PRECISION, 0, MODE_LOGGING_TIMER_RPM, 1, TIMER_MCK_128, 375428}
#define DEFAULT_FREQUENCY2_CONFIG {{CHANNEL_Freq2, SAMPLE_DISABLED}, DEFAULT_TIMER_LOGGING_PRECISION, 0, MODE_LOGGING_TIMER_FREQUENCY, 1, TIMER_MCK_128, 375428}
#define DEFAULT_FREQUENCY3_CONFIG {{CHANNEL_Freq3, SAMPLE_DISABLED}, DEFAULT_TIMER_LOGGING_PRECISION, 0, MODE_LOGGING_TIMER_FREQUENCY, 1, TIMER_MCK_128, 375428}
#define DEFAULT_TIMER_CONFIGS \
			{ \
			DEFAULT_RPM_TIMER_CONFIG, \
			DEFAULT_FREQUENCY2_CONFIG, \
			DEFAULT_FREQUENCY3_CONFIG \
			}

typedef struct _GPIOConfig{
	ChannelConfig cfg;
	unsigned char mode;
} GPIOConfig;

#define	CONFIG_GPIO_IN  					0
#define CONFIG_GPIO_OUT  					1
#define DEFAULT_GPIO_LOGGING_PRECISION		0

#define DEFAULT_GPIO1_CONFIG {{CHANNEL_GPIO1, SAMPLE_DISABLED}, CONFIG_GPIO_IN}
#define DEFAULT_GPIO2_CONFIG {{CHANNEL_GPIO2, SAMPLE_DISABLED}, CONFIG_GPIO_IN}
#define DEFAULT_GPIO3_CONFIG {{CHANNEL_GPIO3, SAMPLE_DISABLED}, CONFIG_GPIO_IN}
#define DEFAULT_GPIO_CONFIGS \
			{ \
			DEFAULT_GPIO1_CONFIG, \
			DEFAULT_GPIO2_CONFIG, \
			DEFAULT_GPIO3_CONFIG \
			}
			
typedef struct _AccelConfig{
	ChannelConfig cfg;
	unsigned char mode;
	unsigned char accelChannel;
	unsigned short zeroValue;
} AccelConfig;

#define MIN_ACCEL_RAW						0
#define MAX_ACCEL_RAW						4097

#define MODE_ACCEL_DISABLED  				0
#define MODE_ACCEL_NORMAL  					1
#define MODE_ACCEL_INVERTED  				2

#define ACCEL_CHANNEL_X						0
#define ACCEL_CHANNEL_Y						1
#define ACCEL_CHANNEL_Z						2
#define	ACCEL_CHANNEL_ZT					3

#define DEFAULT_ACCEL_LOGGING_PRECISION		3
#define DEFAULT_ACCEL_ZERO					2048
#define DEFAULT_YAW_ZERO					1862 //LY330ALH zero state voltage output is 1.5v


#define DEFAULT_ACCEL_X_AXIS_CONFIG  {{CHANNEL_AccelX, SAMPLE_25Hz}, MODE_ACCEL_NORMAL, ACCEL_CHANNEL_X,DEFAULT_ACCEL_ZERO}
#define DEFAULT_ACCEL_Y_AXIS_CONFIG  {{CHANNEL_AccelY, SAMPLE_25Hz}, MODE_ACCEL_NORMAL, ACCEL_CHANNEL_Y,DEFAULT_ACCEL_ZERO}
#define DEFAULT_ACCEL_Z_AXIS_CONFIG  {{CHANNEL_AccelZ, SAMPLE_25Hz}, MODE_ACCEL_NORMAL, ACCEL_CHANNEL_Z,DEFAULT_ACCEL_ZERO}
#define DEFAULT_ACCEL_YAW_AXIS_CONFIG {{CHANNEL_Yaw, SAMPLE_25Hz}, MODE_ACCEL_NORMAL, ACCEL_CHANNEL_ZT,DEFAULT_YAW_ZERO}
#define DEFAULT_ACCEL_CONFIGS \
			{ \
				DEFAULT_ACCEL_X_AXIS_CONFIG, \
				DEFAULT_ACCEL_Y_AXIS_CONFIG, \
				DEFAULT_ACCEL_Z_AXIS_CONFIG, \
				DEFAULT_ACCEL_YAW_AXIS_CONFIG \
			}
	
typedef struct _PWMConfig{
	ChannelConfig cfg;
	unsigned char loggingPrecision;
	unsigned char outputMode;
	unsigned char loggingMode;
	unsigned char startupDutyCycle;
	unsigned short startupPeriod;
	float voltageScaling;
} PWMConfig;

#define VOLTAGE_SCALING_PRECISION			4
/// PWM frequency in Hz.
#define MAX_PWM_CLOCK_FREQUENCY             2000
#define MIN_PWM_CLOCK_FREQUENCY				10
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

#define DEFAULT_PWM_VOLTAGE_SCALING			0.1

#define DEFAULT_PWM_LOGGING_PRECISION		2

#define DEFAULT_PWM1_CONFIG {{CHANNEL_PWM1, SAMPLE_DISABLED},DEFAULT_PWM_LOGGING_PRECISION,MODE_PWM_ANALOG,MODE_LOGGING_PWM_VOLTS,50,100,DEFAULT_PWM_VOLTAGE_SCALING}
#define DEFAULT_PWM2_CONFIG {{CHANNEL_PWM1, SAMPLE_DISABLED},DEFAULT_PWM_LOGGING_PRECISION,MODE_PWM_ANALOG,MODE_LOGGING_PWM_PERIOD,50,100,DEFAULT_PWM_VOLTAGE_SCALING}
#define DEFAULT_PWM3_CONFIG {{CHANNEL_PWM1, SAMPLE_DISABLED},DEFAULT_PWM_LOGGING_PRECISION,MODE_PWM_ANALOG,MODE_LOGGING_PWM_DUTY,50,100,DEFAULT_PWM_VOLTAGE_SCALING}
#define DEFAULT_PWM4_CONFIG {{CHANNEL_PWM1, SAMPLE_DISABLED},DEFAULT_PWM_LOGGING_PRECISION,MODE_PWM_ANALOG,MODE_LOGGING_PWM_PERIOD,50,100,DEFAULT_PWM_VOLTAGE_SCALING}
#define DEFAULT_PWM_CONFIGS \
			{ \
				DEFAULT_PWM1_CONFIG, \
				DEFAULT_PWM2_CONFIG, \
				DEFAULT_PWM3_CONFIG, \
				DEFAULT_PWM4_CONFIG, \
			}

#define OBD2_CHANNELS 20

typedef struct _PidConfig{
	ChannelConfig cfg;
	unsigned short pid;
} PidConfig;

#define OBD2_CHANNELS 20

typedef struct _OBD2Config{
	unsigned char obd2SampleRate;
	unsigned short enabledPids;
	PidConfig pids[OBD2_CHANNELS];
} OBD2Config;

#define DEFAULT_ENABLED_PIDS 1
#define DEFAULT_OBD2_SAMPLE_RATE SAMPLE_10Hz

#define DEFAULT_OBD2_CONFIG \
{ \
	DEFAULT_OBD2_SAMPLE_RATE, \
	DEFAULT_ENABLED_PIDS, \
	{ \
		{{CHANNEL_RPM, SAMPLE_1Hz}, 0x0C}, \
		{{CHANNEL_Unknown, SAMPLE_DISABLED}, 0}, \
		{{CHANNEL_Unknown, SAMPLE_DISABLED}, 0}, \
		{{CHANNEL_Unknown, SAMPLE_DISABLED}, 0}, \
		{{CHANNEL_Unknown, SAMPLE_DISABLED}, 0}, \
		{{CHANNEL_Unknown, SAMPLE_DISABLED}, 0}, \
		{{CHANNEL_Unknown, SAMPLE_DISABLED}, 0}, \
		{{CHANNEL_Unknown, SAMPLE_DISABLED}, 0}, \
		{{CHANNEL_Unknown, SAMPLE_DISABLED}, 0}, \
		{{CHANNEL_Unknown, SAMPLE_DISABLED}, 0}, \
		{{CHANNEL_Unknown, SAMPLE_DISABLED}, 0}, \
		{{CHANNEL_Unknown, SAMPLE_DISABLED}, 0}, \
		{{CHANNEL_Unknown, SAMPLE_DISABLED}, 0}, \
		{{CHANNEL_Unknown, SAMPLE_DISABLED}, 0}, \
		{{CHANNEL_Unknown, SAMPLE_DISABLED}, 0}, \
		{{CHANNEL_Unknown, SAMPLE_DISABLED}, 0}, \
		{{CHANNEL_Unknown, SAMPLE_DISABLED}, 0}, \
		{{CHANNEL_Unknown, SAMPLE_DISABLED}, 0}, \
		{{CHANNEL_Unknown, SAMPLE_DISABLED}, 0}, \
		{{CHANNEL_Unknown, SAMPLE_DISABLED}, 0} \
	} \
}

typedef struct _CANConfig{
	unsigned char enabled;
	unsigned int CANbaudRate;
} CANConfig;

#define DEFAULT_CAN_BAUD_RATE 500000

#define DEFAULT_CAN_CONFIG \
{ \
	CONFIG_FEATURE_NOT_INSTALLED, \
	DEFAULT_CAN_BAUD_RATE \
}


enum gps_channels{
	gps_channel_latitude,
	gps_channel_longitude,
	gps_channel_speed,
	gps_channel_time,
	gps_channel_satellites
};

typedef struct _GPSConfig{
	char GPSInstalled;
	ChannelConfig satellitesCfg;
	ChannelConfig latitudeCfg;
	ChannelConfig longitudeCfg;
	ChannelConfig timeCfg;
	ChannelConfig speedCfg;
} GPSConfig;


#define DEFAULT_GPS_POSITION_LOGGING_PRECISION 		6
#define DEFAULT_GPS_SPEED_LOGGING_PRECISION 		2
#define DEFAULT_GPS_RADIUS_LOGGING_PRECISION 		5
#define DEFAULT_GPS_TIME_LOGGING_PRECISION 			3
#define DEFAULT_LAP_COUNT_LOGGING_PRECISION			0
#define DEFAULT_LAP_TIME_LOGGING_PRECISION			3
#define	DEFAULT_GPS_QUALITY_LOGGING_PRECISION 		0
#define DEFAULT_GPS_SATELLITES_LOGGING_PRECISION 	0

//currently in degrees. This is about a 73 foot diameter circle (in the pacific NW...)
#define DEFAULT_GPS_SATELLITES_CONFIG {CHANNEL_GPSSats, SAMPLE_DISABLED}
#define DEFAULT_GPS_LATITUDE_CONFIG {CHANNEL_Latitude, SAMPLE_10Hz}
#define DEFAULT_GPS_LONGITUDE_CONFIG {CHANNEL_Latitude, SAMPLE_10Hz}
#define DEFAULT_GPS_TIME_CONFIG {CHANNEL_Time, SAMPLE_10Hz}
#define DEFAULT_GPS_SPEED_CONFIG {CHANNEL_Speed, SAMPLE_10Hz}

#define DEFAULT_GPS_CONFIG {CONFIG_FEATURE_INSTALLED, \
							DEFAULT_GPS_SATELLITES_CONFIG, \
							DEFAULT_GPS_LATITUDE_CONFIG, \
							DEFAULT_GPS_LONGITUDE_CONFIG, \
							DEFAULT_GPS_TIME_CONFIG, \
							DEFAULT_GPS_SPEED_CONFIG}


typedef struct _GPSTargetConfig{
	float latitude;
	float longitude;
	float targetRadius;
} GPSTargetConfig;

#define DEFAULT_DISTANCE_LOGGING_PRECISION 3

enum lap_stat_channels{
	lap_stat_channel_lapcount,
	lap_stat_channel_laptime,
	lap_stat_channel_splittime,
	lap_stat_channel_distance,
	lap_stat_channel_predtime
};

typedef struct _TrackConfig{
	GPSTargetConfig startFinishConfig;
	GPSTargetConfig splitConfig;
	ChannelConfig lapCountCfg;
	ChannelConfig lapTimeCfg;
	ChannelConfig splitTimeCfg;
	ChannelConfig distanceCfg;
	ChannelConfig predTimeCfg;
} TrackConfig;

#define DEFAULT_GPS_TARGET_CONFIG {0,0, 0.0004}
#define DEFAULT_LAP_COUNT_CONFIG {CHANNEL_LapCount, SAMPLE_1Hz}
#define DEFAULT_LAP_TIME_CONFIG {CHANNEL_LapTime, SAMPLE_1Hz}
#define DEFAULT_SPLIT_TIME_CONFIG {CHANNEL_SplitTime, SAMPLE_1Hz}
#define DEFAULT_DISTANCE_CONFIG {CHANNEL_Distance, SAMPLE_10Hz}
#define DEFAULT_PRED_TIME_CONFIG {CHANNEL_PredTime, SAMPLE_DISABLED}

#define DEFAULT_TRACK_CONFIG { \
	DEFAULT_GPS_TARGET_CONFIG, \
	DEFAULT_GPS_TARGET_CONFIG, \
	DEFAULT_LAP_COUNT_CONFIG, \
	DEFAULT_LAP_TIME_CONFIG, \
	DEFAULT_SPLIT_TIME_CONFIG, \
	DEFAULT_DISTANCE_CONFIG, \
	DEFAULT_PRED_TIME_CONFIG}


#define BT_DEVICE_NAME_LENGTH 20
#define BT_PASSCODE_LENGTH 4
#define DEFAULT_BT_DEVICE_NAME "RaceCapturePro"
#define DEFAULT_BT_PASSCODE "1010"
#define DEFAULT_BT_BAUD 115200

typedef struct _BluetoothConfig{
	char deviceName [BT_DEVICE_NAME_LENGTH + 1];
	char passcode [BT_PASSCODE_LENGTH + 1];
	unsigned int baudRate;
} BluetoothConfig;

#define DEFAULT_BT_CONFIG { \
	DEFAULT_BT_DEVICE_NAME, \
	DEFAULT_BT_PASSCODE, \
	DEFAULT_BT_BAUD \
}

#define CELL_APN_HOST_LENGTH 30
#define CELL_APN_USER_LENGTH 20
#define CELL_APN_PASS_LENGTH 20
#define DEFAULT_APN_HOST "epc.tmobile.com"
#define DEFAULT_APN_USER ""
#define DEFAULT_APN_PASS ""

typedef struct _CellularConfig{
	char apnHost [CELL_APN_HOST_LENGTH + 1];
	char apnUser [CELL_APN_USER_LENGTH + 1];
	char apnPass [CELL_APN_PASS_LENGTH + 1];
} CellularConfig;

#define DEFAULT_CELL_CONFIG { \
	DEFAULT_APN_HOST, \
	DEFAULT_APN_USER, \
	DEFAULT_APN_PASS \
}

#define DEVICE_ID_LENGTH 36
#define TELEMETRY_SERVER_HOST_LENGTH 100

#define DEFAULT_DEVICE_ID ""
#define DEFAULT_TELEMETRY_SERVER_HOST "54.245.229.2"

typedef struct _TelemetryConfig {
	char telemetryDeviceId[DEVICE_ID_LENGTH + 1];
	char telemetryServerHost[TELEMETRY_SERVER_HOST_LENGTH + 1];
} TelemetryConfig;

#define DEFAULT_TELEMETRY_CONFIG { \
		DEFAULT_DEVICE_ID, \
		DEFAULT_TELEMETRY_SERVER_HOST \
}

typedef struct _ConnectivityConfig {
	char connectivityMode;
	char sdLoggingMode;
	char backgroundStreaming;
	BluetoothConfig bluetoothConfig;
	CellularConfig cellularConfig;
	TelemetryConfig telemetryConfig;
} ConnectivityConfig;


#define CONNECTIVITY_MODE_CONSOLE 					0
#define CONNECTIVITY_MODE_BLUETOOTH					1
#define CONNECTIVITY_MODE_CELL						2

#define SD_LOGGING_MODE_DISABLED					0
#define SD_LOGGING_MODE_CSV							1

#define BACKGROUND_STREAMING_ENABLED				1
#define BACKGROUND_STREAMING_DISABLED				0

#define DEFAULT_CONNECTIVITY_MODE 					CONNECTIVITY_MODE_BLUETOOTH
#define DEFAULT_SD_LOGGING_MODE						SD_LOGGING_MODE_CSV
#define DEFAULT_BACKGROUND_STREAMING				BACKGROUND_STREAMING_ENABLED

#define DEFAULT_CONNECTIVITY_CONFIG { 	DEFAULT_CONNECTIVITY_MODE, \
										DEFAULT_SD_LOGGING_MODE, \
										DEFAULT_BACKGROUND_STREAMING, \
										DEFAULT_BT_CONFIG, \
										DEFAULT_CELL_CONFIG, \
										DEFAULT_TELEMETRY_CONFIG \
										}

typedef struct _LoggerConfig {
	//ADC Calibrations
	ADCConfig ADCConfigs[CONFIG_ADC_CHANNELS];
	//PWM/Analog out configurations
	unsigned short PWMClockFrequency;
	PWMConfig PWMConfigs[CONFIG_PWM_CHANNELS];
	//GPIO configurations
	GPIOConfig GPIOConfigs[CONFIG_GPIO_CHANNELS];
	//Timer Configurations
	TimerConfig TimerConfigs[CONFIG_TIMER_CHANNELS];
	//Accelerometer Configurations
	AccelConfig AccelConfigs[CONFIG_ACCEL_CHANNELS];
	//OBD2 Config
	OBD2Config OBD2Config;
	//GPS Configuration
	GPSConfig GPSConfigs;
	//Track configuration
	TrackConfig TrackConfigs;
	//Connectivity Configuration
	ConnectivityConfig ConnectivityConfigs;
	//Padding data to accommodate flash routine
	char padding_data[FLASH_PAGE_SIZE];
} LoggerConfig;

#define DEFAULT_LOGGER_CONFIG \
	{ \
	DEFAULT_ADC_CONFIGS, \
	DEFAULT_PWM_CLOCK_FREQUENCY, \
	DEFAULT_PWM_CONFIGS, \
	DEFAULT_GPIO_CONFIGS, \
	DEFAULT_TIMER_CONFIGS, \
	DEFAULT_ACCEL_CONFIGS, \
	DEFAULT_OBD2_CONFIG, \
	DEFAULT_GPS_CONFIG, \
	DEFAULT_TRACK_CONFIG, \
	DEFAULT_CONNECTIVITY_CONFIG, \
	"" \
	}
	

void initialize_logger_config();

const LoggerConfig * getSavedLoggerConfig();
LoggerConfig * getWorkingLoggerConfig();

void calculateTimerScaling(unsigned int clockHz, TimerConfig *timerConfig);

int getConnectivitySampleRateLimit();
int encodeSampleRate(int sampleRate);
int decodeSampleRate(int sampleRateCode);

char filterAnalogScalingMode(char mode);
char filterSdLoggingMode(char mode);
char filterConnectivityMode(char mode);
char filterGpioMode(int config);
char filterPwmOutputMode(int config);
char filterPwmLoggingMode(int config);
unsigned short filterPwmDutyCycle(int dutyCycle);
unsigned short filterPwmPeriod(int period);
int filterAccelRawValue(int accelRawValue);
int filterPwmClockFrequency(int frequency);
char filterTimerMode(int config);
unsigned short filterTimerDivider(unsigned short divider);
int filterAccelMode(int mode);
int filterAccelChannel(int channel);

TimerConfig * getTimerConfigChannel(int channel);
ADCConfig * getADCConfigChannel(int channel);
PWMConfig * getPwmConfigChannel(int channel);
GPIOConfig * getGPIOConfigChannel(int channel);
AccelConfig * getAccelConfigChannel(int channel);

void setLabelGeneric(char *dest, const char *source);
void setTextField(char *dest, const char *source, unsigned int maxlen);


unsigned int getHighestSampleRate(LoggerConfig *config);
size_t get_enabled_channel_count(LoggerConfig *loggerConfig);

int flashLoggerConfig(void);
int flash_default_logger_config(void);

#endif /*LOGGERCONFIG_H_*/
