#include "loggerTask.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "loggerHardware.h"
#include "luaLoggerBinding.h"
#include "usb_comm.h"
#include "modp_numtoa.h"
#include "string.h"
#include "stdio.h"
#include "sdcard.h"
#include "gps.h"
#include "accelerometer.h"


#define LOGGER_TASK_PRIORITY				( tskIDLE_PRIORITY + 3 )
#define LOGGER_STACK_SIZE  					200

#define LOGGER_LINE_LENGTH					512
int g_loggingShouldRun;

xSemaphoreHandle g_xLoggerStart;

char g_loggerLineBuffer[LOGGER_LINE_LENGTH];

void createLoggerTask(){

	updateActiveLoggerConfig();

	g_loggingShouldRun = 0;

	registerLuaLoggerBindings();

	vSemaphoreCreateBinary( g_xLoggerStart );
	xSemaphoreTake( g_xLoggerStart, 1 );
	xTaskCreate( loggerTask,( signed portCHAR * ) "loggerTask",	LOGGER_STACK_SIZE, NULL, LOGGER_TASK_PRIORITY, NULL );
}

void lineAppendString(char *s){
	strcat(g_loggerLineBuffer, s);	
}

void lineAppendInt(int num){
	char buf[13];
	modp_itoa10(num,buf);
	lineAppendString(buf);
}

void lineAppendFloat(float num, int precision){
	char buf[20];
	modp_ftoa(num, buf, precision);
	lineAppendString(buf);
}

void lineAppendDouble(double num, int precision){
	char buf[30];
	modp_dtoa(num, buf, precision);
	lineAppendString(buf);
}

void fileWriteQuotedString(EmbeddedFile *f, char *s){
	fileWriteString(f,"\"");
	fileWriteString(f,s);
	fileWriteString(f,"\"");
}

void fileWriteString(EmbeddedFile *f, char *s){
	int len = strlen(s);
	file_write(f,len,(unsigned char *)s);
}

void fileWriteInt(EmbeddedFile *f, int num){
	char buf[10];
	modp_itoa10(num,buf);
	fileWriteString(f,buf);
}

void fileWriteFloat(EmbeddedFile *f, float num, int precision){
	char buf[20];
	modp_ftoa(num, buf, precision);
	fileWriteString(f,buf);
}

void fileWriteDouble(EmbeddedFile *f, double num, int precision){
	char buf[30];
	modp_dtoa(num, buf, precision);
	fileWriteString(f,buf);
}


#define HIGHER_SAMPLE(X,Y) ((X != SAMPLE_DISABLED && X < Y))

portTickType getHighestIdleSampleRate(struct LoggerConfig *config){

	//start with the slowest sample rate
	int s = SAMPLE_1Hz;
	for (int i = 0; i < CONFIG_ACCEL_CHANNELS; i++){
		int sr = config->AccelConfigs[i].idleSampleRate;
		if HIGHER_SAMPLE(sr, s) s = sr;
	}
	return (portTickType)s;
}

portTickType getHighestSampleRate(struct LoggerConfig *config){

	//start with the slowest sample rate
	int s = SAMPLE_1Hz;
	//find the fastest sample rate
	for (int i = 0; i < CONFIG_ADC_CHANNELS; i++){
		int sr = config->ADCConfigs[i].sampleRate; 
		if HIGHER_SAMPLE(sr, s) s = sr; 	
	}
	for (int i = 0; i < CONFIG_PWM_CHANNELS; i++){
		int sr = config->PWMConfigs[i].sampleRate;
		if HIGHER_SAMPLE(sr, s) s = sr;	
	}
	for (int i = 0; i < CONFIG_GPIO_CHANNELS; i++){
		int sr = config->GPIOConfigs[i].sampleRate;
		if HIGHER_SAMPLE(sr, s) s = sr;	
	}
	for (int i = 0; i < CONFIG_TIMER_CHANNELS; i++){
		int sr = config->TimerConfigs[i].sampleRate;
		if HIGHER_SAMPLE(sr, s) s = sr;	
	}
	if (config->AccelInstalled){
		for (int i = 0; i < CONFIG_ACCEL_CHANNELS; i++){
			int sr = config->AccelConfigs[i].sampleRate;
			if HIGHER_SAMPLE(sr, s) s = sr;	
		}
	}
	if (config->GPSInstalled){
		struct GPSConfig *gpsConfig = &(config->GPSConfig);
		{
			int sr = gpsConfig->positionSampleRate; 	
			if HIGHER_SAMPLE(sr, s) s = sr;
		}
		{
			int sr = gpsConfig->timeSampleRate;
			if HIGHER_SAMPLE(sr, s) s = sr;
		}
		{
			int sr = gpsConfig->velocitySampleRate;
			if HIGHER_SAMPLE(sr, s) s = sr;	
		}
	}
	return (portTickType)s;
}


