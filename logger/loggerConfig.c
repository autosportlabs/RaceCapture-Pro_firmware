#include "loggerConfig.h"
#include "string.h"
#include "memory.h"
#include "AT91SAM7S256.h"
#include "Board.h"



struct LoggerConfig g_savedLoggerConfig __attribute__ ((aligned (MEMORY_PAGE_SIZE))) __attribute__((section(".text\n\t#"))) = DEFAULT_LOGGER_CONFIG;
struct LoggerConfig g_workingLoggerConfig;

void updateActiveLoggerConfig(){
	memcpy(&g_workingLoggerConfig,&g_savedLoggerConfig,sizeof(struct LoggerConfig));
}

int flashLoggerConfig(){
	return flashWriteRegion((void *)&g_savedLoggerConfig,(void *)&g_workingLoggerConfig, sizeof (struct LoggerConfig));
}

struct LoggerConfig * getSavedLoggerConfig(){
	return &g_savedLoggerConfig;	
}

struct LoggerConfig * getWorkingLoggerConfig(){
	return &g_workingLoggerConfig;
}

void calculateTimerScaling(struct LoggerConfig *loggerConfig, unsigned int timerChannel){
	struct TimerConfig *timerConfig = &(loggerConfig->TimerConfigs[timerChannel]);
	unsigned int clock = BOARD_MCK / timerConfig->timerDivider;
	clock = clock / timerConfig->pulsePerRevolution;
	timerConfig->calculatedScaling = clock;
}

int mapSampleRate(int sampleRate){

	switch(sampleRate){
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

int filterPWMOutputConfig(int value){
	switch(value){
		case CONFIG_PWM_ANALOG:
			return CONFIG_PWM_ANALOG;
		case CONFIG_PWM_FREQUENCY:
		default:
			return CONFIG_PWM_FREQUENCY;	
	}
}

int filterPWMLoggingConfig(int config){
	switch (config){
		case CONFIG_LOGGING_PWM_PERIOD:
			return CONFIG_LOGGING_PWM_PERIOD;
		case CONFIG_LOGGING_PWM_DUTY:
			return CONFIG_LOGGING_PWM_DUTY;
		case CONFIG_LOGGING_PWM_VOLTS:
		default:
			return CONFIG_LOGGING_PWM_VOLTS;
	}
}


int filterPWMDutyCycle(int dutyCycle){
	if (dutyCycle > MAX_DUTY_CYCLE){
		dutyCycle = MAX_DUTY_CYCLE;
	} else if (dutyCycle < MIN_DUTY_CYCLE){
		dutyCycle = MIN_DUTY_CYCLE;
	}
	return dutyCycle;
}

int filterPWMPeriod(int period){
	if (period > MAX_PWM_PERIOD){
		period = MAX_PWM_PERIOD;
	} else if (period < MIN_PWM_PERIOD){
		period = MIN_PWM_PERIOD;		
	}
	return period;		
}

int filterPWMFreqency(int freq){
	if (freq > MAX_PWM_FREQUENCY){
		freq = MAX_PWM_FREQUENCY;
	} else if (freq < MIN_PWM_FREQUENCY){
		freq = MIN_PWM_FREQUENCY;
	}
	return freq;
}

struct PWMConfig * getPWMConfigChannel(int channel){
	struct PWMConfig * c = NULL;
	if (channel >= 0 && channel < CONFIG_PWM_CHANNELS){
		c = &(getWorkingLoggerConfig()->PWMConfigs[channel]);	
	}
	return c;
}

struct TimerConfig * getTimerConfigChannel(int channel){
	struct TimerConfig * c = NULL;
	if (channel >=0 && channel < CONFIG_TIMER_CHANNELS){
		c = &(getWorkingLoggerConfig()->TimerConfigs[channel]);
	}
	return c;
}

struct ADCConfig * getADCConfigChannel(int channel){
	struct ADCConfig *c = NULL;
	if (channel >=0 && channel < CONFIG_ADC_CHANNELS){
		c = &(getWorkingLoggerConfig()->ADCConfigs[channel]);		
	}
	return c;
}
