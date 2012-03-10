#ifndef LOGGERCONFIG_H_
#define LOGGERCONFIG_H_

#include "board.h"


#define CONFIG_ADC_CHANNELS					8
#define CONFIG_ACCEL_CHANNELS				4
#define CONFIG_GPIO_CHANNELS				3
#define CONFIG_PWM_CHANNELS					4
#define CONFIG_TIMER_CHANNELS				3

#define DEFAULT_ADC_LOGGING_PRECISION		2
#define DEFAULT_LABEL_LENGTH				11
#define DEFAULT_UNITS_LENGTH				11
#define	CONFIG_GPIO_IN  					0
#define CONFIG_GPIO_OUT  					1	

/// PWM frequency in Hz.
#define MAX_PWM_CLOCK_FREQUENCY             2000
#define MIN_PWM_CLOCK_FREQUENCY				10

/// Maximum duty cycle value.
#define MAX_DUTY_CYCLE              		100
#define MIN_DUTY_CYCLE          			1

//TODO: FIX PERIOD/FREQ MIN-MAX
#define MIN_PWM_PERIOD						1
#define MAX_PWM_PERIOD						2000

#define MIN_ACCEL_RAW						0
#define MAX_ACCEL_RAW						4097

#define CONFIG_PWM_ANALOG  					0
#define	CONFIG_PWM_FREQUENCY  				1

#define CONFIG_ACCEL_CHANNEL_X				0
#define CONFIG_ACCEL_CHANNEL_Y				1
#define CONFIG_ACCEL_CHANNEL_Z				2
#define	CONFIG_ACCEL_CHANNEL_ZT				3

#define DEFAULT_ACCEL_LOGGING_PRECISION		3
#define DEFAULT_ACCEL_ZERO					2047
#define DEFAULT_PWM_CLOCK_FREQUENCY			100

#define CONFIG_ACCEL_DISABLED  				0
#define CONFIG_ACCEL_NORMAL  				1
#define CONFIG_ACCEL_INVERTED  				2

#define CONFIG_FEATURE_INSTALLED			1
#define CONFIG_FEATURE_NOT_INSTALLED		0

#define CONFIG_LOGGING_TIMER_RPM			0
#define CONFIG_LOGGING_TIMER_FREQUENCY		1
#define CONFIG_LOGGING_TIMER_PERIOD_USEC	2
#define CONFIG_LOGGING_TIMER_PERIOD_MS		3

#define CONFIG_LOGGING_PWM_PERIOD			0
#define CONFIG_LOGGING_PWM_DUTY				1
#define	CONFIG_LOGGING_PWM_VOLTS			2

#define DEFAULT_GPS_POSITION_LOGGING_PRECISION 	6
#define DEFAULT_GPS_VELOCITY_LOGGING_PRECISION 	2
#define DEFAULT_GPS_RADIUS_LOGGING_PRECISION 	2
#define DEFAULT_GPS_TIME_LOGGING_PRECISION 3
#define DEFAULT_GPS_START_FINISH_LONGITUDE 	0
#define DEFAULT_GPS_START_FINISH_LATITUDE	0
#define DEFAULT_GPS_START_FINISH_RADIUS		50
#define SAMPLE_100Hz 						3
#define SAMPLE_50Hz 						6
#define SAMPLE_30Hz 						10
#define SAMPLE_20Hz 						15
#define SAMPLE_10Hz 						30
#define SAMPLE_5Hz 							60
#define SAMPLE_1Hz 							300
#define SAMPLE_DISABLED 					0

#define DEFAULT_PWM_VOLTAGE_SCALING			0.1
// MCK: 48054840 Hz
// /2 = 24027420
// /8 = 6006855
// /32 = 1501713.75
// /128 = 375428.4375
// /1024 = 46928.5546875
#define MCK_2 		2
#define MCK_8 		8
#define MCK_32 		32
#define MCK_128 	128
#define MCK_1024 	1024

typedef struct _ChannelConfig{
	int precision;
	char label[DEFAULT_LABEL_LENGTH];
	char units[DEFAULT_UNITS_LENGTH];
	int sampleRate;
} ChannelConfig;

typedef struct _ADCConfig{
	ChannelConfig cfg;
	float scaling;
} ADCConfig;

