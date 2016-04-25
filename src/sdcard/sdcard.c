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
#include "mod_string.h"
#include "modp_numtoa.h"
#include "FreeRTOS.h"
#include "task.h"
#include "taskUtil.h"
#include "loggerHardware.h"
#include "watchdog.h"
#include "diskio.h"
#include "sdcard_device.h"
#include "mem_mang.h"

static FATFS *FatFs = NULL;

void InitFSHardware(void)
{
    disk_init_hardware();
}

int InitFS()
{
    if (FatFs == NULL)
        FatFs = pvPortMalloc(sizeof(FATFS));

    if (FatFs == NULL) {
        pr_error("sdcard: FatFS init fail\r\n");
        return -1;
    }

    taskENTER_CRITICAL();
    const int res = disk_initialize(0);
    taskEXIT_CRITICAL();

    if (0 != res) {
        pr_error("sdcard: Disk init fail\r\n");
        return res;
    }

    return f_mount(FatFs, "0", 1);
}

int UnmountFS()
{
    return f_mount(NULL, "0", 1);
}

void TestSDWrite(struct Serial *serial, int lines, int doFlush, int quiet)
{
    int res = 0;
    FIL *fatFile = NULL;

    fatFile = pvPortMalloc(sizeof(FIL));
    if (NULL == fatFile) {
        if (!quiet)
                serial_put_s(serial, "could not allocate file object\r\n");

        goto exit;
    }

    if (!quiet) {
        serial_put_s(serial, "Test Write: Lines: ");
        put_int(serial, lines);
        put_crlf(serial);
        serial_put_s(serial, "Flushing Enabled: " );
        put_int(serial, doFlush);
        put_crlf(serial);
        serial_put_s(serial, "Card Init... ");
    }
    res = InitFS();
    if (res) goto exit;

    if (!quiet) {
        put_int(serial, res);
        put_crlf(serial);
        serial_put_s(serial, "Opening File... ");
    }
    res = f_open(fatFile,"test1.txt", FA_WRITE | FA_CREATE_ALWAYS);
    if (!quiet) {
        put_int(serial, res);
        put_crlf(serial);
    }
    if (res) goto exit;

    if (!quiet)
            serial_put_s(serial, "Writing file..");

    portTickType startTicks = xTaskGetTickCount();
    for (int i = 1; i <= lines; i++) {
        res = f_puts("The quick brown fox jumped over the lazy dog\n",fatFile);
        if (doFlush) f_sync(fatFile);
        if (res == EOF) {
            if (!quiet) serial_put_s(serial, "failed writing at line ");
            put_int(serial, i);
            serial_put_s(serial, "(");
            put_int(serial, res);
            serial_put_s(serial, ")");
            put_crlf(serial);
            goto exit;
        }
        watchdog_reset();
    }
    portTickType endTicks = xTaskGetTickCount();

    if (!quiet) {
        serial_put_s(serial, "Ticks to write: ");
        put_int(serial, endTicks - startTicks);
        put_crlf(serial);
        serial_put_s(serial, "Closing... ");
    }

    res = f_close(fatFile);
    if (!quiet) {
        put_int(serial, res);
        put_crlf(serial);
    }
    if (res) goto exit;

    if (!quiet)
            serial_put_s(serial, "Unmounting... ");

    res = UnmountFS();
    if (!quiet) {
        put_int(serial, res);
        put_crlf(serial);
    }
exit:
    if (quiet) {
        put_int(serial, res == 0 ? 1 : 0);
    } else {
        if(res == 0) {
            serial_put_s(serial, "SUCCESS");
        } else {
            serial_put_s(serial, "ERROR");
            put_int(serial, res);
            put_crlf(serial);
        }
    }
    if (fatFile != NULL) vPortFree(fatFile);
}
