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

#include "cpu.h"
#include "gpsTask.h"
#include "loggerCommands.h"
#include "loggerConfig.h"
#include "loggerData.h"
#include "loggerHardware.h"
#include "loggerSampleData.h"
#include "loggerTaskEx.h"
#include "luaScript.h"
#include "mem_mang.h"
#include "printk.h"
#include "sampleRecord.h"
#include "sdcard.h"
#include "serial.h"
#include "taskUtil.h"
#include "tracks.h"
#include "usart.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

/* Time (ms) to wait for input before continuing */
#define TERM_WAIT_MS 5

static const char* log_type_str(enum serial_log_type type)
{
        switch(type) {
        case SERIAL_LOG_TYPE_ASCII:
                return "ASCII";
        case SERIAL_LOG_TYPE_BINARY:
                return "Binary";
        default:
                return "None";
        }
}

static const char* enable_str(const bool enable)
{
        return enable ? "Enabled" : "Disabled";
}

void TestSD(struct Serial *serial, unsigned int argc, char **argv)
{
    /* TODO BAP - could not remove TestSD from command list b/c statically defined array, fix somehow */
#if SDCARD_SUPPORT
    int lines = 1;
    int doFlush = 0;
    int quiet = 0;
    if (argc > 1) lines = atoi(argv[1]);
    if (argc > 2) doFlush = atoi(argv[2]);
    if (argc > 3) quiet = atoi(argv[3]);
    TestSDWrite(serial, lines, doFlush, quiet);
#endif

}


static void print_reset_status(struct Serial *s, const char *msg, const int status)
{
        char* status_word = status ? "FAILED" : "PASS";
        serial_write_s(s, msg);
        serial_write_s(s, ": ");
        serial_write_s(s, status_word);
        serial_write_s(s, "\r\n");
}

void ResetConfig(struct Serial *serial, unsigned int argc, char **argv)
{
        int tmp;
        int res = 0;

        tmp = flash_default_logger_config();
        res += tmp;
        print_reset_status(serial, "Flashing Default Logger Config", tmp);

#if LUA_SUPPORT
        tmp = flash_default_script();
        res += tmp;
        print_reset_status(serial, "Flashing Default Script", tmp);
#endif

        tmp = flash_default_tracks();
        res += tmp;
        print_reset_status(serial, "Flashing Default Tracks", tmp);

        if (res) {
                put_commandError(serial, ERROR_CODE_CRITICAL_ERROR);
        } else {
                put_commandOK(serial);
        }

        delayMs(500);
        cpu_reset(0);
}

static void StartTerminalSession(struct Serial *fromSerial, struct Serial *toSerial, uint8_t localEcho)
{
        bool stay = true;
        const size_t delay_ticks = msToTicks(TERM_WAIT_MS);
        while (stay) {
                char c;
                /* Delay so we don't starve other tasks (like watchdog) */
                while (0 < serial_read_c_wait(fromSerial, &c, delay_ticks)) {
                        if (c == 27)
                                stay = false;

                        if (localEcho)
                                serial_write_c(fromSerial, c);

                        if (c == '\r' && localEcho)
                                serial_write_c(fromSerial, '\n');

                        serial_write_c(toSerial, c);

                        if (c == '\r')
                                serial_write_c(toSerial, '\n');
                }

                while (0 < serial_read_c_wait(toSerial, &c, 0)) {
                        serial_write_c(fromSerial, c);
                        if (c == '\r' && localEcho)
                                serial_write_c(fromSerial, '\n');
                }
        }
}

void StartTerminal(struct Serial *serial, unsigned int argc, char **argv)
{
        if (argc < 3) {
                put_commandError(serial, ERROR_CODE_MISSING_PARAMS);
                return;
        }

        serial_write_s(serial, "Entering Terminal. Press ESC to exit\r\n");

        uint32_t port = (uint32_t) atoi(argv[1]);
        uint32_t baud = (uint32_t) atoi(argv[2]);
        uint8_t localEcho = (argc > 3 ? (uint8_t) atoi(argv[3]) : 1);

        struct Serial *targetSerial = serial_device_get(port);
        if (!targetSerial) {
                serial_write_s(serial, "Requested Serial port is NULL!\r\n");
                put_commandError(serial, ERROR_CODE_INVALID_PARAM);
                return;
        }

        serial_config(targetSerial, 8, 0, 1, baud);
        StartTerminalSession(serial, targetSerial, localEcho);
}

void ViewLog(struct Serial *serial, unsigned int argc, char **argv)
{
    serial_write_s(serial, "Starting logging mode.  Hit \"q\" to exit\r\n");

    while(1) {
        // Write log to serial
        read_log_to_serial(serial, 0);

        // Look for 'q' to exit.
        char c = 0;
        serial_read_c_wait(serial, &c, msToTicks(5));
        if (c == 'q') break;
    }

    // Give a little space when we finish up with log watching.
    serial_write_s(serial, "\r\n\r\n");
    serial_flush(serial);
}

void SetLogLevel(struct Serial *serial, unsigned int argc, char **argv)
{
    // XXX make this more robust maybe.
    if (argc < 1 || argv[1][0] < '0' || argv[1][0] > '8') {
        put_commandError(serial, ERROR_CODE_INVALID_PARAM);
        return;
    }

    enum log_level level = (enum log_level) atoi(argv[1]);
    set_log_level(level);
    put_commandOK(serial);
}

void LogGpsData(struct Serial *serial, unsigned int argc, char **argv)
{
    if (argc != 2) {
        serial_write_s(serial, "Must pass one argument only.  Enter 0 to disable, "
                      "or non-zero to enable\r\n");
        put_commandError(serial, ERROR_CODE_INVALID_PARAM);
    } else {
        const bool enable = (argv[1][0] != '0');
        setGpsDataLogging(enable);
        serial_write_s(serial, enable_str(enable));
        serial_write_s(serial, " the printing of raw GPS data to the log.\r\n");
        put_commandOK(serial);
    }

    serial_flush(serial);
}

void SetSerialLog(struct Serial *serial, unsigned int argc, char **argv)
{
        if (argc != 3) {
                serial_write_s(serial, "Two arguments required to specify serial port "
                              "and enable/disable option\r\n");
                put_commandError(serial, ERROR_CODE_INVALID_PARAM);
                goto done;
        }

        const serial_id_t port = (serial_id_t) atoi(argv[1]);
        struct Serial *s = serial_device_get(port);
        if (port == SERIAL_USB || !s) {
                serial_write_s(serial, "Invalid serial port.\r\n");
                put_commandError(serial, ERROR_CODE_INVALID_PARAM);
                goto done;
        }

        const enum serial_log_type lt = atoi(argv[2]);
        const enum serial_log_type prev = serial_logging(s, lt);
        getWorkingLoggerConfig()->logging_cfg.serial[port] = lt;

        serial_write_s(serial, log_type_str(prev));
        serial_write_s(serial, " -> ");
        serial_write_s(serial, log_type_str(lt));
        serial_write_s(serial, "\r\n");
        put_commandOK(serial);

done:
        serial_flush(serial);
}

void FlashConfig(struct Serial *serial, unsigned int argc, char **argv)
{
        const bool success = flashLoggerConfig() == 0;
        serial_write_s(serial, success ? "Success" : "Failed");
        serial_write_s(serial, "\r\n");
        put_commandOK(serial);
}
