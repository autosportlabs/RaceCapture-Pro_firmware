#include "loggerConfig.h"
#include "mod_string.h"
#include "memory.h"
#include "printk.h"
#include "virtual_channel.h"

#ifndef RCP_TESTING
#include "memory.h"
static const LoggerConfig g_savedLoggerConfig  __attribute__((section(".config\n\t#"))) = DEFAULT_LOGGER_CONFIG;
#else
static LoggerConfig g_savedLoggerConfig = DEFAULT_LOGGER_CONFIG;
#endif

static const LoggerConfig g_defaultLoggerConfig = DEFAULT_LOGGER_CONFIG;

static LoggerConfig g_workingLoggerConfig;

static int firmware_major_version_matches_last(){
	return g_savedLoggerConfig.RcpVersionInfo.major == MAJOR_REV;
}

static int firmware_version_matches_last(){
	const VersionInfo * version = &g_savedLoggerConfig.RcpVersionInfo;
	return version->major == MAJOR_REV && version->minor == MINOR_REV && version->bugfix == BUGFIX_REV;
}

int flash_default_logger_config(void){
	pr_info("flashing default logger config...");
	int result = memory_flash_region(&g_savedLoggerConfig, &g_defaultLoggerConfig, sizeof (LoggerConfig));
	if (result == 0) pr_info("success\r\n"); else pr_info("failed\r\n");
	return result;
}

int flashLoggerConfig(void){
	return memory_flash_region(&g_savedLoggerConfig, &g_workingLoggerConfig, sizeof (LoggerConfig));
}

void initialize_logger_config(){
	if (! firmware_version_matches_last()){
		pr_info("new firmware detected\r\n");
	}

	if (! firmware_major_version_matches_last()){
		pr_info("firmware major version changed\r\n");
		flash_default_logger_config();
	}
	memcpy(&g_workingLoggerConfig,&g_savedLoggerConfig,sizeof(LoggerConfig));
}

const LoggerConfig * getSavedLoggerConfig(){
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
	ConnectivityConfig *connConfig = &getWorkingLoggerConfig()->ConnectivityConfigs;
	int sampleRateLimit = connConfig->cellularConfig.cellEnabled ? SLOW_LINK_MAX_TELEMETRY_SAMPLE_RATE : FAST_LINK_MAX_TELEMETRY_SAMPLE_RATE;
	return sampleRateLimit;
}

