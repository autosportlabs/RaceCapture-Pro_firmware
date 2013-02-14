#include "loggerTask.h"
#include "diskio.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "loggerHardware.h"
#include "loggerData.h"
#include "luaLoggerBinding.h"
#include "modp_numtoa.h"
#include "string.h"
#include "stdio.h"
#include "sdcard.h"
#include "gps.h"
#include "accelerometer.h"


#define LOGGER_TASK_PRIORITY				( tskIDLE_PRIORITY + 4 )
#define LOGGER_STACK_SIZE  					200
#define LOGGER_LINE_LENGTH					512
#define MAX_LOG_FILE_INDEX 					99999

int g_loggingShouldRun;

FIL g_logfile;

xSemaphoreHandle g_xLoggerStart;

char g_loggerLineBuffer[LOGGER_LINE_LENGTH];



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

void fileWriteQuotedString(FIL *f, char *s){
	fileWriteString(f,"\"");
	fileWriteString(f,s);
	fileWriteString(f,"\"");
}

void fileWriteString(FIL *f, char *s){
	f_puts(s,f);
	f_sync(f);
}

void fileWriteInt(FIL *f, int num){
	char buf[10];
	modp_itoa10(num,buf);
	fileWriteString(f,buf);
}

void fileWriteFloat(FIL *f, float num, int precision){
	char buf[20];
	modp_ftoa(num, buf, precision);
	fileWriteString(f,buf);
}

void fileWriteDouble(FIL *f, double num, int precision){
	char buf[30];
	modp_dtoa(num, buf, precision);
	fileWriteString(f,buf);
}

#define HIGHER_SAMPLE(X,Y) ((X != SAMPLE_DISABLED && X < Y))


void createLoggerTask(){

	g_loggingShouldRun = 0;

	registerLuaLoggerBindings();

	vSemaphoreCreateBinary( g_xLoggerStart );
	xSemaphoreTake( g_xLoggerStart, 1 );
	xTaskCreate( loggerTask,( signed portCHAR * ) "logger",	LOGGER_STACK_SIZE, NULL, LOGGER_TASK_PRIORITY, NULL );
}

static int openNextLogFile(FIL *f){

	char filename[13];
	int i = 0;
	int rc;
	for (; i < MAX_LOG_FILE_INDEX; i++){
		strcpy(filename,"rc_");
		char numBuf[12];
		modp_itoa10(i,numBuf);
		strcat(filename,numBuf);
		strcat(filename,".log");
		rc = f_open(f,filename, FA_WRITE | FA_CREATE_NEW);
		if ( rc == 0 ) break;
		f_close(f);
	}
	if (i >= MAX_LOG_FILE_INDEX) return -2;
	return rc;
}


void loggerTask(void *params){
	
	LoggerConfig *loggerConfig;
	
	loggerConfig = getWorkingLoggerConfig();
	
	if ( loggerConfig->AccelInstalled == CONFIG_FEATURE_INSTALLED ) accel_setup();

	while(1){

		//wait for signal to start logging
		if ( xSemaphoreTake(g_xLoggerStart, portMAX_DELAY) != pdTRUE){
			//perform idle tasks
		}
		else {
			//perform logging tasks
			int gpsInstalled = (int)loggerConfig->GPSConfig.GPSInstalled;
			int accelInstalled = (int)loggerConfig->AccelInstalled;
						
			int rc = InitFS();
			if ( rc == 0 ){
				if (openNextLogFile(&g_logfile) == 0){
					g_loggingShouldRun = 1;
				}
			}
			
			const portTickType xFrequency = getHighestSampleRate(loggerConfig);

			writeHeaders(&g_logfile,loggerConfig);
			
			portTickType currentTicks = 0;
			
			//run until signalled to stop
			portTickType xLastWakeTime = xTaskGetTickCount();
			while (g_loggingShouldRun){
				currentTicks += xFrequency;
				
				toggleLED(LED2);

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
				fileWriteString(&g_logfile, g_loggerLineBuffer);

				//test code for detecting power loss via +12v bus.
/*				if (ReadADC(7) < 230){
					g_loggingShouldRun = 0;

					fileWriteString(&f,"x\r\n");
					break;
				}

				*/
				vTaskDelayUntil( &xLastWakeTime, xFrequency );

			}
			f_close(&g_logfile);
			UnmountFS();
			disableLED(LED2);
		}
	}
}


void writeHeaders(FIL *f, LoggerConfig *config){
	writeADCHeaders(f, config);
	writeGPIOHeaders(f, config);
	writeTimerChannelHeaders(f, config);
	writePWMChannelHeaders(f, config);
	if (config->AccelInstalled) writeAccelChannelHeaders(f, config);
	if (config->GPSConfig.GPSInstalled) writeGPSChannelHeaders(f, &(config->GPSConfig));
	fileWriteString(f,"\n");
}

