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


#include "printk.h"
#include "sdcard.h"
#include <string.h>
#include "modp_numtoa.h"
#include "FreeRTOS.h"
#include "task.h"
#include "taskUtil.h"
#include "loggerHardware.h"
#include "watchdog.h"
#include "diskio.h"
#include "sdcard_device.h"
#include "mem_mang.h"
#include "semphr.h"

static FATFS *fat_fs = NULL;
static xSemaphoreHandle fs_mutex = NULL;

#define SD_TEST_PATTERN "0123456789"

void InitFSHardware(void)
{
        disk_init_hardware();

        if (fat_fs == NULL)
                fat_fs = pvPortMalloc(sizeof(FATFS));

        if (fat_fs == NULL)
                pr_error("sdcard: FatFS init fail\r\n");

        fs_mutex = xSemaphoreCreateMutex();
}

void fs_lock(void){
        xSemaphoreTake(fs_mutex, portMAX_DELAY);
}

void fs_unlock(void){
        xSemaphoreGive(fs_mutex);
}

static bool is_initialized()
{
        if (fat_fs)
                return true;

        return false;
}

bool sdcard_present(void)
{
        return sdcard_device_card_present();
}

bool sdcard_fs_mounted(void)
{
        char label[12]; /* max label is 11 bytes */
        DWORD vsn;
        return sdcard_present() && f_getlabel("0", label, &vsn) == FR_OK;
}

int InitFS()
{
        if(!is_initialized())
                return -1;

        if(!sdcard_device_card_present()) {
                return -1;
        }

        return f_mount(fat_fs, "0", 1);
}

int UnmountFS()
{
        if(!is_initialized())
                return -1;

        return f_mount(NULL, "0", 1);
}

bool test_sd(struct Serial *serial, int lines, int doFlush, int quiet) {
        int res = 0;
        FIL *fatFile = NULL;
        char buffer[strlen(SD_TEST_PATTERN) + 2];

        if(!is_initialized())
                return false;

        fatFile = pvPortMalloc(sizeof(FIL));
        if (NULL == fatFile) {
                if (!quiet) serial_write_s(serial, "could not allocate file object\r\n");
                goto exit;
        }

        if (!quiet) {
                serial_write_s(serial,"Test Write: Lines: ");
                put_int(serial, lines);
                put_crlf(serial);
                serial_write_s(serial,"Periodic flushing enabled: " );
                serial_write_s(serial, doFlush ? "YES" : "NO");
                put_crlf(serial);
        }

        if (!quiet)
                serial_write_s(serial,"Card Init... ");

        bool fs_good = sdcard_fs_mounted();
        if (!fs_good) {
                FRESULT init_rc = InitFS();
                if (!quiet)
                        pr_info_int_msg(" Remounting filesystem: ", init_rc);
                fs_good = (init_rc == FR_OK);
        }
        if (!quiet) {
                serial_write_s(serial, fs_good ? "WIN" : "FAIL");
                put_crlf(serial);
        }

        if (!fs_good){
                res = -1;
                goto exit;
        }

        if (!quiet)
                serial_write_s(serial,"Opening File... ");
        res = f_open(fatFile,"test1.txt", FA_READ | FA_WRITE | FA_CREATE_ALWAYS);
        if (!quiet) {
                serial_write_s(serial, res == 0? "WIN" : "FAIL");
                put_crlf(serial);
        }

        if (res)
                goto exit;

        if (!quiet)
                serial_write_s(serial, "Writing file...  ");

        portTickType startTicks = xTaskGetTickCount();
        for (int i = 1; i <= lines; i++) {
                res = f_puts(SD_TEST_PATTERN "\r\n",fatFile);
                if (doFlush) f_sync(fatFile);
                if (!quiet) {
                        serial_write_s(serial, "\b");
                        serial_write_s(serial, i % 2 == 0 ? "O" : "o");
                }
                if (res == EOF) {
                        if (!quiet){
                                serial_write_s(serial, "failed writing at line ");

                                put_int(serial, i);
                                serial_write_s(serial,"(");
                                put_int(serial, res);
                                serial_write_s(serial,")");
                                put_crlf(serial);
                        }
                        goto exit;
                }
                watchdog_reset();
        }
        if (!quiet)
                serial_write_s(serial, "\b WIN\r\n");

        portTickType endTicks = xTaskGetTickCount();

        if (!quiet) {
                serial_write_s(serial,"time to write: ");
                put_int(serial, ticksToMs(endTicks - startTicks));
                serial_write_s(serial, " ms");
                put_crlf(serial);
        }

        //f_sync(fatFile);

        if (!quiet)
                serial_write_s(serial, "Validating... ");
        FRESULT fseek_res = f_lseek(fatFile, 0);
        if (FR_OK != fseek_res) {
                serial_write_s(serial, "SEEK FAILED");
                put_crlf(serial);
                goto exit;
        }

        char * buffer_result = NULL;
        bool validate_success = true;
        for (size_t i = 0; i < lines; i++){
                buffer_result = f_gets(buffer, sizeof(buffer), fatFile);
                if (strstr(SD_TEST_PATTERN, buffer_result) != 0) {
                        validate_success = false;
                        break;
                }
                if (!quiet) {
                        serial_write_s(serial, "\b");
                        serial_write_s(serial, i % 2 == 0 ? "O" : "o");
                }
        }
        if (!quiet) {
                serial_write_s(serial, "\b ");
                serial_write_s(serial, validate_success ? "WIN" : "FAIL");
                put_crlf(serial);
        }
        res = validate_success ? 0 : -1;
        if (res)
                goto exit;

        if (!quiet) {
                serial_write_s(serial,"Closing... ");
        }
        res = f_close(fatFile);

        if (!quiet) {
                put_int(serial, res);
                put_crlf(serial);
        }
exit:
        if (!quiet) {
                serial_write_s(serial, "Test write overall: ");
                serial_write_s(serial, res== 0 ? "WIN":"FAIL");
                put_crlf(serial);
        }

        if (fatFile != NULL)
                vPortFree(fatFile);

        return res == 0 ? true : false;
}

