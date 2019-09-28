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


#define LOG_PFX "[SDCARD] "
static FATFS *fat_fs = NULL;
static xSemaphoreHandle fs_mutex = NULL;
#define SD_MOUNT_RETRY_TIME_MS 1000
#define SD_MOUNT_RETRY_ATTEMPTS 100

static uint32_t re_mount_retries = SD_MOUNT_RETRY_ATTEMPTS;
static uint32_t re_mount_timeout = 0;
static uint32_t last_mount_attempt = 0;
static bool     write_ready = false;


bool sdcard_write_ready( const bool no_delay )
{
	pr_trace( LOG_PFX "sdcard_write_ready\r\n" );
	if ( fs_mutex == NULL )
	{
		pr_trace( "no mutex\r\n" );
		return false;
	}
        if ( sdcard_ready( no_delay ) && ! write_ready )
        {
            write_ready = test_sd( NULL, 1, 0, 1 );
            if (!write_ready && !re_mount_retries)
                    pr_error(LOG_PFX "write verification failed. Try re-formatting SD card\r\n" );
        }
        return write_ready;
}

bool sdcard_ready( const bool no_delay )
{
	if ( fs_mutex == NULL )
	{
		pr_trace( "no mutex\r\n" );
		return false;
	}
	pr_trace( LOG_PFX "sdcard_ready\r\n" );
        bool fs_good = sdcard_fs_mounted();
        if ( ! fs_good )
        {
                pr_trace( LOG_PFX "not mounted\r\n" );
                write_ready = false;
                if ( no_delay || isTimeoutMs( last_mount_attempt, re_mount_timeout ) )
                {
                        last_mount_attempt = getCurrentTicks();
                        pr_trace(LOG_PFX "Remounting filesystem:\r\n" );
                        fs_good = FR_OK == InitFS();

                        if ( ! fs_good )
                        {
                                re_mount_retries--;
                                pr_error(LOG_PFX "Error Initializing filesystem:\r\n" );
                                if ( ! re_mount_retries )
                                {
                                        pr_error( LOG_PFX "Error Initializing filesystem:\r\n" );
                                }
                                re_mount_timeout = SD_MOUNT_RETRY_TIME_MS;
                        }
                        else
                        {
                                pr_info( LOG_PFX "Filesystem initialized\r\n" );
                        }
                }
		if ( fs_good )
		{
			pr_trace( LOG_PFX "Filesystem sucessfully initialized\r\n" );
		}

        }
	else
	{
		pr_trace( LOG_PFX "Filesystem already initialized\r\n" );
	}
        return fs_good;
}

FRESULT sd_open( FIL* file, const TCHAR* path, BYTE mode )
{
	pr_info_str_msg( LOG_PFX "sd_open: ", path );
	fs_lock();
	const FRESULT rc = f_open( file, path, mode );
	fs_unlock();
        if ( rc != FR_OK )
        {
                pr_error( path );
                pr_error_int_msg(LOG_PFX "Error opening file: ", rc);
        }
        else
        {
                pr_trace_str_msg( LOG_PFX "file opened: ", path );
        }
	return rc;
}

FRESULT sd_close( FIL* file )
{
	fs_lock();
	const FRESULT rc = f_close( file );
	fs_unlock();
        if ( rc != FR_OK )
                pr_error_int_msg(LOG_PFX "Error closing file: ", rc);
	return rc;
}

FRESULT sd_write( FIL* file, const void* buff, UINT btr, UINT* br )
{
	fs_lock();
	const FRESULT rc = f_write( file, buff, btr, br );
	fs_unlock();
        if ( rc != FR_OK )
                pr_error_int_msg(LOG_PFX "Error writing file: ", rc);
	return rc;
}

FRESULT sd_read( FIL* file, void* buff, UINT btr, UINT* br )
{
	fs_lock();
	const FRESULT rc = f_read( file, buff, btr, br );
	fs_unlock();
        if ( rc != FR_OK )
                pr_error_int_msg(LOG_PFX "Error reading file: ", rc);
	return rc;
}

FRESULT sd_lseek( FIL* file, DWORD ofs )
{
	fs_lock();
	const FRESULT rc = f_lseek( file, ofs );
	fs_unlock();
        if ( rc != FR_OK )
                pr_error_int_msg(LOG_PFX "Error failed to seek to end of file: ", rc);
	return rc;
}

FRESULT sd_sync( FIL* file )
{
	fs_lock();
	const FRESULT rc = f_sync( file );
	fs_unlock();
        if ( rc != FR_OK )
                pr_error_int_msg(LOG_PFX "Failed to sync buffer file: ", rc);
	return rc;
}