void writeADCHeaders(FIL *f,LoggerConfig *config){
	for (int i = 0; i < CONFIG_ADC_CHANNELS; i++){
		ADCConfig *c = &(config->ADCConfigs[i]);
		if (c->cfg.sampleRate != SAMPLE_DISABLED){
			fileWriteQuotedString(f,c->cfg.label);
			fileWriteString(f,",");
		}
	}	
}

void writeGPIOHeaders(FIL *f, LoggerConfig *config){
	for (int i = 0; i < CONFIG_GPIO_CHANNELS; i++){
		GPIOConfig *c = &(config->GPIOConfigs[i]);
		if (c->cfg.sampleRate != SAMPLE_DISABLED){
			fileWriteQuotedString(f,c->cfg.label);
			fileWriteString(f,",");
		}	
	}
}

void writeTimerChannelHeaders(FIL *f, LoggerConfig *config){
	for (int i = 0; i < CONFIG_TIMER_CHANNELS; i++){
		TimerConfig *c = &(config->TimerConfigs[i]);
		if (c->cfg.sampleRate != SAMPLE_DISABLED){
			fileWriteQuotedString(f,c->cfg.label);
			fileWriteString(f, ",");	
		}		
	}
}

void writePWMChannelHeaders(FIL *f, LoggerConfig *config){
	for (int i = 0; i < CONFIG_PWM_CHANNELS; i++){
		PWMConfig *c = &(config->PWMConfigs[i]);
		if (c->cfg.sampleRate != SAMPLE_DISABLED){
			fileWriteQuotedString(f,c->cfg.label);
			fileWriteString(f, ",");		
		}		
	}	
}

void writeAccelChannelHeaders(FIL *f, LoggerConfig *config){
	for (int i = 0; i < CONFIG_ACCEL_CHANNELS; i++){
		AccelConfig *c = &(config->AccelConfigs[i]);
		if (c->cfg.sampleRate != SAMPLE_DISABLED){
			fileWriteQuotedString(f,c->cfg.label);
			fileWriteString(f, ",");		
		}	
	}	
}

void writeGPSChannelHeaders(FIL *f, GPSConfig *config){

	if (config->timeCfg.sampleRate != SAMPLE_DISABLED){
		fileWriteQuotedString(f, config->timeCfg.label);
		fileWriteString(f, ",");	
	}
	
	if (config->qualityCfg.sampleRate != SAMPLE_DISABLED){
		fileWriteQuotedString(f,config->qualityCfg.label);
		fileWriteString(f, ",");
	}

	if (config->satellitesCfg.sampleRate != SAMPLE_DISABLED){
		fileWriteQuotedString(f,config->satellitesCfg.label);
		fileWriteString(f, ",");
	}

	if (config->latitudeCfg.sampleRate != SAMPLE_DISABLED){
		fileWriteQuotedString(f,config->latitudeCfg.label);
		fileWriteString(f, ",");
	}

	if (config->longitudeCfg.sampleRate != SAMPLE_DISABLED){
		fileWriteQuotedString(f,config->longitudeCfg.label);
		fileWriteString(f, ",");	
	}

	if (config->speedCfg.sampleRate != SAMPLE_DISABLED){
		fileWriteQuotedString(f, config->speedCfg.label);
		fileWriteString(f, ",");
	}
}


void writeAccelerometer(portTickType currentTicks, LoggerConfig *config){

	unsigned int accelValues[CONFIG_ACCEL_CHANNELS];
	
	for (unsigned int i=0; i < CONFIG_ACCEL_CHANNELS;i++){
		AccelConfig *ac = &(config->AccelConfigs[i]);
		portTickType sr = ac->cfg.sampleRate;
		if (sr != SAMPLE_DISABLED && (currentTicks % sr) == 0){
			accelValues[i] = readAccelChannel(ac->accelChannel);
		}
	}
	
	for (unsigned int i=0; i < CONFIG_ACCEL_CHANNELS;i++){
		AccelConfig *ac = &(config->AccelConfigs[i]);
		portTickType sr = ac->cfg.sampleRate;
		if (sr != SAMPLE_DISABLED){
			if ((currentTicks % sr) == 0) lineAppendFloat(convertAccelRawToG(accelValues[i],ac->zeroValue),DEFAULT_ACCEL_LOGGING_PRECISION);
			lineAppendString(",");
		}
	}
}

void writeADC(portTickType currentTicks, LoggerConfig *config){
	
	unsigned int adc[CONFIG_ADC_CHANNELS];
	readAllADC(&adc[0],&adc[1],&adc[2],&adc[3],&adc[4],&adc[5],&adc[6],&adc[7]);	
	
	for (unsigned int i=0; i < CONFIG_ADC_CHANNELS;i++){
		ADCConfig *ac = &(config->ADCConfigs[i]);
		portTickType sr = ac->cfg.sampleRate;
		if (sr != SAMPLE_DISABLED){
			if ((currentTicks % sr) == 0){
				float analogValue = 0;
				switch(ac->scalingMode){
				case SCALING_MODE_RAW:
					analogValue = adc[i];
					break;
				case SCALING_MODE_LINEAR:
					analogValue = (ac->linearScaling * (float)adc[i]);
					break;
				case SCALING_MODE_MAP:
					analogValue = GetMappedValue((float)adc[i],&(ac->scalingMap));
					break;
				}
				lineAppendFloat(analogValue,ac->loggingPrecision);
			}
			lineAppendString(",");
		}
	}
}

