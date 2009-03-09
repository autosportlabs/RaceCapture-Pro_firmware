#ifndef LOGGERCONFIG_H_
#define LOGGERCONFIG_H_

#include "AT91SAM7S256.h"


#define CONFIG_ADC_CHANNELS			8
#define CONFIG_ACCEL_CHANNELS		4
#define CONFIG_GPIO_CHANNELS		3
#define CONFIG_PWM_CHANNELS			4
#define CONFIG_TIMER_CHANNELS		3

#define DEFAULT_LABEL_LENGTH		11
#define DEFAULT_UNITS_LENGTH		11
#define	CONFIG_GPIO_IN  			0
#define CONFIG_GPIO_OUT  			1	

#define CONFIG_PWM_ANALOG  			0
#define	CONFIG_PWM_FREQUENCY  		1

#define CONFIG_ACCEL_CHANNEL_X		0
#define CONFIG_ACCEL_CHANNEL_Y		1
#define CONFIG_ACCEL_CHANNEL_Z		2
#define	CONFIG_ACCEL_CHANNEL_ZT		3

#define CONFIG_ACCEL_DISABLED  		0
#define CONFIG_ACCEL_NORMAL  		1
#define CONFIG_ACCEL_INVERTED  		2

#define CONFIG_FEATURE_INSTALLED		1
#define CONFIG_FEATURE_NOT_INSTALLED	0

#define CONFIG_TIMER_RPM			0
#define CONFIG_TIMER_FREQUENCY		1

#define SAMPLE_100Hz 3
#define SAMPLE_50Hz 6
#define SAMPLE_30Hz 10
#define SAMPLE_20Hz 15
#define SAMPLE_10Hz 30
#define SAMPLE_5Hz 60
#define SAMPLE_1Hz 300
#define SAMPLE_DISABLED 0

void updateActiveLoggerConfig();

int flashLoggerConfig();

struct LoggerConfig * getSavedLoggerConfig();
struct LoggerConfig * getWorkingLoggerConfig();

struct ADCConfig{
	char label[DEFAULT_LABEL_LENGTH];
	char units[DEFAULT_UNITS_LENGTH];
	char sampleRate;
	float scaling;
};

#define DEFAULT_ADC0_CONFIG {"Analog1","Units",SAMPLE_10Hz,0.0048875f}
#define DEFAULT_ADC1_CONFIG {"Analog2","Units",SAMPLE_10Hz,0.0048875f}
#define DEFAULT_ADC2_CONFIG {"Analog3","Units",SAMPLE_10Hz,0.0048875f}
#define DEFAULT_ADC3_CONFIG {"Analog4","Units",SAMPLE_10Hz,0.0048875f}
#define DEFAULT_ADC4_CONFIG {"Analog5","Units",SAMPLE_10Hz,0.0048875f}
#define DEFAULT_ADC5_CONFIG {"Analog6","Units",SAMPLE_10Hz,0.0048875f}
#define DEFAULT_ADC6_CONFIG {"Analog7","Units",SAMPLE_10Hz,0.0048875f}	
#define BATTERY_ADC7_CONFIG {"Battery","Volts",SAMPLE_10Hz,0.01955f}
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

struct TimerConfig{
	char label[DEFAULT_LABEL_LENGTH];
	char units[DEFAULT_UNITS_LENGTH];
	char sampleRate;
	char config;
	float scaling;
};

#define DEFAULT_RPM_TIMER_CONFIG {"EngineRPM","RPM", SAMPLE_10Hz,CONFIG_TIMER_RPM, 0.0000015f}
#define DEFAULT_FREQUENCY2_CONFIG {"Freq2","Hz", SAMPLE_10Hz, CONFIG_TIMER_FREQUENCY, 0.0000015f}
#define DEFAULT_FREQUENCY3_CONFIG {"Freq3","Hz", SAMPLE_10Hz, CONFIG_TIMER_FREQUENCY, 0.0000015f}
#define DEFAULT_TIMER_CONFIGS \
			{ \
			DEFAULT_RPM_TIMER_CONFIG, \
			DEFAULT_FREQUENCY2_CONFIG, \
			DEFAULT_FREQUENCY3_CONFIG \
			}

struct GPIOConfig{
	char label[DEFAULT_LABEL_LENGTH];
	char sampleRate;
	char config;	
};

#define DEFAULT_GPIO1_CONFIG {"GPIO1", SAMPLE_10Hz, CONFIG_GPIO_IN}
#define DEFAULT_GPIO2_CONFIG {"GPIO2", SAMPLE_10Hz, CONFIG_GPIO_IN}
#define DEFAULT_GPIO3_CONFIG {"GPIO3", SAMPLE_10Hz, CONFIG_GPIO_IN}
#define DEFAULT_GPIO_CONFIGS \
			{ \
			DEFAULT_GPIO1_CONFIG, \
			DEFAULT_GPIO2_CONFIG, \
			DEFAULT_GPIO3_CONFIG \
			}
			
