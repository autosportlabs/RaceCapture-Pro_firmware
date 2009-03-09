#include "loggerTask.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "loggerHardware.h"
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

	g_loggingShouldRun = 0;
	vSemaphoreCreateBinary( g_xLoggerStart );
	
	xSemaphoreTake( g_xLoggerStart, 1 );
	
	xTaskCreate( loggerTask, 	
				( signed portCHAR * ) "loggerTask", 	
				LOGGER_STACK_SIZE, 	
				NULL, 	
				LOGGER_TASK_PRIORITY, 	
				NULL );
}

void lineAppendString(char *s){
	strcat(g_loggerLineBuffer, s);	
}

void lineAppendInt(int num){
	char buf[10];
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


void loggerTask(void *params){
	
	struct LoggerConfig *loggerConfig;
	
	loggerConfig = getWorkingLoggerConfig();
	
	EmbeddedFile f;
	
	while(1){
		//wait for signal to start logging
		if ( xSemaphoreTake(g_xLoggerStart, portMAX_DELAY) == pdTRUE){

			int gpsInstalled = (int)loggerConfig->GPSInstalled;
			int accelInstalled = (int)loggerConfig->AccelInstalled;
			if ( accelInstalled == CONFIG_FEATURE_INSTALLED){
				accel_init();
				accel_setup();
			}
						
			int rc = InitEFS();
			if ( rc == 0 ){
				if (OpenNextLogFile(&f) == 0){
					g_loggingShouldRun = 1;
				}
			}
			
			portTickType xLastWakeTime, startTickTime;
			const portTickType xFrequency = SAMPLE_100Hz;
			startTickTime = xLastWakeTime = xTaskGetTickCount();
			
			writeHeaders(&f,loggerConfig);
			
			//run until signalled to stop
			while (g_loggingShouldRun){
				ToggleLED(LED2);

				//reset line
				g_loggerLineBuffer[0] = '\0';
								
				//Write ADC channels				
				writeADC(loggerConfig);
				//Write GPIO channels
				writeGPIOs(loggerConfig);
				//Write Timer channels
				writeTimerChannels(loggerConfig);
				//Write PWM channels
				writePWMChannels(loggerConfig);
				
				//Optional hardware
				//Write Accelerometer
				if (accelInstalled) writeAccelerometer(loggerConfig);
				//Write GPS
				if (gpsInstalled) writeGPSChannels(&(loggerConfig->GPSConfig));
				
				lineAppendString("\n");
				fileWriteString(&f, g_loggerLineBuffer);
				
				vTaskDelayUntil( &xLastWakeTime, xFrequency );
			}
			file_fclose(&f);
			UnmountEFS();
			DisableLED(LED2);
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
		struct PWMConfig *c = &(config->PWMConfig[i]);
		if (c->sampleRate != SAMPLE_DISABLED){
			fileWriteQuotedString(f,c->label);
			fileWriteString(f, ",");		
		}		
	}	
}

void writeAccelChannelHeaders(EmbeddedFile *f, struct LoggerConfig *config){
	for (int i = 0; i < CONFIG_ACCEL_CHANNELS; i++){
		struct AccelConfig *c = &(config->AccelConfig[i]);
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









void writeAccelerometer(struct LoggerConfig *config){

	int accelValues[CONFIG_ACCEL_CHANNELS];
	
	for (unsigned int i=0; i < CONFIG_ACCEL_CHANNELS;i++){
		struct AccelConfig *ac = &(config->AccelConfig[i]);
		if (ac->sampleRate != SAMPLE_DISABLED){
			accelValues[i] = accel_readAxis(ac->accelChannel);
		}
	}
	
	for (unsigned int i=0; i < CONFIG_ACCEL_CHANNELS;i++){
		struct AccelConfig *ac = &(config->AccelConfig[i]);
		if (ac->sampleRate != SAMPLE_DISABLED){
			lineAppendInt(accelValues[i]);
			lineAppendString(",");	
		}
	}
}

void writeADC(struct LoggerConfig *config){
	
	int precision;

	unsigned int adc[CONFIG_ADC_CHANNELS];
	ReadAllADC(&adc[0],&adc[1],&adc[2],&adc[3],&adc[4],&adc[5],&adc[6],&adc[7]);	
	
	precision = 2;
	
	for (unsigned int i=0; i < CONFIG_ADC_CHANNELS;i++){
		struct ADCConfig *ac = &(config->ADCConfigs[i]);
		if (ac->sampleRate != SAMPLE_DISABLED){
			lineAppendFloat(ac->scaling * (float)adc[i],precision);
			lineAppendString(",");
		}
	}
}

void writeGPSChannels(struct GPSConfig *config){
	
	if (config->timeSampleRate != SAMPLE_DISABLED){
		lineAppendFloat(getUTCTime(),3);
		lineAppendString(",");
	}
	if (config->positionSampleRate != SAMPLE_DISABLED){
		lineAppendInt(getGPSQuality());
		lineAppendString(",");
		
		lineAppendInt(getSatellitesUsedForPosition());
		lineAppendString(",");

		lineAppendDouble(getLatitude(), 6);
		lineAppendString(",");
		
		lineAppendDouble(getLongitude(), 6);
		lineAppendString(",");
		
		setGPSPositionStale();
	}
	if (config->velocitySampleRate != SAMPLE_DISABLED){
		lineAppendFloat(getGPSVelocity(),2);
		lineAppendString(",");
		setGPSVelocityStale();
	}
}

void writeGPIOs(struct LoggerConfig *loggerConfig){
	
	unsigned int gpioMasks[CONFIG_GPIO_CHANNELS]; 
	
	gpioMasks[0] = GPIO_1;
	gpioMasks[1] = GPIO_2;
	gpioMasks[2] = GPIO_3;
	
	unsigned int gpioStates = AT91F_PIO_GetInput(AT91C_BASE_PIOA);
	for (int i = 0; i < CONFIG_GPIO_CHANNELS; i++){
		struct GPIOConfig *c = &(loggerConfig->GPIOConfigs[i]);
		if (c->sampleRate != SAMPLE_DISABLED){
			lineAppendInt((gpioStates & gpioMasks[i]) != 0);
			lineAppendString(",");
		}
	}
}

void writeTimerChannels(struct LoggerConfig *loggerConfig){
	
	unsigned int timers[CONFIG_TIMER_CHANNELS];
	getAllTimerPeriods(timers,timers + 1,timers + 2);
	
	for (int i = 0; i < CONFIG_TIMER_CHANNELS; i++){
		struct TimerConfig *c = &(loggerConfig->TimerConfigs[i]);
		if (c->sampleRate != SAMPLE_DISABLED){
			lineAppendInt(timers[i]);
			lineAppendString(",");	
		}	
	}
}

void writePWMChannels(struct LoggerConfig *loggerConfig){
	
	for (int i = 0; i < CONFIG_PWM_CHANNELS; i++){
		struct PWMConfig *c = &(loggerConfig->PWMConfig[i]);
		if (c->sampleRate != SAMPLE_DISABLED){
			lineAppendInt(i);
			lineAppendString(",");
		}	
	}
}

