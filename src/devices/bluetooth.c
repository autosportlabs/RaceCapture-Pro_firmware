/**
 * AutoSport Labs - Race Capture Firmware
 *
 * Copyright (C) 2014 AutoSport Labs
 *
 * This file is part of the Race Capture firmware suite
 *
 * This is free software: you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details. You should have received a copy of the GNU
 * General Public License along with this code. If not, see <http://www.gnu.org/licenses/>.
 */

#include "bluetooth.h"
#include "FreeRTOS.h"
#include "loggerConfig.h"
#include "mod_string.h"
#include "printk.h"
#include "task.h"
#include "taskUtil.h"

#define COMMAND_WAIT 	600
#define BT_INIT_DELAY   100

static bluetooth_status_t g_bluetooth_status = BT_STATUS_NOT_INIT;

bluetooth_status_t bt_get_status()
{
    return g_bluetooth_status;
}

static int readBtWait(DeviceConfig *config, size_t delay)
{
    int c = config->serial->get_line_wait(config->buffer, config->length, delay);
    return c;
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

static int sendBtCommandWaitResponse(DeviceConfig *config, const char *cmd, const char *rsp,
                                     size_t wait)
{
    flushBt(config);
    vTaskDelay(COMMAND_WAIT);
    putsBt(config, cmd);
    readBtWait(config, wait);
    pr_debug_str_msg("BT: cmd rsp: ", config->buffer);
    int res = strncmp(config->buffer, rsp, strlen(rsp));
    pr_debug_str_msg("BT: ", res == 0 ? "match" : "nomatch");
    return res == 0;
}

static int sendBtCommandWait(DeviceConfig *config, const char *cmd, size_t wait)
{
    return sendBtCommandWaitResponse(config, cmd, "OK", COMMAND_WAIT);
}

static int sendCommand(DeviceConfig *config, const char * cmd)
{
    pr_debug_str_msg("BT: cmd: ", cmd);
    return sendBtCommandWait(config, cmd, COMMAND_WAIT);
}

static const char * baudConfigCmdForRate(unsigned int baudRate)
{
    switch (baudRate) {
    case 9600:
        return "AT+BAUD4";
        break;
    case 115200:
        return "AT+BAUD8";
        break;
    case 230400:
        return "AT+BAUD9";
        break;
    default:
        break;
    }
    pr_error_int_msg("invalid BT baud", baudRate);
    return "";
}

static int configureBt(DeviceConfig *config, unsigned int targetBaud, const char * deviceName)
{
    pr_info_int_msg("BT: Configuring baud Rate", targetBaud);
    //set baud rate
    if (!sendCommand(config, baudConfigCmdForRate(targetBaud)))
        return -1;
    config->serial->init(8, 0, 1, targetBaud);

    //set Device Name
    char btName[30];
    strcpy(btName, "AT+NAME");
    strcat(btName, deviceName);
    pr_info_str_msg("BT: Configuring name", btName);
    if (!sendBtCommandWaitResponse(config, btName, "OK", COMMAND_WAIT))
        return -2;
    return 0;
}

static int bt_probe_config(unsigned int probeBaud, unsigned int targetBaud, const char * deviceName,
                           DeviceConfig *config)
{
    pr_info_int_msg("BT: Probing baud ", probeBaud);
    config->serial->init(8, 0, 1, probeBaud);
    int rc;
    if (sendCommand(config, "AT")
        && (targetBaud == probeBaud || configureBt(config, targetBaud, deviceName) == 0)) {
        rc = DEVICE_INIT_SUCCESS;
    } else {
        rc = DEVICE_INIT_FAIL;
    }
    pr_info_str_msg("BT: Provision ", rc == DEVICE_INIT_SUCCESS ? "win" : "fail");
    return rc;
}

int bt_disconnect(DeviceConfig *config)
{
    return 0; //NOOP
}

int bt_init_connection(DeviceConfig *config)
{
    BluetoothConfig *btConfig = &(getWorkingLoggerConfig()->ConnectivityConfigs.bluetoothConfig);
    unsigned int targetBaud = btConfig->baudRate;
    const char *deviceName = btConfig->deviceName;

    //give a chance for BT module to init
    delayMs(BT_INIT_DELAY);

    // Zero terminated
    const int rates[] = { 115200, 9600, 230400, 0 };
    const int *rate = rates;
    for (; *rate != 0; ++rate) {
        const int status = bt_probe_config(*rate, targetBaud, deviceName, config);
        if (status == 0)
            break;
    }

    if (*rate > 0) {
        pr_info("BT: Init complete\r\n");
        g_bluetooth_status = BT_STATUS_PROVISIONED;
    } else {
        pr_info("BT: Failed to provision module. Assuming already connected.\r\n");
        g_bluetooth_status = BT_STATUS_ERROR;
    }
    config->serial->init(8, 0, 1, targetBaud);

    return DEVICE_INIT_SUCCESS;
}

int bt_check_connection_status(DeviceConfig *config)
{
    return DEVICE_STATUS_NO_ERROR;
}
