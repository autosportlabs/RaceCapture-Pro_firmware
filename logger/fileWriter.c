/*
 * fileWriter.c
 *
 *  Created on: Feb 29, 2012
 *      Author: brent
 */
#include "fileWriter.h"
#include "task.h"
#include "semphr.h"
#include "modp_numtoa.h"
#include "sdcard.h"
#include "sampleRecord.h"
#include "loggerHardware.h"
#include "taskUtil.h"
#include "mod_string.h"
#include "printk.h"
#include "spi.h"

enum writing_status {
	WRITING_INACTIVE = 0,
	WRITING_ACTIVE = 1,
	WRITING_INITIALIZATION_ERROR = 2
};

static enum writing_status g_writingStatus;
static FIL g_logfile;
static xQueueHandle g_sampleRecordQueue = NULL;

#define FILE_WRITER_TASK_PRIORITY				( tskIDLE_PRIORITY + 4 )
#define FILE_WRITER_STACK_SIZE  				200
#define SAMPLE_RECORD_QUEUE_SIZE				10
#define MAX_LOG_FILE_INDEX 						99999
#define FLUSH_INTERVAL_SEC						10

//wait time for sample queue. can be portMAX_DELAY to wait forever, or zero to not wait at all
#define SAMPLE_QUEUE_WAIT_TIME					0
//#define SAMPLE_QUEUE_WAIT_TIME					portMAX_DELAY

portBASE_TYPE queueLogfileRecord(SampleRecord * sr){
	if (NULL != g_sampleRecordQueue){
		return xQueueSend(g_sampleRecordQueue, &sr, SAMPLE_QUEUE_WAIT_TIME);
	}
	else{
		return errQUEUE_EMPTY;
	}
}

static void fileWriteString(FIL *f, char *s){
	f_puts(s,f);
}

static void fileWriteQuotedString(FIL *f, char *s){
	fileWriteString(f,"\"");
	fileWriteString(f,s);
	fileWriteString(f,"\"");
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

static void writeHeaders(FIL *f, SampleRecord *sr){

	int headerCount = 0;
	for (int i = 0; i < SAMPLE_RECORD_CHANNELS;i++){
		ChannelConfig *cfg = (sr->Samples[i].channelConfig);
		if (SAMPLE_DISABLED != cfg->sampleRate){
			if (headerCount++ > 0) fileWriteString(f, ",");
			fileWriteQuotedString(f, cfg->label);
			fileWriteString(f, "|");
			fileWriteQuotedString(f, cfg->units);
			fileWriteString(f, "|");
			fileWriteInt(f, decodeSampleRate(cfg->sampleRate));
		}
	}
	fileWriteString(f,"\n");
}


static void writeSampleRecord(FIL * logfile, SampleRecord * sampleRecord){

	if (NULL == sampleRecord) return;

	int fieldCount = 0;
	for (int i = 0; i < SAMPLE_RECORD_CHANNELS; i++){
		ChannelSample *sample = &(sampleRecord->Samples[i]);
		ChannelConfig * channelConfig = sample->channelConfig;

		if (SAMPLE_DISABLED == channelConfig->sampleRate) continue;

		if (fieldCount++ > 0) fileWriteString(logfile,",");

		if (sample->intValue == NIL_SAMPLE) continue;

		int precision = sample->precision;
		if (precision > 0){
			fileWriteFloat(logfile, sample->floatValue, precision);
		}
		else{
			fileWriteInt(logfile, sample->intValue);
		}
	}
	fileWriteString(logfile,"\n");
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
	pr_debug("open ");
	pr_debug(filename);
	pr_debug("\r\n");
	return rc;
}

void fileWriterTask(void *params){


	SampleRecord *sr = NULL;
	unsigned int flushTimeoutInterval = 0;
	portTickType flushTimeoutStart = 0;
	while(1){
		//wait for the next sample record
		xQueueReceive(g_sampleRecordQueue, &(sr), portMAX_DELAY);

		lock_spi();
		if (NULL != sr && WRITING_INACTIVE == g_writingStatus){
			//start of a new logfile

			int rc = InitFS();
			if (0 != rc){
				pr_error("FS init error\r\n");
				enableLED(LED3);
			}
			else{
				//open next log file
				rc = openNextLogFile(&g_logfile);
				if (0 != rc){
					pr_error("File open error\r\n");
					enableLED(LED3);
				}
				else{
					g_writingStatus = WRITING_ACTIVE;
					writeHeaders(&g_logfile,sr);
					flushTimeoutInterval = FLUSH_INTERVAL_SEC * 1000;
					flushTimeoutStart = xTaskGetTickCount();
				}
			}
			if (0 != rc) g_writingStatus = WRITING_INITIALIZATION_ERROR;
		}

		if (g_writingStatus == WRITING_ACTIVE && NULL != sr){
			//a null sample record means end of sample run; like an EOF
			writeSampleRecord(&g_logfile,sr);
			if (isTimeoutMs(flushTimeoutStart, flushTimeoutInterval)){
				pr_debug("f_sync\r\n");
				f_sync(&g_logfile);
				flushTimeoutStart = xTaskGetTickCount();
			}
		}
		if (NULL == sr){
			if (g_writingStatus == WRITING_ACTIVE){
				pr_debug("f_close\r\n");
				f_close(&g_logfile);
				UnmountFS();
			}
			g_writingStatus = WRITING_INACTIVE;
			disableLED(LED3);
		}
		unlock_spi();
	}
}

void createFileWriterTask(){

	g_writingStatus = WRITING_INACTIVE;
	g_sampleRecordQueue = xQueueCreate(SAMPLE_RECORD_QUEUE_SIZE,sizeof( SampleRecord *));
	if (NULL == g_sampleRecordQueue){
		//TODO log error
		return;
	}
	xTaskCreate( fileWriterTask,( signed portCHAR * ) "fileWriter", FILE_WRITER_STACK_SIZE, NULL, FILE_WRITER_TASK_PRIORITY, NULL );
}
