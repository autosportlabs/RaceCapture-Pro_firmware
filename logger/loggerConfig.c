#include "loggerConfig.h"
#include "mod_string.h"

#ifndef RCP_TESTING
#include "memory.h"
LoggerConfig g_savedLoggerConfig __attribute__ ((aligned (MEMORY_PAGE_SIZE))) __attribute__((section(".text\n\t#"))) = DEFAULT_LOGGER_CONFIG;
#else
LoggerConfig g_savedLoggerConfig = DEFAULT_LOGGER_CONFIG;
#endif

LoggerConfig g_workingLoggerConfig;


void updateActiveLoggerConfig(){
	memcpy(&g_workingLoggerConfig,&g_savedLoggerConfig,sizeof(LoggerConfig));
}

LoggerConfig * getSavedLoggerConfig(){
	return &g_savedLoggerConfig;	
}

LoggerConfig * getWorkingLoggerConfig(){
	return &g_workingLoggerConfig;
}

void calculateTimerScaling(unsigned int clockHz, TimerConfig *timerConfig){
	unsigned int clock = clockHz / timerConfig->timerDivider;
	clock = clock / timerConfig->pulsePerRevolution;
	timerConfig->calculatedScaling = clock;
}

int getConnectivitySampleRateLimit(){
	switch(getWorkingLoggerConfig()->ConnectivityConfigs.connectivityMode){
		case CONNECTIVITY_MODE_BLUETOOTH:
			return FAST_LINK_MAX_TELEMETRY_SAMPLE_RATE;
		case CONNECTIVITY_MODE_CELL:
		default:
			return SLOW_LINK_MAX_TELEMETRY_SAMPLE_RATE;
	}
}

int encodeSampleRate(int sampleRate){

	switch(sampleRate){
		case 200:
			return SAMPLE_200Hz;
		case 100:
			return SAMPLE_100Hz;
		case 50:
			return SAMPLE_50Hz;
		case 30:
			return SAMPLE_30Hz;
		case 20:
			return SAMPLE_20Hz;
		case 10:
			return SAMPLE_10Hz;
		case 5:
			return SAMPLE_5Hz;
		case 1:
			return SAMPLE_1Hz;
		default:
		case 0:
			return SAMPLE_DISABLED;
	}
}

int decodeSampleRate(int sampleRateCode){

	switch(sampleRateCode){
		case SAMPLE_200Hz:
			return 200;
		case SAMPLE_100Hz:
			return 100;
		case SAMPLE_50Hz:
			return 50;
		case SAMPLE_30Hz:
			return 30;
		case SAMPLE_20Hz:
			return 20;
		case SAMPLE_10Hz:
			return 10;
		case SAMPLE_5Hz:
			return 5;
		case SAMPLE_1Hz:
			return 1;
		default:
		case SAMPLE_DISABLED:
			return 0;
	}
}

char filterAnalogScalingMode(char mode){
	switch(mode){
		case SCALING_MODE_LINEAR:
			return SCALING_MODE_LINEAR;
		case SCALING_MODE_MAP:
			return SCALING_MODE_MAP;
		default:
		case SCALING_MODE_RAW:
			return SCALING_MODE_RAW;
	}
}

char filterSdLoggingMode(char mode){
	switch (mode){
		case SD_LOGGING_MODE_CSV:
			return SD_LOGGING_MODE_CSV;
		default:
		case SD_LOGGING_MODE_DISABLED:
			return SD_LOGGING_MODE_DISABLED;
	}
}

char filterConnectivityMode(char mode){
	switch(mode){
		case CONNECTIVITY_MODE_CELL:
			return CONNECTIVITY_MODE_CELL;
		case CONNECTIVITY_MODE_BLUETOOTH:
			return CONNECTIVITY_MODE_BLUETOOTH;
		default:
		case CONNECTIVITY_MODE_CONSOLE:
			return CONNECTIVITY_MODE_CONSOLE;
	}
}

char filterGpioMode(int value){
	switch(value){
		case CONFIG_GPIO_OUT:
			return CONFIG_GPIO_OUT;
		case CONFIG_GPIO_IN:
		default:
			return CONFIG_GPIO_IN;	
	}	
}

