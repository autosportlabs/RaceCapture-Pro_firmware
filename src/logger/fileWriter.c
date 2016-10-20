/*
 * Race Capture Firmware
 *
 * Copyright (C) 2016 Autosport Labs
 *
 * This file is part of the Race Capture firmware suite
 *
 * This is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details. You should
 * have received a copy of the GNU General Public License along with
 * this code. If not, see <http://www.gnu.org/licenses/>.
 */


#include "fileWriter.h"
#include "led.h"
#include "loggerHardware.h"
#include "macros.h"
#include "mem_mang.h"
#include "modp_numtoa.h"
#include "printk.h"
#include "ring_buffer.h"
#include "sampleRecord.h"
#include "sdcard.h"
#include "task.h"
#include "taskUtil.h"
#include "test.h"
#include <stdbool.h>
#include <string.h>
#include <string.h>

#define ERROR_SLEEP_DELAY_MS	500
#define FILE_BUFFER_SIZE	1024
#define FILE_WRITER_STACK_SIZE	512
#define LOG_PFX	"[fileWriter] "
#define MAX_LOG_FILE_INDEX	99999
#define WRITE_FAIL	EOF

static FIL *g_logfile;
static xQueueHandle g_LoggerMessage_queue;
static struct ring_buff *file_buff;

static void error_led(const bool on)
{
        led_set(LED_ERROR, on);
}

static FRESULT flush_file_buffer(void)
{
        while(true) {
		size_t available = 0;
		const void* buff =
			ring_buffer_dma_read_init(file_buff, &available);

		/* If nothing to write, we are done. */
		if (!available)
			return FR_OK;

		unsigned int written = 0;
		const FRESULT res =
			f_write(g_logfile, buff, available, &written);

		ring_buffer_dma_read_fini(file_buff, written);
		if (FR_OK != res) {
			pr_debug_int_msg("[FileWriter] f_write failed "
					 "with status: ", (int) res);
			error_led(true);
			return res;
		}
        }
}

static FRESULT append_file_buffer(const char *str)
{
        if (!str)
                return FR_OK;

        FRESULT res = FR_OK;
        size_t len = strlen(str);
        while(len) {
                const size_t write_len =
                        MIN(ring_buffer_bytes_free(file_buff), len);
                ring_buffer_put(file_buff, str, write_len);
                str += write_len;
                len -= write_len;

                /* If not at end of string, more to write.  Flush */
                if (len > 0)
                        res = flush_file_buffer();
        }

        return res;
}

portBASE_TYPE queue_logfile_record(const LoggerMessage * const msg)
{
        return send_logger_message(g_LoggerMessage_queue, msg);
}

static void appendQuotedString(const char *s)
{
        append_file_buffer("\"");
        append_file_buffer(s);
        append_file_buffer("\"");
}

static void appendInt(int num)
{
        char buf[12];
        modp_itoa10(num, buf);
        buf[11] = '\0';
        append_file_buffer(buf);
}

static void appendLongLong(long long num)
{
        char buf[24];
        modp_ltoa10(num, buf);
        buf[23] = '\0';
        append_file_buffer(buf);
}

static void appendDouble(double num, int precision)
{
        char buf[32];
        modp_dtoa(num, buf, precision);
        buf[31] = '\0';
        append_file_buffer(buf);
}

static void appendFloat(float num, int precision)
{
        char buf[16];
        modp_ftoa(num, buf, precision);
        buf[15] = '\0';
        append_file_buffer(buf);
}

static int write_samples_header(const LoggerMessage *msg)
{
        int i;
        const ChannelSample *sample = msg->sample->channel_samples;
        size_t count = msg->sample->channel_count;

        for (i = 0; 0 < count; count--, sample++, i++) {
                append_file_buffer(0 == i ? "" : ",");

                uint8_t precision = sample->cfg->precision;
                appendQuotedString(sample->cfg->label);
                append_file_buffer("|");
                appendQuotedString(sample->cfg->units);
                append_file_buffer("|");
                appendFloat(sample->cfg->min, precision);
                append_file_buffer("|");
                appendFloat(sample->cfg->max, precision);
                append_file_buffer("|");
                appendInt(decodeSampleRate(sample->cfg->sampleRate));
        }

        append_file_buffer("\n");
        return flush_file_buffer();
}


static int write_samples_data(const LoggerMessage *msg)
{
        const ChannelSample *sample = msg->sample->channel_samples;
        size_t count = msg->sample->channel_count;

        if (NULL == sample) {
                pr_warning(_RCP_BASE_FILE_ "null sample record\r\n");
                return WRITE_FAIL;
        }

        int i;
        for (i = 0; 0 < count; count--, sample++, i++) {
                append_file_buffer(0 == i ? "" : ",");

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
                        pr_warning(_RCP_BASE_FILE_ "Unknown channel "
                                   "sample type\n");
                }
        }

        append_file_buffer("\n");
        return flush_file_buffer();
}

static enum writing_status open_existing_log_file(struct logging_status *ls)
{
        pr_debug_str_msg(_RCP_BASE_FILE_ "Opening log file ", ls->name);

        int rc = f_open(g_logfile, ls->name, FA_WRITE);

        if (FR_OK != rc)
                return WRITING_INACTIVE;

        // Seek to the end so we append instead of overwriting
        rc = f_lseek(g_logfile, f_size(g_logfile));

