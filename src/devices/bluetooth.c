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
#include "printk.h"
#include "task.h"
#include "taskUtil.h"
#include <stdbool.h>
#include <string.h>

/*
 * https://www.olimex.com/Products/Components/RF/BLUETOOTH-SERIAL-HC-06/resources/hc06.pdf
 *
 * Doc specifies 1000ms between commands.  Unit need ~500ms pause to then
 * parse the command and send a reply.  Give it 100ms to reply so that makes
 * the BT_COMMAND_WAIT time 600ms.  Give it 3 chances to respond to an AT
 * ping before we try another baud.
 *
 * BE WARNED... it took a bit of trial and error to come to these parameters
 * but they seem to be very stable (even with name + pin changes).  Adjust at
 * your own peril.
 */
#define BT_BACKOFF_MS	500
#define BT_BAUD_RATES	{230400, 115200, 9600}
#define BT_COMMAND_WAIT	600
#define BT_DEFAULT_NAME	"RaceCapturePro"
#define BT_DEFAULT_PIN	"1234"
#define BT_DBG_LOG_LVL	TRACE
#define BT_MAX_NAME_LEN	(BT_DEVICE_NAME_LENGTH - 1)
#define BT_MAX_PIN_LEN	(BT_PASSCODE_LENGTH - 1)
#define BT_PING_TRIES	3
#define TARGET_BAUD	115200

static bluetooth_status_t g_bluetooth_status = BT_STATUS_NOT_INIT;

bluetooth_status_t bt_get_status()
{
        return g_bluetooth_status;
}

static int sendBtCommandWaitResponse(DeviceConfig *config, const char *cmd,
                                     const char *rsp, const size_t wait)
{
        /* Put a little time between commands for the BT unit to catch up */
        delayMs(BT_BACKOFF_MS);

        serial_flush(config->serial);
        if (serial_write_s(config->serial, cmd) <= 0) {
		pr_error("BT: Failed to write serial command\r\n");
		return 0;
	};

        const int len = serial_read_line_wait(config->serial, config->buffer,
					      config->length, wait);
	if (len < 0) {
		pr_error("BT: Serial device closed\r\n");
		return 0;
	}

        config->buffer[len] = 0;

        const bool res = 0 == strncmp(config->buffer, rsp, strlen(rsp));

        printk_str_msg(BT_DBG_LOG_LVL, "BT: cmd given: ", cmd);
        printk_str_msg(BT_DBG_LOG_LVL, "BT: rsp expected: ", rsp);
        printk_str_msg(BT_DBG_LOG_LVL, "BT: rsp received: ", config->buffer);
        printk_str_msg(BT_DBG_LOG_LVL, "BT: match: ", res ? "YES" : "NO");

        return res;
}