struct AccelConfig{
	char label[DEFAULT_LABEL_LENGTH];
	char sampleRate;
	char config;
	char accelChannel;
};

#define DEFAULT_ACCEL_X_AXIS_CONFIG {"AccelX", SAMPLE_10Hz, CONFIG_ACCEL_NORMAL, CONFIG_ACCEL_CHANNEL_X}
#define DEFAULT_ACCEL_Y_AXIS_CONFIG {"AccelY", SAMPLE_10Hz, CONFIG_ACCEL_NORMAL, CONFIG_ACCEL_CHANNEL_Y}
#define DEFAULT_ACCEL_Z_AXIS_CONFIG {"AccelZ", SAMPLE_10Hz, CONFIG_ACCEL_NORMAL, CONFIG_ACCEL_CHANNEL_Z}
#define DEFAULT_ACCEL_ZT_AXIS_CONFIG {"Yaw", SAMPLE_10Hz, CONFIG_ACCEL_NORMAL, CONFIG_ACCEL_CHANNEL_ZT}
#define DEFAULT_ACCEL_CONFIGS \
			{ \
				DEFAULT_ACCEL_X_AXIS_CONFIG, \
				DEFAULT_ACCEL_Y_AXIS_CONFIG, \
				DEFAULT_ACCEL_Z_AXIS_CONFIG, \
				DEFAULT_ACCEL_ZT_AXIS_CONFIG \
			}
	
struct PWMConfig{
	char label[DEFAULT_LABEL_LENGTH];
	char sampleRate;
	char config;	
};

#define DEFAULT_PWM1_CONFIG {"Vout1",SAMPLE_10Hz,CONFIG_PWM_ANALOG}
#define DEFAULT_PWM2_CONFIG {"Vout2",SAMPLE_10Hz,CONFIG_PWM_ANALOG}
#define DEFAULT_PWM3_CONFIG {"Vout3",SAMPLE_10Hz,CONFIG_PWM_ANALOG}
#define DEFAULT_PWM4_CONFIG {"Vout4",SAMPLE_10Hz,CONFIG_PWM_ANALOG}
#define DEFAULT_PWM_CONFIGS \
			{ \
				DEFAULT_PWM1_CONFIG, \
				DEFAULT_PWM2_CONFIG, \
				DEFAULT_PWM3_CONFIG, \
				DEFAULT_PWM4_CONFIG, \
			}
			
struct GPSConfig{
	char qualityLabel[DEFAULT_LABEL_LENGTH];
	char satsLabel[DEFAULT_LABEL_LENGTH];
	char latitiudeLabel[DEFAULT_LABEL_LENGTH];
	char longitudeLabel[DEFAULT_LABEL_LENGTH];
	char timeLabel[DEFAULT_LABEL_LENGTH];
	char velocityLabel[DEFAULT_LABEL_LENGTH];
	
	char positionSampleRate;
	char velocitySampleRate;
	char timeSampleRate;
};
#define DEFAULT_GPS_CONFIG {"GPS_Qual", "GPS_Sats", "Latitude", "Longitude", "UTCTime", "kph", SAMPLE_5Hz, SAMPLE_5Hz, SAMPLE_5Hz}

struct LoggerConfig {
	//ADC Calibrations
	struct ADCConfig ADCConfigs[CONFIG_ADC_CHANNELS];
	//PWM/Analog out configurations
	struct PWMConfig PWMConfig[CONFIG_PWM_CHANNELS];
	//GPIO configurations
	struct GPIOConfig GPIOConfigs[CONFIG_GPIO_CHANNELS];
	//Timer Configurations
	struct TimerConfig TimerConfigs[CONFIG_TIMER_CHANNELS];
	//Accelerometer Configurations
	char AccelInstalled;
	struct AccelConfig AccelConfig[CONFIG_ACCEL_CHANNELS];
	//GPS Configuration
	char GPSInstalled;
	struct GPSConfig GPSConfig;
	//Padding data to accomodate flash routine
	char padding_data[AT91C_IFLASH_PAGE_SIZE]; 
};

#define DEFAULT_LOGGER_CONFIG \
	{ \
	DEFAULT_ADC_CONFIGS, \
	DEFAULT_PWM_CONFIGS, \
	DEFAULT_GPIO_CONFIGS, \
	DEFAULT_TIMER_CONFIGS, \
	CONFIG_FEATURE_INSTALLED, \
	DEFAULT_ACCEL_CONFIGS, \
	CONFIG_FEATURE_INSTALLED, \
	DEFAULT_GPS_CONFIG, \
	"" \
	}
			
#endif /*LOGGERCONFIG_H_*/