#define DEFAULT_ADC0_CONFIG {{DEFAULT_ADC_LOGGING_PRECISION,"Analog1","Units",SAMPLE_DISABLED},0.0048875f}
#define DEFAULT_ADC1_CONFIG {{DEFAULT_ADC_LOGGING_PRECISION,"Analog2","Units",SAMPLE_DISABLED},0.0048875f}
#define DEFAULT_ADC2_CONFIG {{DEFAULT_ADC_LOGGING_PRECISION,"Analog3","Units",SAMPLE_DISABLED},0.0048875f}
#define DEFAULT_ADC3_CONFIG {{DEFAULT_ADC_LOGGING_PRECISION,"Analog4","Units",SAMPLE_DISABLED},0.0048875f}
#define DEFAULT_ADC4_CONFIG {{DEFAULT_ADC_LOGGING_PRECISION,"Analog5","Units",SAMPLE_DISABLED},0.0048875f}
#define DEFAULT_ADC5_CONFIG {{DEFAULT_ADC_LOGGING_PRECISION,"Analog6","Units",SAMPLE_DISABLED},0.0048875f}
#define DEFAULT_ADC6_CONFIG {{DEFAULT_ADC_LOGGING_PRECISION,"Analog7","Units",SAMPLE_DISABLED},0.0048875f}
#define BATTERY_ADC7_CONFIG {{DEFAULT_ADC_LOGGING_PRECISION,"Battery","Volts",SAMPLE_DISABLED},0.01955f}
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
	char slowTimerEnabled;
	char config;
	char pulsePerRevolution;
	unsigned short timerDivider;
	unsigned int calculatedScaling;
} TimerConfig;

#define DEFAULT_RPM_TIMER_CONFIG  {{0,"EngineRPM", "RPM", SAMPLE_DISABLED}, 0, CONFIG_LOGGING_TIMER_RPM, 1, MCK_128, 375428}
#define DEFAULT_FREQUENCY2_CONFIG {{0,"Freq2","Hz", SAMPLE_DISABLED}, CONFIG_LOGGING_TIMER_FREQUENCY, 1, MCK_128, 375428}
#define DEFAULT_FREQUENCY3_CONFIG {{0,"Freq3","Hz", SAMPLE_DISABLED}, 0, CONFIG_LOGGING_TIMER_FREQUENCY, 1, MCK_128, 375428}
#define DEFAULT_TIMER_CONFIGS \
			{ \
			DEFAULT_RPM_TIMER_CONFIG, \
			DEFAULT_FREQUENCY2_CONFIG, \
			DEFAULT_FREQUENCY3_CONFIG \
			}

typedef struct _GPIOConfig{
	ChannelConfig cfg;
	char config;
} GPIOConfig;

#define DEFAULT_GPIO1_CONFIG {{0,"GPIO1", "", SAMPLE_DISABLED}, CONFIG_GPIO_IN}
#define DEFAULT_GPIO2_CONFIG {{0,"GPIO2", "", SAMPLE_DISABLED}, CONFIG_GPIO_IN}
#define DEFAULT_GPIO3_CONFIG {{0,"GPIO3", "", SAMPLE_DISABLED}, CONFIG_GPIO_IN}
#define DEFAULT_GPIO_CONFIGS \
			{ \
			DEFAULT_GPIO1_CONFIG, \
			DEFAULT_GPIO2_CONFIG, \
			DEFAULT_GPIO3_CONFIG \
			}
			
typedef struct _AccelConfig{
	ChannelConfig cfg;
	char config;
	unsigned char accelChannel;
	unsigned long zeroValue;
} AccelConfig;

#define DEFAULT_ACCEL_X_AXIS_CONFIG  {{DEFAULT_ACCEL_LOGGING_PRECISION,"AccelX", "G", SAMPLE_30Hz}, CONFIG_ACCEL_NORMAL, CONFIG_ACCEL_CHANNEL_X,DEFAULT_ACCEL_ZERO}
#define DEFAULT_ACCEL_Y_AXIS_CONFIG  {{DEFAULT_ACCEL_LOGGING_PRECISION,"AccelY", "G", SAMPLE_30Hz}, CONFIG_ACCEL_NORMAL, CONFIG_ACCEL_CHANNEL_Y,DEFAULT_ACCEL_ZERO}
#define DEFAULT_ACCEL_Z_AXIS_CONFIG  {{DEFAULT_ACCEL_LOGGING_PRECISION,"AccelZ", "G", SAMPLE_30Hz}, CONFIG_ACCEL_NORMAL, CONFIG_ACCEL_CHANNEL_Z,DEFAULT_ACCEL_ZERO}
#define DEFAULT_ACCEL_ZT_AXIS_CONFIG {{DEFAULT_ACCEL_LOGGING_PRECISION,"Yaw", "Deg/Sec", SAMPLE_30Hz}, CONFIG_ACCEL_NORMAL, CONFIG_ACCEL_CHANNEL_ZT,DEFAULT_ACCEL_ZERO}
#define DEFAULT_ACCEL_CONFIGS \
			{ \
				DEFAULT_ACCEL_X_AXIS_CONFIG, \
				DEFAULT_ACCEL_Y_AXIS_CONFIG, \
				DEFAULT_ACCEL_Z_AXIS_CONFIG, \
				DEFAULT_ACCEL_ZT_AXIS_CONFIG \
			}
	
