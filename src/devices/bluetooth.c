/*
 * Race Capture Pro Firmware
 *
 * Copyright (C) 2015 Autosport Labs
 *
 * This file is part of the Race Capture Pro fimrware suite
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

#include "FreeRTOS.h"
#include "bluetooth.h"
#include "loggerConfig.h"
#include "mod_string.h"
#include "printk.h"
#include "task.h"
#include "taskUtil.h"

#include <stdbool.h>

#define BT_AT_CMD_BAUD	9600
#define BT_CMD_BACKOFF_MS	5
#define BT_INIT_DELAY   100
#define BT_MAX_NAME_LEN	20
#define BT_MAX_PIN_LEN	4
#define COMMAND_WAIT 	600

static bluetooth_status_t g_bluetooth_status = BT_STATUS_NOT_INIT;

bluetooth_status_t bt_get_status()
{
        return g_bluetooth_status;
}

static int readBtWait(DeviceConfig *config, size_t delay)
{
        return config->serial->get_line_wait(config->buffer, config->length,
                                             delay);
}

static void flushBt(DeviceConfig *config)
{
        config->buffer[0] = '\0';
        config->serial->flush();
}

void putsBt(DeviceConfig *config, const char *data)
{
        config->serial->put_s(data);
}

static int sendBtCommandWaitResponse(DeviceConfig *config, const char *cmd,
                                     const char *rsp, const size_t wait)
{
        pr_debug_str_msg("BT: cmd: ", cmd);

        flushBt(config);
        putsBt(config, cmd);
        readBtWait(config, wait);

        const bool res = 0 == strncmp(config->buffer, rsp, strlen(rsp));

        pr_debug_str_msg("BT: wanted rsp: ", rsp);
        pr_debug_str_msg("BT: actual rsp: ", config->buffer);
        pr_debug_int_msg("BT: matched: ", res);

        /* Put a little time between commands for the BT unit to catch up */
        delayMs(BT_CMD_BACKOFF_MS);

        return res;
}

static int sendCommand(DeviceConfig *config, const char * cmd)
{
        return sendBtCommandWaitResponse(config, cmd, "OK", COMMAND_WAIT);
}

static const char * baudConfigCmdForRate(unsigned int baudRate)
{
        switch (baudRate) {
        case 9600:
                return "AT+BAUD4";
        case 57600:
                return "AT+BAUD7";
        case 115200:
                return "AT+BAUD8";
        case 230400:
                return "AT+BAUD9";
        }

        pr_error_int_msg("invalid BT baud", baudRate);
        return "";
}

static int set_check_bt_serial_baud(DeviceConfig *config, int baud)
{
        /* Change the baud and give things a bit to catch up */
        config->serial->init(8, 0, 1, baud);
        delayMs(BT_CMD_BACKOFF_MS);
        return sendCommand(config, "AT");
}

static int bt_set_baud(DeviceConfig *config, unsigned int targetBaud)
{
        pr_info_int_msg("BT: Setting baud to ", targetBaud);

        if (!sendCommand(config, "AT+PN"))
                return -2;

        if (!sendCommand(config, baudConfigCmdForRate(targetBaud)))
                return -1;

        return set_check_bt_serial_baud(config, targetBaud);
}

static bool bt_get_version(DeviceConfig *config)
{
        pr_info("BT: Retrieving version info\r\n");

        char *msg = "AT+VERSION";
        if (!sendCommand(config, msg))
                return false;

        /* Strip the leading "OK" */
        pr_info_str_msg("BT: Version Info: ", config->buffer + 2);
        return strlen(config->buffer) > 0;
}

static int bt_set_name(DeviceConfig *config, const char *bt_name)
{
        pr_info_str_msg("BT: Setting name: ", bt_name);

        char buf[BT_MAX_NAME_LEN + 7 + 1] = "AT+NAME";
        strlcpy(buf + 7, bt_name, BT_MAX_NAME_LEN + 1);

        return sendBtCommandWaitResponse(config, buf, "OKsetname",
                                         COMMAND_WAIT);
}

static int bt_set_pin(DeviceConfig *config, const char *pin_str)
{
        pr_info_str_msg("BT: Setting pin: ", pin_str);

        char buf[BT_MAX_PIN_LEN + 6 + 1] = "AT+PIN";
        strlcpy(buf + 6, pin_str, BT_MAX_PIN_LEN + 1);

        return sendBtCommandWaitResponse(config, buf, "OKsetPIN",
                                         COMMAND_WAIT);
}


static bool bt_find_working_baud(DeviceConfig *config, const int targetBaud)
{
        pr_info("BT: Searching for working baud rate...\r\n");

        const int rates[] = {targetBaud, 230400, 115200, 57600, 9600};
        size_t i = 0;
        for (; i < sizeof(rates); ++i) {
                if(set_check_bt_serial_baud(config, rates[i]))
                        break;
        }

        /* Check that we didn't fail to find a workable rate */
        if (i == sizeof(rates)) {
                pr_info("BT: Unable to communicate with device.\r\n");
                return false;
        }

        pr_info_int_msg("BT: Talking with BT device.  Baud: ", rates[i]);
        return true;
}

int bt_disconnect(DeviceConfig *config)
{
        return 0; //NOOP
}

int bt_init_connection(DeviceConfig *config)
{
        BluetoothConfig *btConfig =
                &(getWorkingLoggerConfig()->ConnectivityConfigs.bluetoothConfig);
        unsigned int targetBaud = btConfig->baudRate;
        const char *deviceName = btConfig->deviceName;
        const char *pin = btConfig->passcode;

        /* give a chance for BT module to init */
        delayMs(BT_INIT_DELAY);

        /*
         * The HC-O6 seems to sometimes have trouble dealing with long AT
         * commands. Namely at high speed it seems that the device can't
         * process certain AT commands fast enough, causing an empty response.
         * To get past this, we set the baud rate while programming down to
         * factory level (9600).  This ensures things go slow enough for the
         * HC-06 processor + code to handle it.
         */
        const bool status =
                bt_find_working_baud(config, targetBaud) &&
                bt_set_baud(config, BT_AT_CMD_BAUD) &&
                bt_get_version(config) &&
                bt_set_name(config, deviceName) &&
                bt_set_pin(config, pin) &&
                bt_set_baud(config, targetBaud);

        if (status) {
                pr_info("BT: Init complete\r\n");
                g_bluetooth_status = BT_STATUS_PROVISIONED;
        } else {
                pr_info("BT: Failed to provision module.  This may "
                        "be caused by a device connecting to the BT "
                        "module.\r\n");
                g_bluetooth_status = BT_STATUS_ERROR;
        }

        return DEVICE_INIT_SUCCESS;
}

int bt_check_connection_status(DeviceConfig *config)
{
        return DEVICE_STATUS_NO_ERROR;
}
