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
#include "mem_mang.h"
#include "LED.h"
#include "taskUtil.h"

#include <stdbool.h>

enum writing_status {
    WRITING_INACTIVE = 0,
    WRITING_ACTIVE
};

#define FILE_WRITER_STACK_SIZE  				200
#define SAMPLE_RECORD_QUEUE_SIZE				20
#define FILE_BUFFER_SIZE						256

#define FILENAME_LEN							13
#define MAX_LOG_FILE_INDEX 						99999
#define FLUSH_INTERVAL_MS						5000
#define ERROR_SLEEP_DELAY_MS					500

//wait time for sample queue. can be portMAX_DELAY to wait forever, or zero to not wait at all
#define SAMPLE_QUEUE_WAIT_TIME					0

#define WRITE_SUCCESS  0
#define WRITE_FAIL     EOF

typedef struct _FileBuffer {
    char buffer[FILE_BUFFER_SIZE];
    size_t index;
} FileBuffer;

static FIL *g_logfile;
static xQueueHandle g_sampleRecordQueue = NULL;
static FileBuffer fileBuffer = {"", 0};

static int writeFileBuffer()
{
    int rc = f_puts(fileBuffer.buffer, g_logfile);
    fileBuffer.index = 0;
    fileBuffer.buffer[0] = '\0';
    return rc;
}

static void appendFileBuffer(const char * data)
{
    size_t index = fileBuffer.index;
    char * buffer = fileBuffer.buffer + index;

    while(*data) {
        *buffer++ = *data++;
        index++;
        if (index >= FILE_BUFFER_SIZE) {
            *buffer = '\0';
            writeFileBuffer();
            index = fileBuffer.index;
            buffer = fileBuffer.buffer + index;
        }
    }
    *buffer = '\0';
    fileBuffer.index = index;
}

portBASE_TYPE queue_logfile_record(LoggerMessage * msg)
{
    if (NULL != g_sampleRecordQueue) {
        return xQueueSend(g_sampleRecordQueue, &msg, SAMPLE_QUEUE_WAIT_TIME);
    } else {
        return errQUEUE_EMPTY;
    }
}

static void appendQuotedString(const char *s)
{
    appendFileBuffer("\"");
    appendFileBuffer(s);
    appendFileBuffer("\"");
}

static void appendInt(int num)
{
    char buf[12];
    modp_itoa10(num,buf);
    appendFileBuffer(buf);
}

static void appendLongLong(long long num)
{
    char buf[21];
    modp_ltoa10(num, buf);
    appendFileBuffer(buf);
}

static void appendDouble(double num, int precision)
{
    char buf[30];
    modp_dtoa(num, buf, precision);
    appendFileBuffer(buf);
}

static void appendFloat(float num, int precision)
{
    char buf[11];
    modp_ftoa(num, buf, precision);
    appendFileBuffer(buf);
}

static int writeHeaders(ChannelSample *sample, size_t channelCount)
{
    char *separator = "";

    for (; 0 < channelCount; channelCount--, sample++) {
        appendFileBuffer(separator);
        separator = ",";

        uint8_t precision = sample->cfg->precision;
        appendQuotedString(sample->cfg->label);
        appendFileBuffer("|");
        appendQuotedString(sample->cfg->units);
        appendFileBuffer("|");
        appendFloat(decodeSampleRate(sample->cfg->min), precision);
        appendFileBuffer("|");
        appendFloat(decodeSampleRate(sample->cfg->max), precision);
        appendFileBuffer("|");
        appendInt(decodeSampleRate(sample->cfg->sampleRate));
    }

    appendFileBuffer("\n");
    return writeFileBuffer();
}


static int writeChannelSamples(ChannelSample *sample, size_t channelCount)
{
    if (NULL == sample) {
        pr_debug("fileWriter: null sample record\r\n");
        return WRITE_FAIL;
    }

    char *separator = "";
    for (; 0 < channelCount; channelCount--, sample++) {
        appendFileBuffer(separator);
        separator = ",";

        if (!sample->populated)
            continue;

        const int precision = sample->cfg->precision;

        switch(sample->sampleData) {
        case SampleData_Float:
        case SampleData_Float_Noarg:
            appendFloat(sample->valueFloat, precision);
            break;
        case SampleData_Int:
        case SampleData_Int_Noarg:
            appendInt(sample->valueInt);
            break;
        case SampleData_LongLong:
        case SampleData_LongLong_Noarg:
            appendLongLong(sample->valueLongLong);
            break;
        case SampleData_Double:
        case SampleData_Double_Noarg:
            appendDouble(sample->valueDouble, precision);
            break;
        default:
            pr_warning("file: Unknown channel sample type\n");
        }
    }

    appendFileBuffer("\n");
    writeFileBuffer();

    return WRITE_SUCCESS;
}

static int openLogfile(FIL *f, const char *filename)
{
    int rc = f_open(f,filename, FA_WRITE);
    return rc;
}

