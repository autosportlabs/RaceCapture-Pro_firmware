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
#include "mem_mang.h"

enum writing_status {
	WRITING_INACTIVE = 0,
	WRITING_ACTIVE = 1,
	WRITING_INITIALIZATION_ERROR = 2
};

static FIL g_logfile;
static xQueueHandle g_sampleRecordQueue = NULL;

#define FILE_WRITER_TASK_PRIORITY				( tskIDLE_PRIORITY + 4 )
#define FILE_WRITER_STACK_SIZE  				200
#define SAMPLE_RECORD_QUEUE_SIZE				10
#define MAX_LOG_FILE_INDEX 						99999
#define FLUSH_INTERVAL_SEC						5

//wait time for sample queue. can be portMAX_DELAY to wait forever, or zero to not wait at all
#define SAMPLE_QUEUE_WAIT_TIME					0

#define WRITE_SUCCESS  0
#define WRITE_FAIL     EOF
#define FILE_WRITE(F,S) f_puts(S, F)

portBASE_TYPE queue_logfile_record(LoggerMessage * msg){
	if (NULL != g_sampleRecordQueue){
		return xQueueSend(g_sampleRecordQueue, &msg, SAMPLE_QUEUE_WAIT_TIME);
	}
	else{
		return errQUEUE_EMPTY;
	}
}

static int write_quoted_string(FIL *f, char *s){
	int rc = FILE_WRITE(f, "\"");
	rc = FILE_WRITE(f, s);
	rc = FILE_WRITE(f, "\"");
	return rc;
}


static int write_int(FIL *f, int num){
	char buf[10];
	modp_itoa10(num,buf);
	return FILE_WRITE(f, buf);
}

static int write_float(FIL *f, float num, int precision){
	char buf[20];
	modp_ftoa(num, buf, precision);
	return FILE_WRITE(f, buf);
}

static int write_headers(FIL *f, SampleRecord *sr){
	int rc = WRITE_SUCCESS;
	int headerCount = 0;
	for (int i = 0; i < SAMPLE_RECORD_CHANNELS;i++){
		ChannelConfig *cfg = (sr->Samples[i].channelConfig);
		if (SAMPLE_DISABLED != cfg->sampleRate){
			if (headerCount++ > 0) rc = FILE_WRITE(f, ",");
			rc = write_quoted_string(f, cfg->label);
			rc = FILE_WRITE(f, "|");
			rc = write_quoted_string(f, cfg->units);
			rc = FILE_WRITE(f, "|");
			rc = write_int(f, decodeSampleRate(cfg->sampleRate));
		}
	}
	rc = FILE_WRITE(f, "\n");
	return rc;
}


static int write_sample_record(FIL *f, SampleRecord * sampleRecord){
	int rc = WRITE_SUCCESS;
	if (NULL != sampleRecord){
		int fieldCount = 0;
		for (int i = 0; i < SAMPLE_RECORD_CHANNELS; i++){
			ChannelSample *sample = &(sampleRecord->Samples[i]);
			ChannelConfig * channelConfig = sample->channelConfig;

			if (SAMPLE_DISABLED == channelConfig->sampleRate) continue;

			if (fieldCount++ > 0) rc = FILE_WRITE(f, ",");

			if (sample->intValue == NIL_SAMPLE) continue;

			int precision = sample->precision;
			if (precision > 0){
				rc = write_float(f, sample->floatValue, precision);
			}
			else{
				rc = write_int(f, sample->intValue);
			}
		}
		rc = FILE_WRITE(f, "\n");
	}
	else{
		pr_debug("null sample record\r\n");
	}
	return rc;
}

static int open_next_logfile(FIL *f){

	char filename[13];
	int i = 0;
	int rc;
	for (; i < MAX_LOG_FILE_INDEX; i++){
		strcpy(filename,"rc_");
		char numBuf[12];
		modp_itoa10(i,numBuf);
		strcat(filename, numBuf);
		strcat(filename, ".log");
		rc = f_open(f,filename, FA_WRITE | FA_CREATE_NEW);
		if ( rc == 0 ) break;
		f_close(f);
	}
	if (i >= MAX_LOG_FILE_INDEX) return -2;
	pr_info("open ");
	pr_info(filename);
	pr_info("\r\n");
	return rc;
}

static void end_logfile(){
	pr_info("close logfile\r\n");
	lock_spi();
	f_close(&g_logfile);
	UnmountFS();
	unlock_spi();
}

static void flush_logfile(FIL *file){
	pr_debug("flush logfile\r\n");
	lock_spi();
	int res = f_sync(file);
	if (0 != res){
		pr_debug_int(res);
		pr_debug("=flush error\r\n");
	}
	unlock_spi();
}

static int open_new_logfile(){
	int status = WRITING_INITIALIZATION_ERROR;
	lock_spi();
	//start of a new logfile
	int rc = InitFS();
	if (0 != rc){
		pr_error("FS init error\r\n");
		enableLED(LED3);
	}
	else{
		//open next log file
		rc = open_next_logfile(&g_logfile);
		if (0 != rc){
			pr_error("File open error\r\n");
			enableLED(LED3);
		}
		else{
			status = WRITING_ACTIVE;
		}
	}
	unlock_spi();
	return status;
}

void fileWriterTask(void *params){
	LoggerMessage *msg = NULL;
	unsigned int flushTimeoutInterval = 0;
	portTickType flushTimeoutStart = 0;
	size_t tick = 0;
	enum writing_status writingStatus = WRITING_INACTIVE;

	while(1){
		while(1){
			//wait for the next sample record
			xQueueReceive(g_sampleRecordQueue, &(msg), portMAX_DELAY);

			if (LOGGER_MSG_START_LOG == msg->messageType && WRITING_INACTIVE == writingStatus){
				disableLED(LED3);
				flushTimeoutInterval = FLUSH_INTERVAL_SEC * 1000;
				flushTimeoutStart = xTaskGetTickCount();
				tick = 0;
				writingStatus = open_new_logfile();
			}

			else if (LOGGER_MSG_END_LOG == msg->messageType){
				pr_debug_int(tick);
				pr_debug(" logfile lines written\r\n");
				break;
			}

			else if (LOGGER_MSG_SAMPLE == msg->messageType && WRITING_ACTIVE == writingStatus){
				lock_spi();
				if (0 == tick){
					write_headers(&g_logfile, msg->sampleRecord);
				}
				int rc = write_sample_record(&g_logfile, msg->sampleRecord);
				unlock_spi();

				if (rc == WRITE_FAIL){
					enableLED(LED3);
					pr_error("Error writing file\r\n");
					break;
				}
				if (isTimeoutMs(flushTimeoutStart, flushTimeoutInterval)){
					flush_logfile(&g_logfile);
					flushTimeoutStart = xTaskGetTickCount();
				}
				tick++;
			}
		}
		end_logfile();
		writingStatus = WRITING_INACTIVE;
	}
}

void createFileWriterTask(){

	g_sampleRecordQueue = xQueueCreate(SAMPLE_RECORD_QUEUE_SIZE,sizeof( SampleRecord *));
	if (NULL == g_sampleRecordQueue){
		pr_error("Could not create sample record queue!");
		return;
	}
	xTaskCreate( fileWriterTask,( signed portCHAR * ) "fileWriter", FILE_WRITER_STACK_SIZE, NULL, FILE_WRITER_TASK_PRIORITY, NULL );
}
