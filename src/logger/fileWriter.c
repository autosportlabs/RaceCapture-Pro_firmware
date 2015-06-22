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

static xQueueHandle g_sampleRecordQueue = NULL;
static FileBuffer fileBuffer = {"", 0};

static int clear_file_buffer() {
        fileBuffer.index = 0;
        fileBuffer.buffer[0] = '\0';
}

static int writeFileBuffer()
{
        //int rc = f_puts(fileBuffer.buffer, g_logfile);
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
        int i;

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


static int write_samples(ChannelSample *sample, size_t channelCount)
{
        if (NULL == sample) {
                pr_debug("Logger: null sample record\r\n");
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
        const int rc = f_open(g_logfile, ls->name, FA_WRITE);
        return rc == FR_OK ? WRITING_ACTIVE : WRITING_INACTIVE;
}

static enum writing_status open_new_log_file(struct logging_status *ls)
{
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

        // Open a file if one is set, else create a new one.
        ls->writing_status = ls->name[0] ? open_existing_log_file(ls) :
                open_new_log_file(ls);

        if (WRITING_INACTIVE == ls->writing_status) {
                pr_warning_str_msg("Logging: Failed to open: ", ls->name);
                return;
        }

        pr_info_str_msg("Logging: Opened " , ls->name);
        ls->flush_tick = xTaskGetTickCount();
        ls->write_tick = 0;
}

TESTABLE_STATIC int logging_start(struct logging_status *ls)
{
        pr_info("Logging: Start\r\n");
        ls->logging = true;

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

TESTABLE_STATIC int logging_sample(struct logging_status *ls,
                                   LoggerMessage *msg)
{
        /* If we haven't starting logging yet, then don't log (duh!) */
        if (!ls->logging)
                return 1;

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
                if (WRITING_ACTIVE != ls->writing_status) {
                        if (0 == ls->write_tick)
                                writeHeaders(msg->channelSamples,
                                             msg->sampleCount);

                        rc = write_samples(msg->channelSamples,
                                           msg->sampleCount);
                        if (0 == rc) {
                                /*
                                 * Here b/c we want headers if the initial
                                 * writing of the log file fails.
                                 */
                                ls->write_tick++;
                                break;
                        }
                }

                pr_error("Remounting FS due to write error.\r\n");
                close_log_file(ls);
        }

        logging_led_toggle();
        return 0 == rc ? 0 : rc + 1;
}

TESTABLE_STATIC int flush_logfile(struct logging_status *ls)
{
        if (ls->writing_status != WRITING_ACTIVE)
                return 1;

        if (!isTimeoutMs(ls->flush_tick, FLUSH_INTERVAL_MS))
                return 2;

        pr_debug("Logging: flush\r\n");
        const int res = f_sync(g_logfile);
        if (0 != res)
                pr_debug_int_msg("Logging: flush err", res);

        ls->flush_tick = xTaskGetTickCount();
        return res + 2;
}

void fileWriterTask(void *params)
{
        LoggerMessage *msg = NULL;
        struct logging_status ls = { 0 };

        while(1) {
                int rc;

                // Get a sample.
                xQueueReceive(g_sampleRecordQueue, &(msg), portMAX_DELAY);

                switch (msg->type) {
                case LoggerMessageType_Sample:
                        rc = logging_sample(&ls, msg);

                        /* Ignore failure if not logging */
                        if (1 == rc)
                                rc = 0;

                        break;
                case LoggerMessageType_Start:
                        rc = logging_start(&ls);
                        break;
                case LoggerMessageType_Stop:
                        rc = logging_stop(&ls);
                        break;
                default:
                        pr_warning("Unsupported message type\r\n");
                        rc = 1;
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
                pr_error("file: logfile sruct err\r\n");
                return;
        }
        xTaskCreate( fileWriterTask,( signed portCHAR * ) "fileWriter",
                     FILE_WRITER_STACK_SIZE, NULL, priority, NULL );
}
