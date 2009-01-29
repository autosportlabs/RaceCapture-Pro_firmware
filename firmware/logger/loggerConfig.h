#ifndef LOGGERCONFIG_H_
#define LOGGERCONFIG_H_

#define	CONFIG_GPIO_IN  		0
#define CONFIG_GPIO_OUT  		1	

#define CONFIG_PWM_ANALOG  		0
#define	CONFIG_PWM_FREQUENCY  	1

#define CONFIG_ACCEL_DISABLED  	0
#define CONFIG_ACCEL_NORMAL  	1
#define CONFIG_ACCEL_INVERTED  	2


void updateActiveLoggerConfig();

int flashLoggerConfig();

struct ADCCalibration{
	char label[51];
	char units[51];
};

struct LoggerConfig {
	//ADC Calibrations	
	char ADC1_cal_id;
	char ADC2_cal_id;
	char ADC3_cal_id;
	char ADC4_cal_id;
	char ADC5_cal_id;
	char ADC6_cal_id;
	char ADC7_cal_id;
	char ADC8_cal_id;
	//PWM/Analog out configurations
	char PWM1_config;
	char PWM2_config;
	char PWM3_config;
	char PWM4_config;
	//GPIO configurations
	char GPIO1_config;
	char GPIO2_config;
	char GPIO3_config;
	//Accelerometer Configurations
	char AccelX_config;
	char AccelY_config;
	char AccelZ_config;
	char ThetaZ_config;
	//GPS Configuration
	char GPS_position_enabled;
	char GPS_velocity_enabled;
	char GPS_time_enabled;

	char extra1;
	char extra2;
	char extra3;
	char extra4;
	char extra5;
	char extra6;
	char extra7;
	char extra8;
	char extra9;
	char extra10;
};


#define DEFAULT_LOGGER_CONFIG \
		{	\
			0, \
			1, \
			2, \
			3, \
			4, \
			5, \
			6, \
			7, \
			CONFIG_PWM_ANALOG, \
			CONFIG_PWM_ANALOG, \
			CONFIG_PWM_ANALOG, \
			CONFIG_PWM_ANALOG, \
			CONFIG_GPIO_IN, \
			CONFIG_GPIO_IN, \
			CONFIG_GPIO_IN, \
			CONFIG_ACCEL_DISABLED, \
			CONFIG_ACCEL_DISABLED, \
			CONFIG_ACCEL_DISABLED, \
			CONFIG_ACCEL_DISABLED, \
			0, \
			0, \
			0, \
			0, \
			1, \
			2, \
			3, \
			4, \
			5, \
			6, \
			7, \
			8, \
			9 \
	}
			
#endif /*LOGGERCONFIG_H_*/
