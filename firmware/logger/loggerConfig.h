#ifndef LOGGERCONFIG_H_
#define LOGGERCONFIG_H_

#include "board.h"

#define CONFIG_FEATURE_INSTALLED			1
#define CONFIG_FEATURE_NOT_INSTALLED		0

//Number of channels per feature
#define CONFIG_ADC_CHANNELS					8
#define CONFIG_ACCEL_CHANNELS				4
#define CONFIG_GPIO_CHANNELS				3
#define CONFIG_PWM_CHANNELS					4
#define CONFIG_TIMER_CHANNELS				3

//standard sample rates based on OS timer ticks
#define SAMPLE_100Hz 						3
#define SAMPLE_50Hz 						6
#define SAMPLE_30Hz 						10
#define SAMPLE_20Hz 						15
#define SAMPLE_10Hz 						30
#define SAMPLE_5Hz 							60
#define SAMPLE_1Hz 							300
#define SAMPLE_DISABLED 					0


#define DEFAULT_LABEL_LENGTH				11
#define DEFAULT_UNITS_LENGTH				11

#define ANALOG_SCALING_PRECISION			4

#define ANALOG_SCALING_BINS					5

#define SCALING_MODE_RAW					0
#define SCALING_MODE_LINEAR					1
#define SCALING_MODE_MAP					2
#define DEFAULT_SCALING_MODE				SCALING_MODE_LINEAR

typedef struct _ChannelConfig{
	char label[DEFAULT_LABEL_LENGTH];
	char units[DEFAULT_UNITS_LENGTH];
	int sampleRate;
} ChannelConfig;

typedef struct _ScalingMap{
	unsigned short rawValues[ANALOG_SCALING_BINS];
	float scaledValues[ANALOG_SCALING_BINS];
} ScalingMap;

typedef struct _ADCConfig{
	ChannelConfig cfg;
	int loggingPrecision;
	float linearScaling;
	char scalingMode;
	ScalingMap scalingMap;
} ADCConfig;

#define DEFAULT_ADC_LOGGING_PRECISION		2
#define DEFAULT_SCALING_MAP {{0,1024,2048,3072,4095},{0,1.25,2.5,3.75,5.0}}

#define DEFAULT_ADC0_CONFIG {{"Analog1","Units",SAMPLE_DISABLED},DEFAULT_ADC_LOGGING_PRECISION,0.0048875f,DEFAULT_SCALING_MODE,DEFAULT_SCALING_MAP}
#define DEFAULT_ADC1_CONFIG {{"Analog2","Units",SAMPLE_DISABLED},DEFAULT_ADC_LOGGING_PRECISION,0.0048875f,DEFAULT_SCALING_MODE,DEFAULT_SCALING_MAP}
#define DEFAULT_ADC2_CONFIG {{"Analog3","Units",SAMPLE_DISABLED},DEFAULT_ADC_LOGGING_PRECISION,0.0048875f,DEFAULT_SCALING_MODE,DEFAULT_SCALING_MAP}
#define DEFAULT_ADC3_CONFIG {{"Analog4","Units",SAMPLE_DISABLED},DEFAULT_ADC_LOGGING_PRECISION,0.0048875f,DEFAULT_SCALING_MODE,DEFAULT_SCALING_MAP}
#define DEFAULT_ADC4_CONFIG {{"Analog5","Units",SAMPLE_DISABLED},DEFAULT_ADC_LOGGING_PRECISION,0.0048875f,DEFAULT_SCALING_MODE,DEFAULT_SCALING_MAP}
#define DEFAULT_ADC5_CONFIG {{"Analog6","Units",SAMPLE_DISABLED},DEFAULT_ADC_LOGGING_PRECISION,0.0048875f,DEFAULT_SCALING_MODE,DEFAULT_SCALING_MAP}
#define DEFAULT_ADC6_CONFIG {{"Analog7","Units",SAMPLE_DISABLED},DEFAULT_ADC_LOGGING_PRECISION,0.0048875f,DEFAULT_SCALING_MODE,DEFAULT_SCALING_MAP}
#define BATTERY_ADC7_CONFIG {{"Battery","Volts",SAMPLE_DISABLED},DEFAULT_ADC_LOGGING_PRECISION,0.01955f,DEFAULT_SCALING_MODE,DEFAULT_SCALING_MAP}
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
	unsigned int loggingPrecision;
	char slowTimerEnabled;
	char mode;
	char pulsePerRevolution;
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