static int openNextLogfile(FIL *f, char *filename)
{
    int i = 0;
    int rc;
    for (; i < MAX_LOG_FILE_INDEX; i++) {
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
    pr_info_str_msg("Open: " , filename);
    return rc;
}

static void endLogfile()
{
    pr_info("fileWriter: close\r\n");
    f_close(g_logfile);
    UnmountFS();
}

static void flushLogfile(FIL *file)
{
    pr_debug("fileWriter: flush\r\n");
    int res = f_sync(file);
    if (0 != res) {
        pr_debug_int_msg("flush err:", res);
    }
}

static void error_led(bool on) {
        on ? LED_enable(3) : LED_disable(3);
}

static void logging_led_toggle() {
        LED_toggle(2);
}

static void logging_led_off() {
        LED_disable(2);
}

static enum writing_status openNewLogfile(char *filename)
{
    int rc = InitFS();

    if (0 != rc) {
        pr_error_int_msg("FS init error: ", rc);
        return WRITING_INACTIVE;
    }

    //open next log file
    rc = openNextLogfile(g_logfile, filename);
    if (0 != rc) {
        pr_error_int_msg("File open err: ", rc);
        return WRITING_INACTIVE;
    }

    return WRITING_ACTIVE;
}

struct file_status
{
        portTickType flush_tick;
        portTickType write_tick;
        bool logging;
        enum writing_status writing_status;
        char name[FILENAME_LEN];
};

static int logging_start(struct file_status *fs)
{
        if (fs->writing_status != WRITING_INACTIVE)
                return 2;

        pr_debug("fileWriter: start\r\n");

        fs->flush_tick = xTaskGetTickCount();
        fs->write_tick = 0;
        fs->writing_status = openNewLogfile(fs->name);
        fs->logging = true;

        return fs->writing_status == WRITING_ACTIVE ? 0 : 1;
}

static int logging_stop(struct file_status *fs)
{
        endLogfile();
        fs->writing_status = WRITING_INACTIVE;
        fs->logging = false;

        logging_led_off();
        return 0;
}

static bool remount_log_file(const struct file_status *fs)
{
        pr_error("Remounting FS due to write error.\r\n");
        // XXX: Do we need to worry about rolling over to new files?

        f_close(g_logfile);
        UnmountFS();
        InitFS();

        return openLogfile(g_logfile, fs->name) == 0;
}

static int logging_sample(struct file_status *fs, LoggerMessage *msg)
{
        /* If we haven't starting logging yet, then no op */
        if (!fs->logging)
                return 0;

        if (fs->writing_status != WRITING_ACTIVE)
                return 1;

        if (0 == fs->write_tick)
                writeHeaders(msg->channelSamples, msg->sampleCount);

        fs->write_tick++;
        int attempts = 2;
        int rc = WRITE_FAIL;
        while (attempts--) {
                rc = writeChannelSamples(msg->channelSamples,
                                         msg->sampleCount);
                if (rc == WRITE_SUCCESS)
                        break;

                if (attempts)
                        remount_log_file(fs);
        }

        logging_led_toggle();
        return rc == WRITE_SUCCESS ? 0 : 2;
}

static void flush_logfile(struct file_status *fs)
{
        if (fs->writing_status != WRITING_ACTIVE)
                return;

        if (!isTimeoutMs(fs->flush_tick, FLUSH_INTERVAL_MS))
                return;

        flushLogfile(g_logfile);
        fs->flush_tick = xTaskGetTickCount();
}

void fileWriterTask(void *params)
{
        LoggerMessage *msg = NULL;
        struct file_status fs = { 0 };

        while(1) {
                int rc;

                // Get a sample.
                xQueueReceive(g_sampleRecordQueue, &(msg), portMAX_DELAY);

                switch (msg->type) {
                case LoggerMessageType_Sample:
                        rc = logging_sample(&fs, msg);
                        break;
                case LoggerMessageType_Start:
                        rc = logging_start(&fs);
                        break;
                case LoggerMessageType_Stop:
                        rc = logging_stop(&fs);
                        break;
                default:
                        pr_warning("Unsupported message type\r\n");
                        rc = 1;
                        break;
                }

                /* Turns the LED on if things are bad, off otherwise. */
                error_led(rc);
                if (rc) {
                        pr_debug("fileWriter: Msg type ");
                        pr_debug_int(msg->type);
                        pr_debug_int_msg(" failed with code ", rc);
                }

                flush_logfile(&fs);
        }
}

void startFileWriterTask( int priority )
{
        g_sampleRecordQueue = xQueueCreate(SAMPLE_RECORD_QUEUE_SIZE,
                                           sizeof( ChannelSample *));

        if (NULL == g_sampleRecordQueue) {
                pr_error("file: sampleRecordQueue err\r\n");
                return;
        }

        g_logfile = pvPortMalloc(sizeof(FIL));
        if (NULL == g_logfile) {
                pr_error("file: logfile sruct err\r\n");
                return;
        }
        xTaskCreate( fileWriterTask,( signed portCHAR * ) "fileWriter",
                     FILE_WRITER_STACK_SIZE, NULL, priority, NULL );
}