void loggerTask(void *params){
	
	struct LoggerConfig *loggerConfig;
	
	loggerConfig = getWorkingLoggerConfig();
	
	EmbeddedFile f;
	
	if ( loggerConfig->AccelInstalled == CONFIG_FEATURE_INSTALLED ) accel_setup();

	portTickType idleTicks = 0;
	
	portTickType idleDelay = portMAX_DELAY;

	if (loggerConfig->AccelInstalled){
		idleDelay = getHighestIdleSampleRate(loggerConfig);			
	}

	while(1){

		//wait for signal to start logging
		if ( xSemaphoreTake(g_xLoggerStart, idleDelay) != pdTRUE){
			//perform idle tasks

			idleTicks+=idleDelay;
			for (unsigned int i=0; i < CONFIG_ACCEL_CHANNELS;i++){
				struct AccelConfig *ac = &(loggerConfig->AccelConfigs[i]);
				portTickType sr = ac->idleSampleRate;
				if (sr != SAMPLE_DISABLED && (idleTicks % sr) == 0){

//					int xx = readAccelAxis(ac->accelChannel);
	//				SendString("Blah ");
		//			SendInt(xx);
			//		SendCrlf();
				}
			}
		}
		else if (isCardPresent() && isCardWritable()){

			//perform logging tasks
			int gpsInstalled = (int)loggerConfig->GPSInstalled;
			int accelInstalled = (int)loggerConfig->AccelInstalled;
						
			int rc = InitEFS();
			if ( rc == 0 ){
				if (OpenNextLogFile(&f) == 0){
					g_loggingShouldRun = 1;
				}
			}
			
			portTickType xLastWakeTime;
			
			const portTickType xFrequency = getHighestSampleRate(loggerConfig);
			
			xLastWakeTime = xTaskGetTickCount();

			writeHeaders(&f,loggerConfig);
			
			portTickType currentTicks = 0;
			
			//run until signalled to stop
			while (g_loggingShouldRun){
				currentTicks += xFrequency;
				
				ToggleLED(LED2);

				//reset line
				g_loggerLineBuffer[0] = '\0';
								
				//Write ADC channels				
				writeADC(currentTicks, loggerConfig);
				//Write GPIO channels
				writeGPIOs(currentTicks, loggerConfig);
				//Write Timer channels
				writeTimerChannels(currentTicks, loggerConfig);
				//Write PWM channels
				writePWMChannels(currentTicks, loggerConfig);
				
				//Optional hardware
				//Write Accelerometer
				if (accelInstalled) writeAccelerometer(currentTicks, loggerConfig);
				//Write GPS
				if (gpsInstalled) writeGPSChannels(currentTicks, &(loggerConfig->GPSConfig));
				
				lineAppendString("\n");
				fileWriteString(&f, g_loggerLineBuffer);
				
				if (ReadADC(7) < 400){
					g_loggingShouldRun = 0;

					fileWriteString(&f,"x\n");
					break;
				}
				vTaskDelayUntil( &xLastWakeTime, xFrequency );
			}
			file_fclose(&f);
			UnmountEFS();
			DisableLED(LED2);
			
			idleTicks = 0;
			idleDelay = portMAX_DELAY;
			if (loggerConfig->AccelInstalled){
				idleDelay = getHighestIdleSampleRate(loggerConfig);			
			}
		}
	}
}


void writeHeaders(EmbeddedFile *f, struct LoggerConfig *config){
	writeADCHeaders(f, config);
	writeGPIOHeaders(f, config);
	writeTimerChannelHeaders(f, config);
	writePWMChannelHeaders(f, config);
	if (config->AccelInstalled) writeAccelChannelHeaders(f, config);
	if (config->GPSInstalled) writeGPSChannelHeaders(f, &(config->GPSConfig));
	fileWriteString(f,"\n");
}

void writeADCHeaders(EmbeddedFile *f,struct LoggerConfig *config){
	for (int i = 0; i < CONFIG_ADC_CHANNELS; i++){
		struct ADCConfig *c = &(config->ADCConfigs[i]);
		if (c->sampleRate != SAMPLE_DISABLED){
			fileWriteQuotedString(f,c->label);
			fileWriteString(f,",");
		}
	}	
}

