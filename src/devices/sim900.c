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

#include "LED.h"
#include "cellular.h"
#include "devices_common.h"
#include "mod_string.h"
#include "modp_atonum.h"
#include "modp_numtoa.h"
#include "printk.h"
#include "serial_buffer.h"
#include "sim900.h"
#include "sim900_device.h"
#include "taskUtil.h"

#define min(a,b) ((a)<(b)?(a):(b))
#define NETWORK_CONNECT_MAX_TRIES 10

#define PAUSE_DELAY 100
#define READ_TIMEOUT 	1000
#define SHORT_TIMEOUT 	4500
#define MEDIUM_TIMEOUT 	15000
#define CONNECT_TIMEOUT 60000
#define NO_CELL_RESPONSE -99

/* XXX STIEG: Review sanity of this */
static void stripTrailingWhitespace(char *data)
{
        for(; *data >= 32; ++data);
        *data = 0;
}

static int waitCommandResponse(struct serial_buffer *sb, const char *expectedRsp,
                               size_t wait)
{
    int res = NO_CELL_RESPONSE;
    serial_buffer_read_wait(sb, wait);
    int len = serial_buffer_read_wait(sb, READ_TIMEOUT);
    delayMs(PAUSE_DELAY); //this is a magic delay that sim900 needs for proper communications

    if (len) {
        stripTrailingWhitespace(sb->buffer);
        if (strlen(sb->buffer) > 0) {
            res = (strstr(expectedRsp, sb->buffer) != NULL);
        }
    }

    return res;
}

static int sendCommandWait(struct serial_buffer *sb, const char *cmd,
                           const char *expectedRsp, size_t wait)
{
    serial_buffer_flush(sb);
    serial_buffer_puts(sb, cmd);
    return waitCommandResponse(sb, expectedRsp, wait);
}

static int sendCommand(struct serial_buffer *sb, const char * cmd,
                       const char *expectedRsp)
{
    return sendCommandWait(sb, cmd, expectedRsp, READ_TIMEOUT);
}

static int sendCommandOK(struct serial_buffer *sb, const char * cmd)
{
    return sendCommand(sb, cmd, "OK");
}

static int sendCommandRetry(struct serial_buffer *sb, const char * cmd,
                            const char * expectedRsp, size_t maxAttempts,
                            size_t maxNoResponseAttempts)
{
    int result = 0;
    size_t attempts = 0;

    while (attempts++ < maxAttempts) {
        result = sendCommand(sb, cmd, expectedRsp);

        if (result == 1)
                break;

        if (result == NO_CELL_RESPONSE && attempts > maxNoResponseAttempts)
                break;

        delayMs(1000);
    }

    return result;
}

static int read_subscriber_number(struct serial_buffer *sb,
                                  struct cellular_info *cell_info)
{
    int res = sendCommand(sb, "AT+CNUM\r", "+CNUM:");
    if (res != NO_CELL_RESPONSE) {
        char *num_start = strstr(sb->buffer, ",\"");
        if (num_start) {
            num_start += 2;
            char *num_end = strstr(num_start, "\"");
            if (num_end) {
                *num_end = '\0';
                strncpy(cell_info->number, num_start,
                        sizeof(cell_info->number));
                pr_debug_str_msg("Cell: phone number: ", cell_info->number);
            }
        }
    }
    return res;
}

static int getSignalStrength(struct serial_buffer *sb,
                             struct cellular_info *cell_info)
{
        const int res = sendCommand(sb, "AT+CSQ\r", "+CSQ:");

        if (res == NO_CELL_RESPONSE || strlen(sb->buffer) <= 6)
                return res;

        char *next_start = NULL;
        char *rssi_string = strtok_r(sb->buffer + 6, ",", &next_start);
        if (rssi_string != NULL) {
                const int signal = modp_atoi(rssi_string);
                pr_debug_int_msg("Cell: signal strength: ", signal);
                cell_info->signal = signal;
        }

        return res;
}

static int read_IMEI(struct serial_buffer *sb, struct cellular_info *cell_info)
{
        const int res = sendCommand(sb, "AT+GSN\r", "");

        if (res == NO_CELL_RESPONSE || strlen(sb->buffer) != 15)
                return res;

        strncpy(cell_info->imei, sb->buffer, sizeof(cell_info->imei));
        pr_debug_str_msg("Cell: IMEI: ", cell_info->imei);

        return res;
}

static int isNetworkConnected(struct serial_buffer *sb, size_t maxRetries,
                              size_t maxNoResponseRetries)
{
        serial_buffer_flush(sb);
        return sendCommandRetry(sb, "AT+CREG?\r", "+CREG: 0,1|+CREG: 0,5",
                                maxRetries, maxNoResponseRetries);
}

static int isDataReady(struct serial_buffer *sb, size_t maxRetries,
                       size_t maxNoResponseRetries)
{
        serial_buffer_flush(sb);
        return sendCommandRetry(sb, "AT+CGATT?\r", "+CGATT: 1",
                                maxRetries, maxNoResponseRetries);
}

static int getIpAddress(struct serial_buffer *sb)
{
	putsCell(sb, "AT+UPSND=0,0\r");
	serial_buffer_read_wait(sb, MEDIUM_TIMEOUT);
	serial_buffer_read_wait(sb, READ_TIMEOUT);
	if (strlen(sb->buffer) == 0) return -1;
	if (strncmp(sb->buffer, "ERROR", 5) == 0) return -2;
	delayMs(PAUSE_DELAY);
	return 0;
}