static int sendCommand(DeviceConfig *config, const char * cmd)
{
        return sendBtCommandWaitResponse(config, cmd, "OK", BT_COMMAND_WAIT);
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

static void set_bt_serial_baud(DeviceConfig *config, int baud)
{
        pr_info_int_msg("BT: Baudrate: ", baud);
        serial_config(config->serial, 8, 0, 1, baud);
}

static int set_check_bt_serial_baud(DeviceConfig *config, int baud)
{
        int rc = false;

        /* Change the baud and give things a bit to catch up */
        set_bt_serial_baud(config, baud);
        for (size_t tries = BT_PING_TRIES; 0 < tries && !rc; --tries)
                rc = sendCommand(config, "AT");

        return rc;
}

static int bt_set_baud(DeviceConfig *config, unsigned int targetBaud)
{
        pr_info_int_msg("BT: Setting baud to ", targetBaud);

        if (!sendCommand(config, baudConfigCmdForRate(targetBaud)))
                return -1;

        return set_check_bt_serial_baud(config, targetBaud);
}

static bool bt_set_name(DeviceConfig *config, const char *new_name)
{
        pr_info_str_msg("BT: Setting name: ", new_name);

        char buf[BT_MAX_NAME_LEN + 7 + 1] = "AT+NAME";
        strncpy(buf + 7, new_name, BT_MAX_NAME_LEN + 1);

        return sendBtCommandWaitResponse(config, buf, "OKsetname",
                                         BT_COMMAND_WAIT);
}

static bool bt_set_pin(DeviceConfig *config, const char *new_pin)
{
        pr_info_str_msg("BT: Setting pin: ", new_pin);

        char buf[BT_MAX_PIN_LEN + 6 + 1] = "AT+PIN";
        strncpy(buf + 6, new_pin, BT_MAX_PIN_LEN + 1);

        return sendBtCommandWaitResponse(config, buf, "OKsetPIN",
                                         BT_COMMAND_WAIT);
}

static int bt_find_working_baud(DeviceConfig *config,
                                BluetoothConfig *btc,
                                const int targetBaud)
{
        pr_info("BT: Detecting baud rate...\r\n");
        const int rates[] = BT_BAUD_RATES;
        int rate = 0;

        if (set_check_bt_serial_baud(config, targetBaud))
                        rate = targetBaud;

        for (size_t i = 0; rate == 0 && i < sizeof(rates)/sizeof(*rates); ++i) {
                /* Skip the baud rate if we have already tried it */
                if (rates[i] == targetBaud)
                        continue;

                if (set_check_bt_serial_baud(config, rates[i]))
                        rate = rates[i];
        }

        /* Check that we didn't fail to find a workable rate */
        if (rate) {
                pr_info_int_msg("BT: Device responds at baud ", rate);
        } else {
                pr_info("BT: Could not detect device using known baud "
                        "rates.\r\n");
        }

        return rate;
}

static void bt_clear_new_vals(BluetoothConfig *btc)
{
        /* If these values aren't set, then no action is required */
        if ('\0' == btc->new_name[0] && '\0' == btc->new_pin[0])
                return;

        btc->new_name[0] = '\0';
        btc->new_pin[0] = '\0';

        /*
         * Have to do this to ensure that we reset the new_name and new_pin
         * variables after we change them in the BT device.  This is so
         * we don't constantly re-program the BT unit (which causes it
         * to unpair with any paired devices).
         */
        pr_info("BT: Resetting name/pin values in NVRAM\r\n");
        flashLoggerConfig();
        return;
}

int bt_disconnect(DeviceConfig *config)
{
        return 0; /* NOOP */
}

int bt_init_connection(DeviceConfig *config)
{
        BluetoothConfig *btConfig =
                &(getWorkingLoggerConfig()->ConnectivityConfigs.bluetoothConfig);
        const int targetBaud = TARGET_BAUD;
        const char *new_name = btConfig->new_name;
        const char *new_pin = btConfig->new_pin;

        /*
         * The HC-O6 seems to sometimes have trouble dealing with long AT
         * commands. Namely at high speed it seems that the device can't
         * process certain AT commands fast enough, causing an empty response.
         * To get past this, we set the baud rate while programming down to
         * factory level (9600).  This ensures things go slow enough for the
         * HC-06 processor + code to handle it.
         */
        int baud = bt_find_working_baud(config, btConfig, targetBaud);
        switch (baud) {
        case 0:
                pr_info("BT: Failed to communicate with device.\r\n");

                /* Restore the targetBaud rate in case already in command mode */
                pr_info_int_msg("BT: Restoring to target baud: ", targetBaud);
                set_bt_serial_baud(config, targetBaud);

                break;
        case 9600:
                pr_info("BT: Detected factory settings.  Assuming not "
                        "configured. Initializing...\r\n");
                /* Doing this allows us to set the name without reflashing */
                new_name = BT_DEFAULT_NAME;
                new_pin = BT_DEFAULT_PIN;
                break;
        }

        const bool status =
                baud &&
                ('\0' == *new_name || bt_set_name(config, new_name)) &&
                ('\0' == *new_pin ||bt_set_pin(config, new_pin)) &&
                (baud == targetBaud || bt_set_baud(config, targetBaud));

        if (status) {
                pr_info("BT: Init complete\r\n");
                bt_clear_new_vals(btConfig);
                g_bluetooth_status = BT_STATUS_PROVISIONED;
        } else {
                pr_info("BT: Failed to provision module. A client may "
                        "already be connected.\r\n");
                g_bluetooth_status = BT_STATUS_ERROR;
        }

        return DEVICE_INIT_SUCCESS;
}

int bt_check_connection_status(DeviceConfig *config)
{
        return DEVICE_STATUS_NO_ERROR;
}