void writeGPSChannels(portTickType currentTicks, GPSConfig *config){

	{
		portTickType sr = config->timeCfg.sampleRate;
		if (sr != SAMPLE_DISABLED){
			if ((currentTicks % sr) == 0) lineAppendFloat(getUTCTime(),3);
			lineAppendString(",");
		}
	}
	{
		portTickType sr = config->latitudeCfg.sampleRate;
		if (sr != SAMPLE_DISABLED){
			if ((currentTicks % sr) == 0){
				lineAppendDouble(getLatitude(), 6);
				lineAppendString(",");
				
				lineAppendDouble(getLongitude(), 6);
				lineAppendString(",");
			} else{
				lineAppendString(",,");
			}
		}
	}
	
	{
		portTickType sr = config->qualityCfg.sampleRate;
		if (sr != SAMPLE_DISABLED){
			if ((currentTicks % sr) == 0) lineAppendInt(getGPSQuality());
			lineAppendString(",");
		}
	}

	{
		portTickType sr = config->satellitesCfg.sampleRate;
		if (sr != SAMPLE_DISABLED){
			if ((currentTicks % sr) == 0) lineAppendInt(getSatellitesUsedForPosition());
			lineAppendString(",");
		}
	}

	{
		portTickType sr = config->speedCfg.sampleRate;
		if (sr != SAMPLE_DISABLED){
			if ((currentTicks % sr) == 0) lineAppendFloat(getGPSSpeed(),2);
			lineAppendString(",");
		}
	}
}

void writeGPIOs(portTickType currentTicks, LoggerConfig *loggerConfig){
	
	unsigned int gpioMasks[CONFIG_GPIO_CHANNELS]; 
	
	gpioMasks[0] = GPIO_1;
	gpioMasks[1] = GPIO_2;
	gpioMasks[2] = GPIO_3;
	
	unsigned int gpioStates = AT91F_PIO_GetInput(AT91C_BASE_PIOA);
	for (int i = 0; i < CONFIG_GPIO_CHANNELS; i++){
		GPIOConfig *c = &(loggerConfig->GPIOConfigs[i]);
		portTickType sr = c->cfg.sampleRate;
		if (sr != SAMPLE_DISABLED){
			if ((currentTicks % sr) == 0) lineAppendInt((gpioStates & gpioMasks[i]) != 0);
			lineAppendString(",");
		}
	}
}

void writeTimerChannels(portTickType currentTicks, LoggerConfig *loggerConfig){
	
	unsigned int timers[CONFIG_TIMER_CHANNELS];
	getAllTimerPeriods(timers,timers + 1,timers + 2);
	
	for (int i = 0; i < CONFIG_TIMER_CHANNELS; i++){
		TimerConfig *c = &(loggerConfig->TimerConfigs[i]);
		portTickType sr = c->cfg.sampleRate;
		if (sr != SAMPLE_DISABLED){
			if ((currentTicks % sr) == 0){
				int value = 0;
				int scaling = c->calculatedScaling;
				unsigned int timerValue = timers[i];
				switch (c->mode){
					case MODE_LOGGING_TIMER_RPM:
						value = calculateRPM(timerValue,scaling);
						break;
					case MODE_LOGGING_TIMER_FREQUENCY:
						value = calculateFrequencyHz(timerValue,scaling);
						break;
					case MODE_LOGGING_TIMER_PERIOD_MS:
						value = calculatePeriodMs(timerValue,scaling);
						break;
					case MODE_LOGGING_TIMER_PERIOD_USEC:
						value = calculatePeriodUsec(timerValue,scaling);
						break;
					default:
						value = -1;
						break;
				}
				lineAppendInt(value);
			}
			lineAppendString(",");	
		}	
	}
}

void writePWMChannels(portTickType currentTicks, LoggerConfig *loggerConfig){
	
	for (int i = 0; i < CONFIG_PWM_CHANNELS; i++){
		PWMConfig *c = &(loggerConfig->PWMConfigs[i]);
		portTickType sr = c->cfg.sampleRate;
		if (sr != SAMPLE_DISABLED){
			if ((currentTicks % sr) == 0){
				switch (c->loggingMode){
					case MODE_LOGGING_PWM_PERIOD:
						lineAppendInt(PWM_GetPeriod(i));
						break;
					case MODE_LOGGING_PWM_DUTY:
						lineAppendInt(PWM_GetDutyCycle(i));
						break;
					case MODE_LOGGING_PWM_VOLTS:
						lineAppendFloat(PWM_GetDutyCycle(i) * c->voltageScaling,1); 
						break;
					default:
						lineAppendInt(-1);
						break;
				}
			}
			lineAppendString(",");
		}	
	}
}




