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
//#define SAMPLE_QUEUE_WAIT_TIME					portMAX_DELAY

portBASE_TYPE queue_logfile_record(LoggerMessage * msg){
	if (NULL != g_sampleRecordQueue){
		return xQueueSend(g_sampleRecordQueue, &msg, SAMPLE_QUEUE_WAIT_TIME);
	}
	else{
		return errQUEUE_EMPTY;
	}
}

static void append_quoted_string(char *line, char *s){
	strcat(line, "\"");
	strcat(line, s);
	strcat(line, "\"");
}


static void append_int(char *line, int num){
	char buf[10];
	modp_itoa10(num,buf);
	strcat(line,buf);
}

static void append_float(char *line, float num, int precision){
	char buf[20];
	modp_ftoa(num, buf, precision);
	strcat(line,buf);
}

static char * create_headers(char *line, SampleRecord *sr){

	line[0] = '\0';
	int headerCount = 0;
	for (int i = 0; i < SAMPLE_RECORD_CHANNELS;i++){
		ChannelConfig *cfg = (sr->Samples[i].channelConfig);
		if (SAMPLE_DISABLED != cfg->sampleRate){
			if (headerCount++ > 0) strcat(line, ",");
			append_quoted_string(line, cfg->label);
			strcat(line, "|");
			append_quoted_string(line, cfg->units);
			strcat(line, "|");
			append_int(line, decodeSampleRate(cfg->sampleRate));
		}
	}
	strcat(line, "\n");
	return line;
}


static char * create_sample_record(char *line, SampleRecord * sampleRecord){
	line[0] = '\0';
	if (NULL == sampleRecord){
		pr_debug("null sample record..\r\n");
		return;
	}
	int fieldCount = 0;
	for (int i = 0; i < SAMPLE_RECORD_CHANNELS; i++){
		ChannelSample *sample = &(sampleRecord->Samples[i]);
		ChannelConfig * channelConfig = sample->channelConfig;

		if (SAMPLE_DISABLED == channelConfig->sampleRate) continue;

		if (fieldCount++ > 0) strcat(line, ",");

		if (sample->intValue == NIL_SAMPLE) continue;

		int precision = sample->precision;
		if (precision > 0){
			append_float(line, sample->floatValue, precision);
		}
		else{
			append_int(line, sample->intValue);
		}
	}
	strcat(line,"\n");
	return line;
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

static char * create_sample_line_buffer(){
	//budget 30 characters per channel 'column' plus 1 delimiter.
	//in the future the number of sample records will be dynamic and the LoggerMessage will need to be passed in.
	int bufferSize = SAMPLE_RECORD_CHANNELS * 31;
	pr_debug_int(bufferSize);
	pr_debug("=sample buffer size\r\n");
	char * buffer = (char *)portMalloc(bufferSize);
	return buffer;
}

static void free_sample_line_buffer(char *lineBuffer){
	if (lineBuffer != NULL){
		portFree(lineBuffer);
		pr_debug("free sample buffer\r\n");
	}
}

static void end_logfile(){
	pr_info("close logfile\r\n");
	lock_spi();
	f_close(&g_logfile);
	UnmountFS();
	unlock_spi();
}

static int write_logfile(const char *buffer, FIL *file){
	lock_spi();
	int rc = f_puts(buffer, file);
	unlock_spi();
	return rc;
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
	char *lineBuffer = NULL;

	while(1){
		while(1){
			//wait for the next sample record
			xQueueReceive(g_sampleRecordQueue, &(msg), portMAX_DELAY);

			if (LOGGER_MSG_START_LOG == msg->messageType && WRITING_INACTIVE == writingStatus){
				disableLED(LED3);
				flushTimeoutInterval = FLUSH_INTERVAL_SEC * 1000;
				flushTimeoutStart = xTaskGetTickCount();
				tick = 0;

				lineBuffer = create_sample_line_buffer();
				if (0 == lineBuffer){
					pr_error("Could not create line buffer\r\n");
					enableLED(LED3);
				}
				else{
					writingStatus = open_new_logfile();
				}
			}

			else if (LOGGER_MSG_END_LOG == msg->messageType){
				pr_debug_int(tick);
				pr_debug(" logfile lines written\r\n");
				break;
			}

			else if (LOGGER_MSG_SAMPLE == msg->messageType && WRITING_ACTIVE == writingStatus){
				if (0 == tick){
					write_logfile(create_headers(lineBuffer, msg->sampleRecord), &g_logfile);
				}
				int rc = write_logfile(create_sample_record(lineBuffer, msg->sampleRecord), &g_logfile);

				if (rc == EOF){
					enableLED(LED3);
					pr_error("Error writing file- EOF\r\n");
					break;
				}
				if (isTimeoutMs(flushTimeoutStart, flushTimeoutInterval)){
					flush_logfile(&g_logfile);
					flushTimeoutStart = xTaskGetTickCount();
				}
				tick++;
			}
		}
		free_sample_line_buffer(lineBuffer);
		lineBuffer = NULL;
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