int encodeSampleRate(int sampleRate){

	switch(sampleRate){
		case 200:
			return SAMPLE_200Hz;
		case 100:
			return SAMPLE_100Hz;
		case 50:
			return SAMPLE_50Hz;
		case 25:
			return SAMPLE_25Hz;
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
		case SAMPLE_25Hz:
			return 25;
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

unsigned char filterAnalogScalingMode(unsigned char mode){
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

unsigned char filterSdLoggingMode(unsigned char mode){
	switch (mode){
		case SD_LOGGING_MODE_CSV:
			return SD_LOGGING_MODE_CSV;
		default:
		case SD_LOGGING_MODE_DISABLED:
			return SD_LOGGING_MODE_DISABLED;
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

unsigned char filterPulsePerRevolution(unsigned char pulsePerRev){
	return pulsePerRev == 0 ? 1 : pulsePerRev;
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

int filterImuChannel(int config){
	switch(config){
		case IMU_CHANNEL_Y:
			return IMU_CHANNEL_Y;
		case IMU_CHANNEL_Z:
			return IMU_CHANNEL_Z;
		case IMU_CHANNEL_YAW:
			return IMU_CHANNEL_YAW;
		default:
		case IMU_CHANNEL_X:
			return IMU_CHANNEL_X;
	}
}

int filterImuRawValue(int imuRawValue){
	if (imuRawValue > MAX_IMU_RAW){
		imuRawValue = MAX_IMU_RAW;
	} else if (imuRawValue < MIN_IMU_RAW){
		imuRawValue = MIN_IMU_RAW;
	}
	return imuRawValue;
}

int filterImuMode(int mode){
	switch (mode){
		case MODE_IMU_DISABLED:
			return MODE_IMU_DISABLED;
		case MODE_IMU_INVERTED:
			return MODE_IMU_INVERTED;
		default:
		case MODE_IMU_NORMAL:
			return MODE_IMU_NORMAL;
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

ImuConfig * getImuConfigChannel(int channel){
	ImuConfig * c = NULL;
	if (channel >= 0 && channel < CONFIG_IMU_CHANNELS){
		c = &(getWorkingLoggerConfig()->ImuConfigs[channel]);
	}
	return c;
}

unsigned int getHighestSampleRate(LoggerConfig *config){
   /*
    * Bypass Interval and Utc here since they will always be logging
    * at the highest rate based on the results of this very method
    */

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
	for (int i = 0; i < CONFIG_IMU_CHANNELS; i++){
		int sr = config->ImuConfigs[i].cfg.sampleRate;
		if HIGHER_SAMPLE(sr, s) s = sr;
	}
	GPSConfig *gpsConfig = &(config->GPSConfigs);
	{
		//TODO this represents "Position sample rate".
		int sr = gpsConfig->sampleRate;
		if HIGHER_SAMPLE(sr, s) s = sr;
	}
	LapConfig *trackCfg = &(config->LapConfigs);
	{
		int sr = trackCfg->lapCountCfg.sampleRate;
		if HIGHER_SAMPLE(sr, s) s = sr;
	}
	{
		int sr = trackCfg->lapTimeCfg.sampleRate;
		if HIGHER_SAMPLE(sr, s) s = sr;
	}
	{
		int sr = trackCfg->sectorCfg.sampleRate;
		if HIGHER_SAMPLE(sr, s) s = sr;
	}
	{
		int sr = trackCfg->sectorTimeCfg.sampleRate;
		if HIGHER_SAMPLE(sr, s) s = sr;
	}
	{
		int sr = trackCfg->predTimeCfg.sampleRate;
		if HIGHER_SAMPLE(sr, s) s = sr;
	}

	return s;
}

size_t get_enabled_channel_count(LoggerConfig *loggerConfig){
	size_t channels = 2; // Always have Interval (Uptime) and Utc

	for (int i=0; i < CONFIG_IMU_CHANNELS; i++){
		if (loggerConfig->ImuConfigs[i].cfg.sampleRate != SAMPLE_DISABLED) channels++;
	}

	for (int i=0; i < CONFIG_ADC_CHANNELS; i++){
		if (loggerConfig->ADCConfigs[i].cfg.sampleRate != SAMPLE_DISABLED) channels++;
	}

	for (int i=0; i < CONFIG_TIMER_CHANNELS; i++){
		if (loggerConfig->TimerConfigs[i].cfg.sampleRate != SAMPLE_DISABLED) channels++;
	}

	for (int i=0; i < CONFIG_GPIO_CHANNELS; i++){
		if (loggerConfig->GPIOConfigs[i].cfg.sampleRate != SAMPLE_DISABLED) channels++;
	}

	for (int i=0; i < CONFIG_PWM_CHANNELS; i++){
		if (loggerConfig->PWMConfigs[i].cfg.sampleRate != SAMPLE_DISABLED) channels++;
	}

	channels+=loggerConfig->OBD2Configs.enabledPids;

	GPSConfig *gpsConfigs = &loggerConfig->GPSConfigs;
	if (gpsConfigs->sampleRate != SAMPLE_DISABLED){
		if (gpsConfigs->positionEnabled) channels+=2;
		if (gpsConfigs->speedEnabled) channels++;
		if (gpsConfigs->timeEnabled != SAMPLE_DISABLED) channels++;
		if (gpsConfigs->satellitesEnabled != SAMPLE_DISABLED) channels++;
		if (gpsConfigs->distanceEnabled != SAMPLE_DISABLED) channels++;
	}

	LapConfig *lapConfig = &loggerConfig->LapConfigs;
	if (lapConfig->lapCountCfg.sampleRate != SAMPLE_DISABLED) channels++;
	if (lapConfig->lapTimeCfg.sampleRate != SAMPLE_DISABLED) channels++;
	if (lapConfig->sectorCfg.sampleRate != SAMPLE_DISABLED) channels++;
	if (lapConfig->sectorTimeCfg.sampleRate != SAMPLE_DISABLED) channels++;
	if (lapConfig->predTimeCfg.sampleRate != SAMPLE_DISABLED) channels++;

	channels += get_virtual_channel_count();
	return channels;
}
