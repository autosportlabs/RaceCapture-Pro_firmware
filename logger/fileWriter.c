/*
 * fileWriter.c
 *
 *  Created on: Feb 29, 2012
 *      Author: brent
 */
#import "fileWriter.h"

static int g_shouldWrite;
static FIL g_logfile;
static xSemaphoreHandle g_xWritingStart;

#define FILE_WRITER_TASK_PRIORITY				( tskIDLE_PRIORITY + 3 )
#define FILE_WRITER_STACK_SIZE  				200


void createFileWriterTask(){

	g_shouldWrite = 0;

	vSemaphoreCreateBinary( g_xWritingStart );
	xSemaphoreTake( g_xWritingStart, 1 );
	xTaskCreate( fileWriterTask,( signed portCHAR * ) "fileTask", FILE_WRITER_STACK_SIZE, NULL, FILE_WRITER_TASK_PRIORITY, NULL );
}

static void fileWriteQuotedString(FIL *f, char *s){
	fileWriteString(f,"\"");
	fileWriteString(f,s);
	fileWriteString(f,"\"");
}

static void fileWriteString(FIL *f, char *s){
	f_puts(s,f);
	f_sync(f);
}

static void fileWriteInt(FIL *f, int num){
	char buf[10];
	modp_itoa10(num,buf);
	fileWriteString(f,buf);
}

static void fileWriteFloat(FIL *f, float num, int precision){
	char buf[20];
	modp_ftoa(num, buf, precision);
	fileWriteString(f,buf);
}

static void fileWriteDouble(FIL *f, double num, int precision){
	char buf[30];
	modp_dtoa(num, buf, precision);
	fileWriteString(f,buf);
}

static void writeHeaders(FIL *f, LoggerConfig *config){
	writeADCHeaders(f, config);
	writeGPIOHeaders(f, config);
	writeTimerChannelHeaders(f, config);
	writePWMChannelHeaders(f, config);
	if (config->AccelInstalled) writeAccelChannelHeaders(f, config);
	if (config->GPSInstalled) writeGPSChannelHeaders(f, &(config->GPSConfig));
	fileWriteString(f,"\n");
}

static void writeADCHeaders(FIL *f,LoggerConfig *config){
	for (int i = 0; i < CONFIG_ADC_CHANNELS; i++){
		ADCConfig *c = &(config->ADCConfigs[i]);
		if (c->sampleRate != SAMPLE_DISABLED){
			fileWriteQuotedString(f,c->label);
			fileWriteString(f,",");
		}
	}
}

static void writeGPIOHeaders(FIL *f, LoggerConfig *config){
	for (int i = 0; i < CONFIG_GPIO_CHANNELS; i++){
		GPIOConfig *c = &(config->GPIOConfigs[i]);
		if (c->sampleRate != SAMPLE_DISABLED){
			fileWriteQuotedString(f,c->label);
			fileWriteString(f,",");
		}
	}
}

static void writeTimerChannelHeaders(FIL *f, LoggerConfig *config){
	for (int i = 0; i < CONFIG_TIMER_CHANNELS; i++){
		TimerConfig *c = &(config->TimerConfigs[i]);
		if (c->sampleRate != SAMPLE_DISABLED){
			fileWriteQuotedString(f,c->label);
			fileWriteString(f, ",");
		}
	}
}

static void writePWMChannelHeaders(FIL *f, LoggerConfig *config){
	for (int i = 0; i < CONFIG_PWM_CHANNELS; i++){
		PWMConfig *c = &(config->PWMConfigs[i]);
		if (c->sampleRate != SAMPLE_DISABLED){
			fileWriteQuotedString(f,c->label);
			fileWriteString(f, ",");
		}
	}
}

static void writeAccelChannelHeaders(FIL *f, LoggerConfig *config){
	for (int i = 0; i < CONFIG_ACCEL_CHANNELS; i++){
		AccelConfig *c = &(config->AccelConfigs[i]);
		if (c->sampleRate != SAMPLE_DISABLED){
			fileWriteQuotedString(f,c->label);
			fileWriteString(f, ",");
		}
	}
}

static void writeGPSChannelHeaders(FIL *f, GPSConfig *config){

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


static SampleRecord * getNextSampleRecord(){
	return null;
}

static writeSampleRecord(FIL * logfile, SampleRecord * sampleRecord){

	if (NULL == sampleRecord) return;

	for (int i = 0; i < SAMPLE_RECORD_CHANNELS; i++){
		ChannelSample &sample = sampleRecord->Samples[i];
		ChannelConfig * channelConfig = sample.channelConfig;

		if (SAMPLE_DISABLED == channelConfig->sampleRate) continue;

		if (i > 0) fileWriteString(",");

		if (IS_NIL_VALUE(sample)) continue;

		size_t precision = sample->precision;
		if (precision > 0){
			fileWriteFloat(logfile,sample.floatValue,precision);
		}
		else{
			fileWriteInt(logfile, sample.intValue);
		}
	}
	fileWriteString("\n");
}

void fileWriterTask(void *params){

	SampleRecord *sr = NULL;
	while(1){
		//wait for signal to start logging
		if ( xSemaphoreTake(g_xWritingStart, portMAX_DELAY) != pdTRUE){
			//perform idle tasks
		}
		else {
			if (0 == g_shouldWrite){
				//start of a new logfile
				int rc = InitFS();
				if (0 != rc) continue;

				//open next log file
				rc = OpenNextLogFile(&g_logfile);
				if (0 != rc) continue;

				g_shouldWrite = 1;
				writeHeaders(&g_logfile,getWorkingLoggerConfig());
			}

			if (g_shouldWrite){
				//a null sample record means - end of sample run
				//like an EOF
				if (NULL != sr){
					writeSampleRecord(&g_logfile,sr);
				}
				else{
					f_close(&g_logfile);
					g_shouldWrite = 0;
				}
			}
		}
	}
}