#define DEFAULT_RPM_TIMER_CONFIG  {{"RPM", "RPM", SAMPLE_DISABLED}, DEFAULT_TIMER_LOGGING_PRECISION, 0, MODE_LOGGING_TIMER_RPM, 1, TIMER_MCK_128, 375428}
#define DEFAULT_FREQUENCY2_CONFIG {{"Freq2","Hz", SAMPLE_DISABLED}, DEFAULT_TIMER_LOGGING_PRECISION, 0, MODE_LOGGING_TIMER_FREQUENCY, 1, TIMER_MCK_128, 375428}
#define DEFAULT_FREQUENCY3_CONFIG {{"Freq3","Hz", SAMPLE_DISABLED}, DEFAULT_TIMER_LOGGING_PRECISION, 0, MODE_LOGGING_TIMER_FREQUENCY, 1, TIMER_MCK_128, 375428}
#define DEFAULT_TIMER_CONFIGS \
			{ \
			DEFAULT_RPM_TIMER_CONFIG, \
			DEFAULT_FREQUENCY2_CONFIG, \
			DEFAULT_FREQUENCY3_CONFIG \
			}

typedef struct _GPIOConfig{
	ChannelConfig cfg;
	char mode;
} GPIOConfig;

#define	CONFIG_GPIO_IN  					0
#define CONFIG_GPIO_OUT  					1
#define DEFAULT_GPIO_LOGGING_PRECISION		0

#define DEFAULT_GPIO1_CONFIG {{"GPIO1", "", SAMPLE_DISABLED}, CONFIG_GPIO_IN}
#define DEFAULT_GPIO2_CONFIG {{"GPIO2", "", SAMPLE_DISABLED}, CONFIG_GPIO_IN}
#define DEFAULT_GPIO3_CONFIG {{"GPIO3", "", SAMPLE_DISABLED}, CONFIG_GPIO_IN}
#define DEFAULT_GPIO_CONFIGS \
			{ \
			DEFAULT_GPIO1_CONFIG, \
			DEFAULT_GPIO2_CONFIG, \
			DEFAULT_GPIO3_CONFIG \
			}
			