void writeGPIOHeaders(EmbeddedFile *f, struct LoggerConfig *config){
	for (int i = 0; i < CONFIG_GPIO_CHANNELS; i++){
		struct GPIOConfig *c = &(config->GPIOConfigs[i]);
		if (c->sampleRate != SAMPLE_DISABLED){
			fileWriteQuotedString(f,c->label);
			fileWriteString(f,",");
		}	
	}
}

void writeTimerChannelHeaders(EmbeddedFile *f, struct LoggerConfig *config){
	for (int i = 0; i < CONFIG_TIMER_CHANNELS; i++){
		struct TimerConfig *c = &(config->TimerConfigs[i]);
		if (c->sampleRate != SAMPLE_DISABLED){
			fileWriteQuotedString(f,c->label);
			fileWriteString(f, ",");	
		}		
	}
}

void writePWMChannelHeaders(EmbeddedFile *f, struct LoggerConfig *config){
	for (int i = 0; i < CONFIG_PWM_CHANNELS; i++){
		struct PWMConfig *c = &(config->PWMConfigs[i]);
		if (c->sampleRate != SAMPLE_DISABLED){
			fileWriteQuotedString(f,c->label);
			fileWriteString(f, ",");		
		}		
	}	
}

void writeAccelChannelHeaders(EmbeddedFile *f, struct LoggerConfig *config){
	for (int i = 0; i < CONFIG_ACCEL_CHANNELS; i++){
		struct AccelConfig *c = &(config->AccelConfigs[i]);
		if (c->sampleRate != SAMPLE_DISABLED){
			fileWriteQuotedString(f,c->label);
			fileWriteString(f, ",");		
		}	
	}	
}

void writeGPSChannelHeaders(EmbeddedFile *f, struct GPSConfig *config){

	if (config->timeSampleRate != SAMPLE_DISABLED){
		fileWriteQuotedString(f, config->timeLabel);
		fileWriteString(f, ",");	
	}
	
	if (config->positionSampleRate != SAMPLE_DISABLED){
		fileWriteQuotedString(f,config->qualityLabel);
		fileWriteString(f, ",");
		
		fileWriteQuotedString(f,config->satsLabel);
		fileWriteString(f, ",");
		
		fileWriteQuotedString(f,config->latitiudeLabel);
		fileWriteString(f, ",");
		fileWriteQuotedString(f,config->longitudeLabel);
		fileWriteString(f, ",");	
	}

	if (config->velocitySampleRate != SAMPLE_DISABLED){
		fileWriteQuotedString(f, config->velocityLabel);
		fileWriteString(f, ",");
	}
}


void writeAccelerometer(portTickType currentTicks, struct LoggerConfig *config){

	unsigned int accelValues[CONFIG_ACCEL_CHANNELS];
	
	for (unsigned int i=0; i < CONFIG_ACCEL_CHANNELS;i++){
		struct AccelConfig *ac = &(config->AccelConfigs[i]);
		portTickType sr = ac->sampleRate;
		if (sr != SAMPLE_DISABLED && (currentTicks % sr) == 0){
			accelValues[i] = readAccelChannel(ac->accelChannel);
		}
	}
	
	for (unsigned int i=0; i < CONFIG_ACCEL_CHANNELS;i++){
		struct AccelConfig *ac = &(config->AccelConfigs[i]);
		portTickType sr = ac->sampleRate;
		if (sr != SAMPLE_DISABLED){
			if ((currentTicks % sr) == 0) lineAppendFloat(convertAccelRawToG(accelValues[i],ac->zeroValue),DEFAULT_ACCEL_LOGGING_PRECISION);
			lineAppendString(",");
		}
	}
}

void writeADC(portTickType currentTicks, struct LoggerConfig *config){
	
	unsigned int adc[CONFIG_ADC_CHANNELS];
	ReadAllADC(&adc[0],&adc[1],&adc[2],&adc[3],&adc[4],&adc[5],&adc[6],&adc[7]);	
	
	for (unsigned int i=0; i < CONFIG_ADC_CHANNELS;i++){
		struct ADCConfig *ac = &(config->ADCConfigs[i]);
		portTickType sr = ac->sampleRate;
		if (sr != SAMPLE_DISABLED){
			if ((currentTicks % sr) == 0) lineAppendFloat(ac->scaling * (float)adc[i],DEFAULT_ADC_LOGGING_PRECISION);
			lineAppendString(",");
		}
	}
}

