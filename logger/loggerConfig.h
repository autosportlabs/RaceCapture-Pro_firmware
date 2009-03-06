#ifndef LOGGERCONFIG_H_
#define LOGGERCONFIG_H_

#define	CONFIG_GPIO_IN  			0
#define CONFIG_GPIO_OUT  			1	

#define CONFIG_PWM_ANALOG  			0
#define	CONFIG_PWM_FREQUENCY  		1

#define CONFIG_ACCEL_DISABLED  		0
#define CONFIG_ACCEL_NORMAL  		1
#define CONFIG_ACCEL_INVERTED  		2

#define CONFIG_ACCEL_INSTALLED		1
#define CONFIG_ACCEL_NOT_INSTALLED	0

void updateActiveLoggerConfig();

int flashLoggerConfig();


struct LoggerConfig * getSavedLoggerConfig();
struct LoggerConfig * getWorkingLoggerConfig();

struct ADCCalibration{
	char label[10];
	char units[10];
	float scaling;
};

struct LoggerConfig {
	//ADC Calibrations
	char ADC_Sample_Rate;
	struct ADCCalibration ADC_Calibrations[8];
	//PWM/Analog out configurations
	char PWM1_config;
	char PWM2_config;
	char PWM3_config;
	char PWM4_config;
	//GPIO configurations
	char GPI_Sample_Rate;
	char GPIO1_config;
	char GPIO2_config;
	char GPIO3_config;
	//Accelerometer Configurations
	char Accel_Installed;
	char Accel_Sample_Rate;
	char AccelX_config;
	char AccelY_config;
	char AccelZ_config;
	char ThetaZ_config;
	//GPS Configuration
	char GPS_position_enabled;
	char GPS_velocity_enabled;
	char GPS_time_enabled;
	char extra2[44];
};

#define DEFAULT_ADC_CALIBRATION {"Analog","Units",0.0048875f}	
#define BATTERY_ADC_CALIBRATION {"Battery","Volts",.01955f}
	
#define DEFAULT_ADC_CALIBRATIONS {DEFAULT_ADC_CALIBRATION, DEFAULT_ADC_CALIBRATION, DEFAULT_ADC_CALIBRATION, DEFAULT_ADC_CALIBRATION, DEFAULT_ADC_CALIBRATION, DEFAULT_ADC_CALIBRATION, DEFAULT_ADC_CALIBRATION, BATTERY_ADC_CALIBRATION}


#define DEFAULT_LOGGER_CONFIG \
		{	\
			10, \
			DEFAULT_ADC_CALIBRATIONS, \
			CONFIG_PWM_ANALOG, \
			CONFIG_PWM_ANALOG, \
			CONFIG_PWM_ANALOG, \
			CONFIG_PWM_ANALOG, \
			10, \
			CONFIG_GPIO_IN, \
			CONFIG_GPIO_IN, \
			CONFIG_GPIO_IN, \
			CONFIG_ACCEL_INSTALLED, \
			10, \
			CONFIG_ACCEL_NORMAL, \
			CONFIG_ACCEL_NORMAL, \
			CONFIG_ACCEL_NORMAL, \
			CONFIG_ACCEL_DISABLED, \
			0, \
			0, \
			0, \
			"" \
	}
			
#endif /*LOGGERCONFIG_H_*/