typedef struct _AccelConfig{
	ChannelConfig cfg;
	char mode;
	unsigned char accelChannel;
	unsigned long zeroValue;
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
#define DEFAULT_ACCEL_ZERO					2047


#define DEFAULT_ACCEL_X_AXIS_CONFIG  {{"AccelX", "G", SAMPLE_30Hz}, MODE_ACCEL_NORMAL, ACCEL_CHANNEL_X,DEFAULT_ACCEL_ZERO}
#define DEFAULT_ACCEL_Y_AXIS_CONFIG  {{"AccelY", "G", SAMPLE_30Hz}, MODE_ACCEL_NORMAL, ACCEL_CHANNEL_Y,DEFAULT_ACCEL_ZERO}
#define DEFAULT_ACCEL_Z_AXIS_CONFIG  {{"AccelZ", "G", SAMPLE_30Hz}, MODE_ACCEL_NORMAL, ACCEL_CHANNEL_Z,DEFAULT_ACCEL_ZERO}
#define DEFAULT_ACCEL_ZT_AXIS_CONFIG {{"Yaw", "Deg/Sec", SAMPLE_30Hz}, MODE_ACCEL_NORMAL, ACCEL_CHANNEL_ZT,DEFAULT_ACCEL_ZERO}
#define DEFAULT_ACCEL_CONFIGS \
			{ \
				DEFAULT_ACCEL_X_AXIS_CONFIG, \
				DEFAULT_ACCEL_Y_AXIS_CONFIG, \
				DEFAULT_ACCEL_Z_AXIS_CONFIG, \
				DEFAULT_ACCEL_ZT_AXIS_CONFIG \
			}
	
typedef struct _PWMConfig{
	ChannelConfig cfg;
	int loggingPrecision;
	char outputMode;
	char loggingMode;
	unsigned short startupDutyCycle;
	unsigned short startupPeriod;
	float voltageScaling;
} PWMConfig;

#define VOLTAGE_SCALING_PRECISION			2
/// PWM frequency in Hz.
#define MAX_PWM_CLOCK_FREQUENCY             2000
#define MIN_PWM_CLOCK_FREQUENCY				10
#define DEFAULT_PWM_CLOCK_FREQUENCY			100

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

#define DEFAULT_PWM1_CONFIG {{"Vout1","V",SAMPLE_DISABLED},DEFAULT_PWM_LOGGING_PRECISION,MODE_PWM_ANALOG,MODE_LOGGING_PWM_VOLTS,50,100,DEFAULT_PWM_VOLTAGE_SCALING}
#define DEFAULT_PWM2_CONFIG {{"Vout2","V",SAMPLE_DISABLED},DEFAULT_PWM_LOGGING_PRECISION,MODE_PWM_ANALOG,MODE_LOGGING_PWM_PERIOD,50,100,DEFAULT_PWM_VOLTAGE_SCALING}
#define DEFAULT_PWM3_CONFIG {{"Vout3","V",SAMPLE_DISABLED},DEFAULT_PWM_LOGGING_PRECISION,MODE_PWM_ANALOG,MODE_LOGGING_PWM_DUTY,50,100,DEFAULT_PWM_VOLTAGE_SCALING}
#define DEFAULT_PWM4_CONFIG {{"Vout4","V",SAMPLE_DISABLED},DEFAULT_PWM_LOGGING_PRECISION,MODE_PWM_ANALOG,MODE_LOGGING_PWM_PERIOD,50,100,DEFAULT_PWM_VOLTAGE_SCALING}
#define DEFAULT_PWM_CONFIGS \
			{ \
				DEFAULT_PWM1_CONFIG, \
				DEFAULT_PWM2_CONFIG, \
				DEFAULT_PWM3_CONFIG, \
				DEFAULT_PWM4_CONFIG, \
			}
			
typedef struct _GPSConfig{
	char GPSInstalled;
	float startFinishLatitude;
	float startFinishLongitude;
	float startFinishRadius;
	ChannelConfig lapCountCfg;
	ChannelConfig lapTimeCfg;
	ChannelConfig qualityCfg;
	ChannelConfig satellitesCfg;
	ChannelConfig latitudeCfg;
	ChannelConfig longitudeCfg;
	ChannelConfig timeCfg;
	ChannelConfig velocityCfg;
} GPSConfig;


#define DEFAULT_GPS_POSITION_LOGGING_PRECISION 		6
#define DEFAULT_GPS_VELOCITY_LOGGING_PRECISION 		2
#define DEFAULT_GPS_RADIUS_LOGGING_PRECISION 		5
#define DEFAULT_GPS_TIME_LOGGING_PRECISION 			3
#define DEFAULT_LAP_COUNT_LOGGING_PRECISION			0
#define DEFAULT_LAP_TIME_LOGGING_PRECISION			3
#define	DEFAULT_GPS_QUALITY_LOGGING_PRECISION 		0
#define DEFAULT_GPS_SATELLITES_LOGGING_PRECISION 	0
#define DEFAULT_GPS_START_FINISH_LONGITUDE 			0
#define DEFAULT_GPS_START_FINISH_LATITUDE			0
//currently in degrees. This is about a 73 foot diameter circle (in the pacific NW...)
#define DEFAULT_GPS_START_FINISH_RADIUS				0.0001

#define DEFAULT_LAP_COUNT_CONFIG {"LapCount", "", SAMPLE_DISABLED}
#define DEFAULT_LAP_TIME_CONFIG {"LapTime", "seconds", SAMPLE_DISABLED}
#define DEFAULT_GPS_QUAL_CONFIG {"GpsQual", "", SAMPLE_DISABLED}
#define DEFAULT_GPS_SATELLITES_CONFIG {"GpsSats", "", SAMPLE_DISABLED}
#define DEFAULT_GPS_LATITUDE_CONFIG {"Latitude", "deg", SAMPLE_5Hz}
#define DEFAULT_GPS_LONGITUDE_CONFIG {"Longitude", "deg", SAMPLE_5Hz}
#define DEFAULT_GPS_TIME_CONFIG {"Time", "Time", SAMPLE_5Hz}
#define DEFAULT_GPS_VELOCITY_CONFIG {"Velocity", "kph", SAMPLE_5Hz}

#define DEFAULT_GPS_CONFIG {CONFIG_FEATURE_INSTALLED, \
							DEFAULT_GPS_START_FINISH_LATITUDE, \
							DEFAULT_GPS_START_FINISH_LONGITUDE, \
							DEFAULT_GPS_START_FINISH_RADIUS, \
							DEFAULT_LAP_COUNT_CONFIG, \
							DEFAULT_LAP_TIME_CONFIG, \
							DEFAULT_GPS_QUAL_CONFIG, \
							DEFAULT_GPS_SATELLITES_CONFIG, \
							DEFAULT_GPS_LATITUDE_CONFIG, \
							DEFAULT_GPS_LONGITUDE_CONFIG, \
							DEFAULT_GPS_TIME_CONFIG, \
							DEFAULT_GPS_VELOCITY_CONFIG}