void writeGPSChannels(portTickType currentTicks, struct GPSConfig *config){

	{
		portTickType sr = config->timeSampleRate;
		if (sr != SAMPLE_DISABLED){
			if ((currentTicks % sr) == 0) lineAppendFloat(getUTCTime(),3);
			lineAppendString(",");
		}
	}
	{
		portTickType sr = config->positionSampleRate;
		if (sr != SAMPLE_DISABLED){
			if ((currentTicks % sr) == 0){
				lineAppendInt(getGPSQuality());
				lineAppendString(",");
				
				lineAppendInt(getSatellitesUsedForPosition());
				lineAppendString(",");
		
				lineAppendDouble(getLatitude(), 6);
				lineAppendString(",");
				
				lineAppendDouble(getLongitude(), 6);
				lineAppendString(",");
			} else{
				lineAppendString(",,,,");	
			}
		}
	}
	
	{
		portTickType sr = config->velocitySampleRate;
		if (sr != SAMPLE_DISABLED){
			if ((currentTicks % sr) == 0){
				lineAppendFloat(getGPSVelocity(),2);
			}
			lineAppendString(",");
		}
	}
}

void writeGPIOs(portTickType currentTicks, struct LoggerConfig *loggerConfig){
	
	unsigned int gpioMasks[CONFIG_GPIO_CHANNELS]; 
	
	gpioMasks[0] = GPIO_1;
	gpioMasks[1] = GPIO_2;
	gpioMasks[2] = GPIO_3;
	
	unsigned int gpioStates = AT91F_PIO_GetInput(AT91C_BASE_PIOA);
	for (int i = 0; i < CONFIG_GPIO_CHANNELS; i++){
		struct GPIOConfig *c = &(loggerConfig->GPIOConfigs[i]);
		portTickType sr = c->sampleRate;
		if (sr != SAMPLE_DISABLED){
			if ((currentTicks % sr) == 0) lineAppendInt((gpioStates & gpioMasks[i]) != 0);
			lineAppendString(",");
		}
	}
}

void writeTimerChannels(portTickType currentTicks, struct LoggerConfig *loggerConfig){
	
	unsigned int timers[CONFIG_TIMER_CHANNELS];
	getAllTimerPeriods(timers,timers + 1,timers + 2);
	
	for (int i = 0; i < CONFIG_TIMER_CHANNELS; i++){
		struct TimerConfig *c = &(loggerConfig->TimerConfigs[i]);
		portTickType sr = c->sampleRate;
		if (sr != SAMPLE_DISABLED){
			if ((currentTicks % sr) == 0){
				int value = 0;
				int scaling = c->calculatedScaling;
				unsigned int timerValue = timers[i];
				switch (c->config){
					case CONFIG_LOGGING_TIMER_RPM:
						value = calculateRPM(timerValue,scaling);
						break;
					case CONFIG_LOGGING_TIMER_FREQUENCY:
						value = calculateFrequencyHz(timerValue,scaling);
						break;
					case CONFIG_LOGGING_TIMER_PERIOD_MS:
						value = calculatePeriodMs(timerValue,scaling);
						break;
					case CONFIG_LOGGING_TIMER_PERIOD_USEC:
						value = calculatePeriodUsec(timerValue,scaling);
					default:
						value = -1;
				}
				lineAppendInt(value);
			}
			lineAppendString(",");	
		}	
	}
}

void writePWMChannels(portTickType currentTicks, struct LoggerConfig *loggerConfig){
	
	for (int i = 0; i < CONFIG_PWM_CHANNELS; i++){
		struct PWMConfig *c = &(loggerConfig->PWMConfigs[i]);
		portTickType sr = c->sampleRate;
		if (sr != SAMPLE_DISABLED){
			if ((currentTicks % sr) == 0){
				switch (c->loggingConfig){
					case CONFIG_LOGGING_PWM_PERIOD:
						lineAppendInt(PWM_GetPeriod(i));
						break;
					case CONFIG_LOGGING_PWM_DUTY:
						lineAppendInt(PWM_GetDutyCycle(i));
						break;
					case CONFIG_LOGGING_PWM_VOLTS:
						lineAppendFloat(PWM_GetDutyCycle(i) * c->voltageScaling,1); 
						break;
					default:
						lineAppendInt(-1);
				}
			}
			lineAppendString(",");
		}	
	}
}