void test_sd_interactive(struct Serial *serial, int lines, int doFlush, int quiet)
{
        fs_lock();
        bool result = test_sd(serial, lines, doFlush, quiet);
        if (quiet)
                put_int(serial, result);
        fs_unlock();
}


#define MAX_BITMAPS 10

static void fs_write_sample_meta(FIL *buffer_file, const struct sample *sample,
                              int sampleRateLimit, char * buf, bool more)
{
        f_puts("\"meta\":[", buffer_file);
        ChannelSample *channel_sample = sample->channel_samples;

        for (size_t i = 0; i < sample->channel_count; ++i, ++channel_sample) {
                ChannelConfig *cfg = channel_sample->cfg;
                if (0 < i)
                        f_puts(",", buffer_file);

                f_puts("{", buffer_file);


                f_puts("\"nm\":\"", buffer_file);
                f_puts(cfg->label, buffer_file);

                f_puts("\",\"ut\":\"", buffer_file);
                f_puts(cfg->units, buffer_file);

                f_puts("\",\"min\":", buffer_file);
                modp_ftoa(cfg->min, buf, cfg->precision);
                f_puts(buf, buffer_file);

                f_puts(",\"max\":", buffer_file);
                modp_ftoa(cfg->max, buf, cfg->precision);
                f_puts(buf, buffer_file);

                f_puts(",\"prec\":", buffer_file);
                modp_itoa10((int)cfg->precision, buf);
                f_puts(buf, buffer_file);

                f_puts(",\"sr\":", buffer_file);
                modp_itoa10(decodeSampleRate(cfg->sampleRate), buf);
                f_puts(buf, buffer_file);

                f_puts("}", buffer_file);
        }
        f_puts(more ? "]," : "]", buffer_file);
}

void fs_write_sample_record(FIL *buffer_file,
                            const struct sample *sample,
                            const unsigned int tick, const int sendMeta)
{
        char buf[30];
        f_puts("{\"s\":{\"t\":", buffer_file);

        modp_uitoa10(tick, buf);
        f_puts(buf, buffer_file);
        f_puts(",", buffer_file);
        if (sendMeta)
                fs_write_sample_meta(buffer_file, sample, getConnectivitySampleRateLimit(), buf, true);

        size_t channelBitmaskIndex = 0;
        unsigned int channelBitmask[MAX_BITMAPS];
        memset(channelBitmask, 0, sizeof(channelBitmask));

        f_puts("\"d\":[", buffer_file);
        ChannelSample *cs = sample->channel_samples;

        size_t channelBitPosition = 0;
        for (size_t i = 0; i < sample->channel_count;
             i++, channelBitPosition++, cs++) {

                if (channelBitPosition > 31) {
                        channelBitmaskIndex++;
                        channelBitPosition=0;
                        if (channelBitmaskIndex > MAX_BITMAPS)
                                break;
                }

                if (cs->populated) {
                        channelBitmask[channelBitmaskIndex] |=
                                (1 << channelBitPosition);

                        const int precision = cs->cfg->precision;
                        switch(cs->sampleData) {
                        case SampleData_Float:
                        case SampleData_Float_Noarg:
                                modp_ftoa(cs->valueFloat, buf, precision);
                                f_puts(buf, buffer_file);
                                break;
                        case SampleData_Int:
                        case SampleData_Int_Noarg:
                                modp_itoa10(cs->valueInt, buf);
                                f_puts(buf, buffer_file);
                                break;
                        case SampleData_LongLong:
                        case SampleData_LongLong_Noarg:
                                modp_ltoa10(cs->valueLongLong, buf);
                                f_puts(buf, buffer_file);
                                break;
                        case SampleData_Double:
                        case SampleData_Double_Noarg:
                                modp_dtoa(cs->valueDouble, buf, precision);
                                f_puts(buf, buffer_file);
                                break;
                        default:
                                pr_warning_int_msg("[sdcard] Unknown sample"
                                                   " data type: ",
                                                   cs->sampleData);
                                break;
                        }
                        f_puts(",", buffer_file);
                }
        }

        size_t channelBitmaskCount = channelBitmaskIndex + 1;
        for (size_t i = 0; i < channelBitmaskCount; i++) {
                modp_uitoa10(channelBitmask[i], buf);
                f_puts(buf, buffer_file);
                if (i < channelBitmaskCount - 1)
                        f_puts(",", buffer_file);
        }
        f_puts("]}}\r\n", buffer_file);
}