TCHAR*  sd_gets( TCHAR* buff, int len, FIL* fp )
{
	fs_lock();
	TCHAR* rc = f_gets( buff, len, fp );
	fs_unlock();
	return rc;
}

FRESULT sd_truncate( FIL* file )
{
	fs_lock();
	const FRESULT rc = f_truncate( file );
	fs_unlock();
        if ( rc != FR_OK )
                pr_error_int_msg(LOG_PFX "Error truncating file: ", rc);
	return rc;
}

DWORD sd_size( FIL* file )
{
	fs_lock();
        DWORD ret = f_size( file );
        fs_unlock();
        return ret;
}

FRESULT sd_getlabel (const TCHAR* path, TCHAR* label, DWORD* vsn)
{
	fs_lock();
	const FRESULT rc = f_getlabel(path, label, vsn);
	fs_unlock();
        if ( rc != FR_OK )
                    pr_error_int_msg(LOG_PFX "Error getting label: ", rc);

	return rc;
}

FRESULT sd_mount(FATFS* fs, const TCHAR* path, BYTE opt) 
{
	pr_trace( LOG_PFX "sd_mount\r\n" );
	fs_lock();
	const FRESULT rc = f_mount (fs, path, opt);
	fs_unlock();
        if ( rc != FR_OK )
        {
                pr_error( path );
                pr_error_int_msg(LOG_PFX "Error mounting filesystem: ", rc);
        }
	else
	{
		pr_trace( LOG_PFX "sd_mounted OK\r\n" );
	}
	return rc;
}
#define SD_TEST_PATTERN "0123456789"

void InitFSHardware(void)
{
	pr_trace( LOG_PFX "InitFSHardware\r\n" );

	if ( fs_mutex == NULL )
		fs_mutex = xSemaphoreCreateMutex();

	fs_lock();
        if (fat_fs == NULL)
                fat_fs = pvPortMalloc(sizeof(FATFS));
	disk_init_hardware();
	fs_unlock();
}

static int fs_lockcount = 0;
void fs_lock(void){
	fs_lockcount++;
	// pr_trace_int_msg( LOG_PFX "fs_lock: ", fs_lockcount );
        xSemaphoreTake(fs_mutex, portMAX_DELAY);
}

void fs_unlock(void){
	fs_lockcount--;
        xSemaphoreGive(fs_mutex);
}

static bool is_initialized()
{
	pr_trace( LOG_PFX "is_initialized\r\n" );
        if (fat_fs)
                return true;

        return false;
}

bool sdcard_present(void)
{
	fs_lock();
	bool ret = sdcard_device_card_present();
	fs_unlock();
	if ( ret )
	{
		pr_trace( LOG_PFX "sdcard_present: yes\r\n" );
	}
	else 
	{
		pr_trace( LOG_PFX "sdcard_present: no\r\n" );
	}
        return ret;
}

bool sdcard_fs_mounted(void)
{
	pr_trace( LOG_PFX "sdcard_fs_mounted\r\n" );
        char label[12]; /* max label is 11 bytes */
        DWORD vsn;
	const bool ret = sdcard_present() && FR_OK == sd_getlabel("0", label, &vsn);
        return ret;
}


int InitFS()
{
	pr_trace( LOG_PFX "InitFS\r\n" );
	if ( fs_mutex == NULL )
	{
		pr_trace( "no mutex\r\n" );
		return -1;
	}
        if(!is_initialized())
		return -1;
                

        if(!sdcard_present()) {
                return -1;
        }

        return sd_mount(fat_fs, "0", 1);
}

int UnmountFS()
{
        if(!is_initialized())
                return -1;

        return sd_mount(NULL, "0", 1);
}

bool test_sd(struct Serial *serial, int lines, int doFlush, int quiet) {
	pr_trace( LOG_PFX "test_sd\r\n" );
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

        bool fs_good = sdcard_ready( true );

	fs_lock();
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
        startTicks = xTaskGetTickCount();
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

        endTicks = xTaskGetTickCount();

        if (!quiet) {
                serial_write_s(serial,"time to read: ");
                put_int(serial, ticksToMs(endTicks - startTicks));
                serial_write_s(serial, " ms");
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

	fs_unlock();
        return res == 0 ? true : false;
}

void test_sd_interactive(struct Serial *serial, int lines, int doFlush, int quiet)
{
	pr_trace( LOG_PFX "test_sd_intereactive\r\n" );
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
	pr_trace( LOG_PFX "fs_write_sample_meta\r\n" );
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
	pr_trace( LOG_PFX "fs_write_sample_record\r\n" );
        char buf[30];

	fs_lock();
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
                                pr_warning_int_msg( LOG_PFX "Unknown sample"
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
	fs_unlock();
}