        return rc == FR_OK ? WRITING_ACTIVE : WRITING_INACTIVE;
}

static enum writing_status open_new_log_file(struct logging_status *ls)
{
        pr_debug(_RCP_BASE_FILE_ "Opening new log file\r\n");

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

static void logging_led_toggle(void)
{
        led_toggle(LED_LOGGER);
}

static void logging_led_off(void)
{
        led_disable(LED_LOGGER);
}

static void open_log_file(struct logging_status *ls)
{
        pr_info(_RCP_BASE_FILE_ "Opening log file\r\n");
        ls->writing_status = WRITING_INACTIVE;

        const int rc = InitFS();
        if (0 != rc) {
                pr_error_int_msg(_RCP_BASE_FILE_ "FS init error: ", rc);
                return;
        }

        pr_debug(_RCP_BASE_FILE_ "FS init success.  Opening file...\r\n");
        // Open a file if one is set, else create a new one.
        ls->writing_status = ls->name[0] ? open_existing_log_file(ls) :
                open_new_log_file(ls);

        if (WRITING_INACTIVE == ls->writing_status) {
                pr_warning_str_msg(_RCP_BASE_FILE_ "Failed to open: ", ls->name);
                return;
        }

        pr_info_str_msg(_RCP_BASE_FILE_ "Opened " , ls->name);
        ls->flush_tick = xTaskGetTickCount();
	ls->last_sample_tick = 0;
}

TESTABLE_STATIC int logging_start(struct logging_status *ls)
{
        pr_info(_RCP_BASE_FILE_ "Start\r\n");
        ls->logging = true;

        /* Set this here because this is the start of the log stream */
        ls->rows_written = 0;

        logging_led_toggle();
        return 0;
}

TESTABLE_STATIC int logging_stop(struct logging_status *ls)
{
        pr_debug(_RCP_BASE_FILE_ "End\r\n");
        ls->logging = false;

        close_log_file(ls);

        /* Prevent log file from being re-opened */
        ls->name[0] = '\0';

        logging_led_off();
        return 0;
}

static int write_samples(struct logging_status *ls, const LoggerMessage *msg)
{
	/* Ensure the LoggerMessage we are writing is valid */
	if (!is_sample_data_valid(msg)) {
		pr_warning(LOG_PFX "Sample invalid.  Skipping...\r\n");
		return 0;
	}

	/* Ensure that we don't write a sample that is older than previous */
	if (msg->ticks < ls->last_sample_tick) {
		pr_debug(LOG_PFX "Sample is too old.  Skipping...\r\n");
		return 0;
	}
	ls->last_sample_tick = msg->ticks;

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
                pr_error(_RCP_BASE_FILE_ "Remounting FS due to write "
                         "error.\r\n");
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

        pr_debug(_RCP_BASE_FILE_ "flush\r\n");
        const int res = f_sync(g_logfile);
        if (0 != res)
                pr_debug_int_msg(_RCP_BASE_FILE_ "flush err ", res);

        ls->flush_tick = xTaskGetTickCount();
        return res;
}

static void fileWriterTask(void *params)
{
        LoggerMessage msg;
        struct logging_status ls;
        memset(&ls, 0, sizeof(struct logging_status));

        while(1) {
                int rc = -1;

                /* Get a sample. */
                const char status = receive_logger_message(g_LoggerMessage_queue,
                                                           &msg, portMAX_DELAY);

                /* If we fail to receive for any reason, keep trying */
                if (pdPASS != status)
                   continue;

                switch (msg.type) {
                case LoggerMessageType_Sample:
                        rc = logging_sample(&ls, &msg);
                        break;
                case LoggerMessageType_Start:
                        rc = logging_start(&ls);
                        break;
                case LoggerMessageType_Stop:
                        rc = logging_stop(&ls);
                        break;
                default:
                        pr_warning(_RCP_BASE_FILE_ "Unsupported message "
                                   "type\r\n");
                }

                /* Turns the LED on if things are bad, off otherwise. */
                error_led(rc);
                if (rc) {
                        pr_debug(_RCP_BASE_FILE_ "Msg type ");
                        pr_debug_int(msg.type);
                        pr_debug_int_msg(" failed with code ", rc);
                }

                flush_logfile(&ls);
        }
}

void startFileWriterTask(int priority)
{
        g_LoggerMessage_queue = create_logger_message_queue();

        if (NULL == g_LoggerMessage_queue) {
                pr_error(_RCP_BASE_FILE_ "LoggerMessage Queue is null!\r\n");
                return;
        }

        g_logfile = (FIL *) portMalloc(sizeof(FIL));
        if (NULL == g_logfile) {
                pr_error(_RCP_BASE_FILE_ "logfile sruct alloc err\r\n");
                return;
        }
        memset(g_logfile, 0, sizeof(FIL));

        file_buff = ring_buffer_create(FILE_BUFFER_SIZE);
        if (!file_buff) {
                pr_error(_RCP_BASE_FILE_ "Failed to alloc ring buffer.\r\n");
                return;
        }

        /* Make all task names 16 chars including NULL char */
        static const signed portCHAR task_name[] = "File Task       ";
        xTaskCreate(fileWriterTask, task_name, FILE_WRITER_STACK_SIZE,
                    NULL, priority, NULL );
}
