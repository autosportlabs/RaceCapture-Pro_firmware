/*
 * fileWriter.c
 *
 *  Created on: Feb 29, 2012
 *      Author: brent
 */

#include "LED.h"
#include "fileWriter.h"
#include "loggerHardware.h"
#include "mem_mang.h"
#include "mod_string.h"
#include "modp_numtoa.h"
#include "printk.h"
#include "sampleRecord.h"
#include "sdcard.h"
#include "semphr.h"
#include "task.h"
#include "taskUtil.h"
#include "test.h"

#include <stdbool.h>

#define FILE_WRITER_STACK_SIZE  				200
#define SAMPLE_RECORD_QUEUE_SIZE				20
#define FILE_BUFFER_SIZE						256

#define MAX_LOG_FILE_INDEX 						99999
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
static xQueueHandle g_sampleRecordQueue;
static FileBuffer fileBuffer = {"", 0};


static void clear_file_buffer() {
        fileBuffer.index = 0;
        fileBuffer.buffer[0] = '\0';
}

static int writeFileBuffer()
{
        unsigned int bw;
        const int rc = f_write(g_logfile, fileBuffer.buffer,
                               fileBuffer.index, &bw);
        clear_file_buffer();
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
    if (NULL == g_sampleRecordQueue)
        return errQUEUE_EMPTY;

    return xQueueSend(g_sampleRecordQueue, &msg, SAMPLE_QUEUE_WAIT_TIME);
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

static int write_samples_header(const LoggerMessage *msg)
{
        int i;
        ChannelSample *sample = msg->channelSamples;
        size_t channelCount = msg->sampleCount;

        for (i = 0; 0 < channelCount; channelCount--, sample++, i++) {
                appendFileBuffer(0 == i ? "" : ",");

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


static int write_samples_data(const LoggerMessage *msg)
{
        ChannelSample *sample = msg->channelSamples;
        size_t channelCount = msg->sampleCount;

        if (NULL == sample) {
                pr_warning("Logger: null sample record\r\n");
                return WRITE_FAIL;
        }

        int i;
        for (i = 0; 0 < channelCount; channelCount--, sample++, i++) {
                appendFileBuffer(0 == i ? "" : ",");

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
        return writeFileBuffer();
}

static enum writing_status open_existing_log_file(struct logging_status *ls)
{
        pr_debug_str_msg("Logging: Opening log file ", ls->name);

        int rc = f_open(g_logfile, ls->name, FA_WRITE);

        if (FR_OK != rc)
                return WRITING_INACTIVE;

        // Seek to the end so we append instead of overwriting
        rc = f_lseek(g_logfile, f_size(g_logfile));

        return rc == FR_OK ? WRITING_ACTIVE : WRITING_INACTIVE;
}

static enum writing_status open_new_log_file(struct logging_status *ls)
{
        pr_debug("Logging: Opening new log file\r\n");

        int i;

        for (i = 0; i < MAX_LOG_FILE_INDEX; i++) {
                char buf[12];
                modp_itoa10(i, buf);

                strcpy(ls->name, "rc_");
                strcat(ls->name, buf);
                strcat(ls->name, ".log");

                const FRESULT res = f_open(g_logfile, ls->name,
                                           FA_WRITE | FA_CREATE_NEW);
                if ( FR_OK == res )
                        return WRITING_ACTIVE;

                f_close(g_logfile);
        }

        /* We fail if here. Be sure to clean up name buffer.*/
        ls->name[0] = '\0';
        return WRITING_INACTIVE;
}

static void close_log_file(struct logging_status *ls)
{
        ls->writing_status = WRITING_INACTIVE;
        f_close(g_logfile);
        UnmountFS();
}

static void error_led(bool on)
{
        on ? LED_enable(3) : LED_disable(3);
}

static void logging_led_toggle()
{
        LED_toggle(2);
}

static void logging_led_off()
{
        LED_disable(2);
}

static void open_log_file(struct logging_status *ls)
{
        pr_info("Logging: Opening log file\r\n");
        ls->writing_status = WRITING_INACTIVE;

        const int rc = InitFS();
        if (0 != rc) {
                pr_error_int_msg("Logging: FS init error: ", rc);
                return;
        }

        pr_debug("Logging: FS init success.  Opening file...\r\n");
        // Open a file if one is set, else create a new one.
        ls->writing_status = ls->name[0] ? open_existing_log_file(ls) :
                open_new_log_file(ls);

        if (WRITING_INACTIVE == ls->writing_status) {
                pr_warning_str_msg("Logging: Failed to open: ", ls->name);
                return;
        }

        pr_info_str_msg("Logging: Opened " , ls->name);
        ls->flush_tick = xTaskGetTickCount();
}

TESTABLE_STATIC int logging_start(struct logging_status *ls)
{
        pr_info("Logging: Start\r\n");
        ls->logging = true;

        /* Set this here because this is the start of the log stream */
        ls->rows_written = 0;

        logging_led_toggle();
        return 0;
}

TESTABLE_STATIC int logging_stop(struct logging_status *ls)
{
        pr_debug("Logging: End\r\n");
        ls->logging = false;

        close_log_file(ls);

        /* Prevent log file from being re-opened */
        ls->name[0] = '\0';

        logging_led_off();
        return 0;
}

static int write_samples(struct logging_status *ls, const LoggerMessage *msg)
{
        int rc = 0;

        /* If we haven't written to this file yet, start with the headers */
        if (0 == ls->rows_written) {
                rc = write_samples_header(msg);

                /* If headers written, then don't write them again */
                if (0 == rc)
                        ls->rows_written++;
        }

        /* If the above write failed, then don't bother with the next */
        if (0 != rc)
                return rc;

        rc = write_samples_data(msg);

        if (0 == rc)
                ls->rows_written++;

        return rc;
}

TESTABLE_STATIC int logging_sample(struct logging_status *ls,
                                   LoggerMessage *msg)
{
        /* If we haven't starting logging yet, then don't log (duh!) */
        if (!ls->logging)
                return 0;

        int attempts = 2;
        int rc = WRITE_FAIL;
        while (attempts--) {
                /*
                 * Open the log file if not yet done.  This is good for
                 * both lazy opening and easy recovering of failure as
                 * this method will init the FS.
                 */
                if (WRITING_ACTIVE != ls->writing_status)
                        open_log_file(ls);

                /* Don't try to write if file isn't open */
                if (WRITING_ACTIVE == ls->writing_status) {
                        rc = write_samples(ls, msg);
                        if (0 == rc)
                                break;
                }

                /* If here, then unmount and try attempts more time */
                pr_error("Remounting FS due to write error.\r\n");
                close_log_file(ls);

                /*
                 * We yield here because init/f_open/f_close all involve
                 * locks and can eat up significant resournce.  This ensures
                 * we don't hog resources beyond reason.  Or, you know, you
                 * could keep your damn SD card plugged in.  --Stieg
                 */
                taskYIELD();
        }

        logging_led_toggle();
        return rc;
}

TESTABLE_STATIC int flush_logfile(struct logging_status *ls)
{
        if (ls->writing_status != WRITING_ACTIVE)
                return -1;

        if (!isTimeoutMs(ls->flush_tick, FLUSH_INTERVAL_MS))
                return -2;

        pr_debug("Logging: flush\r\n");
        const int res = f_sync(g_logfile);
        if (0 != res)
                pr_debug_int_msg("Logging: flush err ", res);

        ls->flush_tick = xTaskGetTickCount();
        return res;
}

static void fileWriterTask(void *params)
{
        LoggerMessage *msg = NULL;
        struct logging_status ls;
        memset(&ls, 0, sizeof(struct logging_status));

        while(1) {
                int rc = -1;

                /* Get a sample. */
                xQueueReceive(g_sampleRecordQueue, &(msg), portMAX_DELAY);

                switch (msg->type) {
                case LoggerMessageType_Sample:
                        rc = logging_sample(&ls, msg);
                        break;
                case LoggerMessageType_Start:
                        rc = logging_start(&ls);
                        break;
                case LoggerMessageType_Stop:
                        rc = logging_stop(&ls);
                        break;
                default:
                        pr_warning("Unsupported message type\r\n");
                }

                /* Turns the LED on if things are bad, off otherwise. */
                error_led(rc);
                if (rc) {
                        pr_debug("Msg type ");
                        pr_debug_int(msg->type);
                        pr_debug_int_msg(" failed with code ", rc);
                }

                flush_logfile(&ls);
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

        g_logfile = (FIL *) pvPortMalloc(sizeof(FIL));
        if (NULL == g_logfile) {
                pr_error("file: logfile sruct alloc err\r\n");
                return;
        }
        memset(g_logfile, 0, sizeof(FIL));

        xTaskCreate( fileWriterTask,( signed portCHAR * ) "fileWriter",
                     FILE_WRITER_STACK_SIZE, NULL, priority, NULL );
}