char filterPwmOutputMode(int value){
	switch(value){
		case MODE_PWM_ANALOG:
			return MODE_PWM_ANALOG;
		case MODE_PWM_FREQUENCY:
		default:
			return MODE_PWM_FREQUENCY;
	}
}

char filterPwmLoggingMode(int config){
	switch (config){
		case MODE_LOGGING_PWM_PERIOD:
			return MODE_LOGGING_PWM_PERIOD;
		case MODE_LOGGING_PWM_DUTY:
			return MODE_LOGGING_PWM_DUTY;
		case MODE_LOGGING_PWM_VOLTS:
		default:
			return MODE_LOGGING_PWM_VOLTS;
	}
}

unsigned short filterTimerDivider(unsigned short divider){
	switch(divider){
	case TIMER_MCK_2:
		return TIMER_MCK_2;
	case TIMER_MCK_8:
		return TIMER_MCK_8;
	case TIMER_MCK_32:
		return TIMER_MCK_32;
	case TIMER_MCK_128:
		return TIMER_MCK_128;
	case TIMER_MCK_1024:
		return TIMER_MCK_1024;
	default:
		return TIMER_MCK_128;
	}
}
char filterTimerMode(int mode){
	switch (mode){
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

int filterAccelChannel(int config){
	switch(config){
		case ACCEL_CHANNEL_Y:
			return ACCEL_CHANNEL_Y;
		case ACCEL_CHANNEL_Z:
			return ACCEL_CHANNEL_Z;
		case ACCEL_CHANNEL_ZT:
			return ACCEL_CHANNEL_ZT;
		default:
		case ACCEL_CHANNEL_X:
			return ACCEL_CHANNEL_X;
	}
}	

int filterAccelRawValue(int accelRawValue){
	if (accelRawValue > MAX_ACCEL_RAW){
		accelRawValue = MAX_ACCEL_RAW;
	} else if (accelRawValue < MIN_ACCEL_RAW){
		accelRawValue = MIN_ACCEL_RAW;	
	}
	return accelRawValue;
}

int filterAccelMode(int mode){
	switch (mode){
		case MODE_ACCEL_DISABLED:
			return MODE_ACCEL_DISABLED;
		case MODE_ACCEL_INVERTED:
			return MODE_ACCEL_INVERTED;
		default:
		case MODE_ACCEL_NORMAL:
			return MODE_ACCEL_NORMAL;
	}
}

unsigned short filterPwmDutyCycle(int dutyCycle){
	if (dutyCycle > MAX_PWM_DUTY_CYCLE){
		dutyCycle = MAX_PWM_DUTY_CYCLE;
	} else if (dutyCycle < MIN_PWM_DUTY_CYCLE){
		dutyCycle = MIN_PWM_DUTY_CYCLE;
	}
	return dutyCycle;
}

unsigned short filterPwmPeriod(int period){
	if (period > MAX_PWM_PERIOD){
		period = MAX_PWM_PERIOD;
	} else if (period < MIN_PWM_PERIOD){
		period = MIN_PWM_PERIOD;		
	}
	return period;		
}

int filterPwmClockFrequency(int freq){
	if (freq > MAX_PWM_CLOCK_FREQUENCY){
		freq = MAX_PWM_CLOCK_FREQUENCY;
	} else if (freq < MIN_PWM_CLOCK_FREQUENCY){
		freq = MIN_PWM_CLOCK_FREQUENCY;
	}
	return freq;
}

PWMConfig * getPwmConfigChannel(int channel){
	PWMConfig * c = NULL;
	if (channel >= 0 && channel < CONFIG_PWM_CHANNELS){
		c = &(getWorkingLoggerConfig()->PWMConfigs[channel]);	
	}
	return c;
}

TimerConfig * getTimerConfigChannel(int channel){
	TimerConfig * c = NULL;
	if (channel >=0 && channel < CONFIG_TIMER_CHANNELS){
		c = &(getWorkingLoggerConfig()->TimerConfigs[channel]);
	}
	return c;
}

ADCConfig * getADCConfigChannel(int channel){
	ADCConfig *c = NULL;
	if (channel >=0 && channel < CONFIG_ADC_CHANNELS){
		c = &(getWorkingLoggerConfig()->ADCConfigs[channel]);		
	}
	return c;
}

GPIOConfig * getGPIOConfigChannel(int channel){
	GPIOConfig *c = NULL;
	if (channel >=0 && channel < CONFIG_GPIO_CHANNELS){
		c = &(getWorkingLoggerConfig()->GPIOConfigs[channel]);	
	}
	return c;	
}

AccelConfig * getAccelConfigChannel(int channel){
	AccelConfig * c = NULL;
	if (channel >= 0 && channel < CONFIG_ACCEL_CHANNELS){
		c = &(getWorkingLoggerConfig()->AccelConfigs[channel]);
	}
	return c;		
}

void setTextField(char *dest, const char *source, unsigned int maxlen){
	const char *from = source;
	if (*from == '"') from++;
	int len = strlen(from);
	if (*(from + len -1) == '"') len--;
	if (len > maxlen) len = maxlen;
	for (size_t i = 0; i < len; i++){
		char c = from[i];
		if (! ( (c >= '0' && c <= '9') ||
				(c >= 'A' && c <= 'Z') ||
				(c >= 'a' && c <= 'z') ||
				c == '_' ||
				c == '-' ||
				c == '@' ||
				c == '%' ||
				c == '+' ||
				c == ':' ||
				c == '.' ) ){
			c = '_';
		}
		dest[i] = c;
	}
	dest[len] = '\0';
}

void setLabelGeneric(char *dest, const char *source){
	setTextField(dest, source, DEFAULT_LABEL_LENGTH);
}


unsigned int getHighestSampleRate(LoggerConfig *config){

	//start with the slowest sample rate
	int s = SAMPLE_1Hz;
	//find the fastest sample rate
	for (int i = 0; i < CONFIG_ADC_CHANNELS; i++){
		int sr = config->ADCConfigs[i].cfg.sampleRate;
		if HIGHER_SAMPLE(sr, s) s = sr;
	}
	for (int i = 0; i < CONFIG_PWM_CHANNELS; i++){
		int sr = config->PWMConfigs[i].cfg.sampleRate;
		if HIGHER_SAMPLE(sr, s) s = sr;
	}
	for (int i = 0; i < CONFIG_GPIO_CHANNELS; i++){
		int sr = config->GPIOConfigs[i].cfg.sampleRate;
		if HIGHER_SAMPLE(sr, s) s = sr;
	}
	for (int i = 0; i < CONFIG_TIMER_CHANNELS; i++){
		int sr = config->TimerConfigs[i].cfg.sampleRate;
		if HIGHER_SAMPLE(sr, s) s = sr;
	}
	for (int i = 0; i < CONFIG_ACCEL_CHANNELS; i++){
		int sr = config->AccelConfigs[i].cfg.sampleRate;
		if HIGHER_SAMPLE(sr, s) s = sr;
	}
	if (config->GPSConfigs.GPSInstalled){
		GPSConfig *gpsConfig = &(config->GPSConfigs);
		{
			//TODO this represents "Position sample rate".
			int sr = gpsConfig->latitudeCfg.sampleRate;
			if HIGHER_SAMPLE(sr, s) s = sr;
		}
		{
			int sr = gpsConfig->dateCfg.sampleRate;
			if HIGHER_SAMPLE(sr, s) s = sr;
		}
		{
			int sr = gpsConfig->timeCfg.sampleRate;
			if HIGHER_SAMPLE(sr, s) s = sr;
		}
		{
			int sr = gpsConfig->speedCfg.sampleRate;
			if HIGHER_SAMPLE(sr, s) s = sr;
		}
	}
	TrackConfig *trackCfg = &(config->TrackConfigs);
	{
		int sr = trackCfg->lapCountCfg.sampleRate;
		if HIGHER_SAMPLE(sr, s) s = sr;
	}
	{
		int sr = trackCfg->lapTimeCfg.sampleRate;
		if HIGHER_SAMPLE(sr, s) s = sr;
	}
	{
		int sr = trackCfg->distanceCfg.sampleRate;
		if HIGHER_SAMPLE(sr, s) s = sr;
	}
	{
		int sr = trackCfg->predTimeCfg.sampleRate;
		if HIGHER_SAMPLE(sr, s) s = sr;
	}
	return s;
}