typedef struct _PWMConfig{
	ChannelConfig cfg;
	char outputConfig;
	char loggingConfig;
	unsigned short startupDutyCycle;
	unsigned short startupPeriod;
	float voltageScaling;
} PWMConfig;

#define DEFAULT_PWM1_CONFIG {{2,"Vout1","V",SAMPLE_DISABLED},CONFIG_PWM_ANALOG,CONFIG_LOGGING_PWM_VOLTS,50,100,DEFAULT_PWM_VOLTAGE_SCALING}
#define DEFAULT_PWM2_CONFIG {{2,"Vout2","V",SAMPLE_DISABLED},CONFIG_PWM_ANALOG,CONFIG_LOGGING_PWM_PERIOD,50,100,DEFAULT_PWM_VOLTAGE_SCALING}
#define DEFAULT_PWM3_CONFIG {{2,"Vout3","V",SAMPLE_DISABLED},CONFIG_PWM_ANALOG,CONFIG_LOGGING_PWM_DUTY,50,100,DEFAULT_PWM_VOLTAGE_SCALING}
#define DEFAULT_PWM4_CONFIG {{2,"Vout4","V",SAMPLE_DISABLED},CONFIG_PWM_ANALOG,CONFIG_LOGGING_PWM_PERIOD,50,100,DEFAULT_PWM_VOLTAGE_SCALING}
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
	ChannelConfig qualityCfg;
	ChannelConfig satellitesCfg;
	ChannelConfig latitudeCfg;
	ChannelConfig longitudeCfg;
	ChannelConfig timeCfg;
	ChannelConfig velocityCfg;
} GPSConfig;

#define DEFAULT_GPS_QUAL_CONFIG {0, "GPS_Qual", "",SAMPLE_DISABLED}
#define DEFAULT_GPS_SATELLITES_CONFIG {0, "GPS_Sats", "", SAMPLE_DISABLED}
#define DEFAULT_GPS_LATITUDE_CONFIG {DEFAULT_GPS_POSITION_LOGGING_PRECISION, "Latitude", "deg", SAMPLE_5Hz}
#define DEFAULT_GPS_LONGITUDE_CONFIG {DEFAULT_GPS_POSITION_LOGGING_PRECISION, "Longitude", "deg", SAMPLE_5Hz}
#define DEFAULT_GPS_TIME_CONFIG {DEFAULT_GPS_TIME_LOGGING_PRECISION, "UTCTIme", "Time", SAMPLE_5Hz}
#define DEFAULT_GPS_VELOCITY_CONFIG {DEFAULT_GPS_VELOCITY_LOGGING_PRECISION, "Velocity", "kph", SAMPLE_5Hz}
#define DEFAULT_GPS_CONFIG {CONFIG_FEATURE_INSTALLED, DEFAULT_GPS_START_FINISH_LATITUDE, DEFAULT_GPS_START_FINISH_LONGITUDE, DEFAULT_GPS_START_FINISH_RADIUS, DEFAULT_GPS_QUAL_CONFIG, DEFAULT_GPS_SATELLITES_CONFIG, DEFAULT_GPS_LATITUDE_CONFIG, DEFAULT_GPS_LONGITUDE_CONFIG, DEFAULT_GPS_TIME_CONFIG, DEFAULT_GPS_VELOCITY_CONFIG}

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
	"" \
	}
	

int flashLoggerConfig();
void updateActiveLoggerConfig();

LoggerConfig * getSavedLoggerConfig();
LoggerConfig * getWorkingLoggerConfig();

void calculateTimerScaling(TimerConfig *timerConfig);

int encodeSampleRate(int sampleRate);
int decodeSampleRate(int sampleRateCode);


int filterGPIOConfig(int config);
int filterPWMOutputConfig(int config);
int filterPWMLoggingConfig(int config);
int filterPWMDutyCycle(int dutyCycle);
int filterAccelRawValue(int accelRawValue);
int filterPWMPeriod(int period);
int filterPWMClockFrequency(int frequency);
int filterTimerConfig(int config);
unsigned short filterTimerDivider(unsigned short divider);
int filterAccelConfig(int config);
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