static int getDNSServer(struct serial_buffer *sb)
{
    putsCell(sb, "AT+UPSND=0,1\r");
    serial_buffer_read_wait(sb, MEDIUM_TIMEOUT);
    serial_buffer_read_wait(sb, READ_TIMEOUT);
    if (strlen(sb->buffer) == 0) return -1;
    if (strncmp(sb->buffer, "ERROR", 5) == 0) return -2;
    delayMs(PAUSE_DELAY);
    return 0;
}


int configureNet(struct serial_buffer *sb)
{

	if (sendCommandWait(sb, "AT+UPSDA=0,3\r", "OK", CONNECT_TIMEOUT) != 1){
		return -1;
	}

	if (getIpAddress(sb) != 0) {
		return -2;
	}

        if (getDNSServer(sb) != 0){
                return -3;
        }

        return 0;
}


int connectNet(struct serial_buffer *sb, const char *host, const char *port, int udpMode){
	serial_buffer_flush(sb);

	//create TCP socket
        if (sendCommandWait(sb, "AT+USOCR=6\r", "+USOCR: 0", CONNECT_TIMEOUT) != 1){
                return -1;
        }

	strcpy(sb->buffer, "AT+USOCO=0,\"");
	strcat(sb->buffer, host);
	strcat(sb->buffer, "\",");
	strcat(sb->buffer, port);
	strcat(sb->buffer, "\r");

        if (sendCommandWait(sb, "AT+USOCO=0,\"race-capture.com\",8080\r", "OK", CONNECT_TIMEOUT) != 1){
//    if (sendCommandWait(sb, sb->buffer, "OK", CONNECT_TIMEOUT) != 1){
                return -2;
        }

        putsCell(sb, "AT+USODL=0\r");
	int attempt = 0;
	while (attempt++ < 5){
		serial_buffer_read_wait(sb, SHORT_TIMEOUT);
		if (strncmp(sb->buffer,"CONNECT",7) == 0) return 0;
		if (strncmp(sb->buffer,"ERROR",5) == 0) return -1;
		if (strncmp(sb->buffer,"FAIL",4) == 0) return -1;
		if (strncmp(sb->buffer,"CLOSED",6) == 0) return -1;
	}
	return -3;
}

int closeNet(struct serial_buffer *sb){
	delayMs(1100);
	putsCell(sb, "+++");
	delayMs(1100);
	sendCommandWait(sb, "AT+USOCL=0\r", "OK", SHORT_TIMEOUT);
	sendCommandWait(sb, "AT+UPSDA=0,4\r", "OK", SHORT_TIMEOUT);
	return 0;
}

int isNetConnectionErrorOrClosed(struct serial_buffer *sb){
	if (strncmp(sb->buffer,"DISCONNECT", 10) == 0){
	    pr_info("ublox: socket disconnect\r\n");
	    return 1;
	}
	return 0;
}

static void powerCycleCellModem(void)
{
        sim900_device_power_button(1);
        delayMs(2000);
        sim900_device_power_button(0);
        delayMs(3000);
}

static int initAPN(struct serial_buffer *sb, CellularConfig *cellCfg){
    putsCell(sb, "AT+UPSD=0,1,\"");
    putsCell(sb, cellCfg->apnHost);
    putsCell(sb, "\"\r");
    if (!waitCommandResponse(sb, "OK", READ_TIMEOUT)){
        return -1;
    }

    putsCell(sb, "AT+UPSD=0,2,\"");
    putsCell(sb, cellCfg->apnUser);
    putsCell(sb, "\"\r");
    if (!waitCommandResponse(sb, "OK", READ_TIMEOUT)){
        return -2;
    }

    putsCell(sb, "AT+UPSD=0,3,\"");
    putsCell(sb, cellCfg->apnPass);
    putsCell(sb, "\"\r");
    if (!waitCommandResponse(sb, "OK", READ_TIMEOUT)){
        return -3;
    }

    putsCell(sb, "AT+UPSD=0,4,\"8.8.8.8\"\r");
    if (!waitCommandResponse(sb, "OK", READ_TIMEOUT)){
        return -4;
    }

    putsCell(sb, "AT+UPSD=0,5,\"8.8.4.4\"\r");
    if (!waitCommandResponse(sb, "OK", READ_TIMEOUT)){
        return -5;
    }
    return 0;
}

int initCellModem(struct serial_buffer *sb, CellularConfig *cellCfg,
                  struct cellular_info *cell_info)
{
	size_t attempts = 0;
	cell_info->status = CELLMODEM_STATUS_NOT_INIT;

	while (attempts++ < 3){
		closeNet(sb);

		if (attempts > 1){
			pr_debug("ublox: power cycling\r\n");
			if (sendCommandOK(sb, "AT\r") == 1 && attempts > 1){
				pr_debug("ublox: powering down\r\n");
				powerCycleCellModem();
			}
			powerCycleCellModem();
		}

		if (sendCommandRetry(sb, "ATZ\r", "OK", 2, 2) != 1)
                        continue;
		if (sendCommandRetry(sb, "ATE0\r", "OK", 2, 2) != 1)
                        continue;
		if (isNetworkConnected(sb, 60, 3) != 1)
                        continue;
		if (initAPN(sb, cellCfg) != 0)
                        continue;

		getSignalStrength(sb, cell_info);
		read_subscriber_number(sb, cell_info);
		read_IMEI(sb, cell_info);

		if (isDataReady(sb, 30, 2) != 1) {
                        cell_info->status = CELLMODEM_STATUS_PROVISIONED;
                        return 0;
                }
	}

        /* If here, we failed :( */
        cell_info->status = CELLMODEM_STATUS_NO_NETWORK;
        return -1;
}