typedef struct _LoggerOutputConfig {
	char telemetryMode;
	char sdLoggingMode;
	unsigned int p2pDestinationAddrHigh;
	unsigned int p2pDestinationAddrLow;
} LoggerOutputConfig;

#define TELEMETRY_MODE_DISABLED 				0
#define TELEMETRY_MODE_P2P 						1
#define TELEMETRY_MODE_CELL						2

#define SD_LOGGING_MODE_DISABLED				0
#define SD_LOGGING_MODE_CSV						1
#define SD_LOGGING_MODE_BINARY					2

#define DEFAULT_TELEMETRY_MODE 					TELEMETRY_MODE_DISABLED
#define DEFAULT_SD_LOGGING_MODE					SD_LOGGING_MODE_CSV

#define DEFAULT_P2P_DESTINATION_ADDR_HIGH	 	0x00000000
#define DEFAULT_P2P_DESTINATION_ADDR_LOW		0X0000FFFF

#define DEFAULT_LOGGER_OUTPUT_CONFIG { 	DEFAULT_TELEMETRY_MODE, \
										DEFAULT_SD_LOGGING_MODE, \
										DEFAULT_P2P_DESTINATION_ADDR_HIGH, \
										DEFAULT_P2P_DESTINATION_ADDR_LOW \
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
	char AccelInstalled;
	AccelConfig AccelConfigs[CONFIG_ACCEL_CHANNELS];
	//GPS Configuration
	GPSConfig GPSConfig;
	//Logger Output Configuration
	LoggerOutputConfig LoggerOutputConfig;
	//Padding data to accommodate flash routine
	char padding_data[AT91C_IFLASH_PAGE_SIZE]; 
} LoggerConfig;

#define DEFAULT_LOGGER_CONFIG \
	{ \
	DEFAULT_ADC_CONFIGS, \
	DEFAULT_PWM_CLOCK_FREQUENCY, \
	DEFAULT_PWM_CONFIGS, \
	DEFAULT_GPIO_CONFIGS, \
	DEFAULT_TIMER_CONFIGS, \
	CONFIG_FEATURE_INSTALLED, \
	DEFAULT_ACCEL_CONFIGS, \
	DEFAULT_GPS_CONFIG, \
	DEFAULT_LOGGER_OUTPUT_CONFIG, \
	"" \
	}
	

int flashLoggerConfig();
void updateActiveLoggerConfig();

LoggerConfig * getSavedLoggerConfig();
LoggerConfig * getWorkingLoggerConfig();

void calculateTimerScaling(TimerConfig *timerConfig);

int encodeSampleRate(int sampleRate);
int decodeSampleRate(int sampleRateCode);


int filterGPIOMode(int config);
int filterPWMOutputConfig(int config);
int filterPWMLoggingConfig(int config);
int filterPWMDutyCycle(int dutyCycle);
int filterAccelRawValue(int accelRawValue);
int filterPWMPeriod(int period);
int filterPWMClockFrequency(int frequency);
int filterTimerMode(int config);
unsigned short filterTimerDivider(unsigned short divider);
int filterAccelMode(int mode);
int filterAccelChannel(int channel);

TimerConfig * getTimerConfigChannel(int channel);
ADCConfig * getADCConfigChannel(int channel);
PWMConfig * getPWMConfigChannel(int channel);
GPIOConfig * getGPIOConfigChannel(int channel);
AccelConfig * getAccelConfigChannel(int channel);

void setLabelGeneric(char *dest, const char *source);

void calibrateAccelZero();

int getHighestSampleRate(LoggerConfig *config);

#endif /*LOGGERCONFIG_H_*/
