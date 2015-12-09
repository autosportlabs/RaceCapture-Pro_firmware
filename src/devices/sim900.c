<<<<<<< b94673be94d111e8670dd0352ca18fd5500bab4d:src/devices/cellModem.c
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
#include "cellModem.h"
#include "devices_common.h"
#include "mod_string.h"
#include "modp_atonum.h"
#include "modp_numtoa.h"
#include "printk.h"
#include "serial.h"
#include "sim900_device.h"
#include "taskUtil.h"

#define min(a,b) ((a)<(b)?(a):(b))
#define NETWORK_CONNECT_MAX_TRIES 10

static char *g_cellBuffer;
static size_t g_bufferLen;

#define PAUSE_DELAY 100

#define READ_TIMEOUT 	1000
#define SHORT_TIMEOUT 	4500
#define MEDIUM_TIMEOUT 	15000
#define CONNECT_TIMEOUT 60000

#define NO_CELL_RESPONSE -99

void setCellBuffer(char *buffer, size_t len)
{
    g_cellBuffer = buffer;
    g_bufferLen = len;
}

static int readModemWait(Serial *serial, size_t delay)
{
    int c = serial->get_line_wait(g_cellBuffer, g_bufferLen, msToTicks(delay));
    if (c > 2) {
            /*
             * Cell messages always end with a newline.  This also ignores
             * the messages that are simply stupid short.
             */
            pr_debug("Cell: read ");
            pr_debug(g_cellBuffer);
    }
    return c;
}

static void flushModem(Serial *serial)
{
    g_cellBuffer[0] = '\0';
    serial->flush();
}

static void stripTrailingWhitespace(char *data)
{
    char * ch = data;
    while(*ch >= 32) {
        ch++;
    }
    *ch = 0;
}

static int waitCommandResponse(Serial *serial, const char *expectedRsp, size_t wait)
{
    int res = NO_CELL_RESPONSE;
    readModemWait(serial, wait);
    int len = readModemWait(serial, READ_TIMEOUT);
    delayMs(PAUSE_DELAY); //this is a magic delay that sim900 needs for proper communications
    if  (len) {
        stripTrailingWhitespace(g_cellBuffer);
        if (strlen(g_cellBuffer) > 0) {
            res = (strstr(expectedRsp, g_cellBuffer) != NULL);
        }
    }
    return res;
}

static int sendCommandWait(Serial *serial, const char *cmd, const char *expectedRsp, size_t wait)
{
    flushModem(serial);
    putsCell(serial, cmd);
    int res = waitCommandResponse(serial, expectedRsp, wait);
    return res;
}

static int sendCommand(Serial *serial, const char * cmd, const char *expectedRsp)
{
    return sendCommandWait(serial, cmd, expectedRsp, READ_TIMEOUT);
}

static int sendCommandOK(Serial *serial, const char * cmd)
{
    return sendCommand(serial, cmd, "OK");
}

static int sendCommandRetry(Serial *serial, const char * cmd, const char * expectedRsp, size_t maxAttempts, size_t maxNoResponseAttempts)
{
    int result = 0;
    size_t attempts = 0;

    while (attempts++ < maxAttempts) {
        result = sendCommand(serial, cmd, expectedRsp);
        if (result == 1) break;
        if (result == NO_CELL_RESPONSE && attempts > maxNoResponseAttempts) break;
        delayMs(1000);
    }
    return result;
}

static int read_subscriber_number(Serial *serial,
                                  struct cellular_info *cell_info)
{
    int res = sendCommand(serial, "AT+CNUM\r", "+CNUM:");
    if (res != NO_CELL_RESPONSE) {
        char *num_start = strstr(g_cellBuffer, ",\"");
        if (num_start) {
            num_start += 2;
            char *num_end = strstr(num_start, "\"");
            if (num_end) {
                *num_end = '\0';
                strncpy(cell_info->number, num_start,
                        sizeof(cell_info->number));
                pr_debug_str_msg("Cell: phone number: ", num_start);
            }
        }
    }
    return res;
}

static int getSignalStrength(Serial *serial,
                             struct cellular_info *cell_info)
{
        const int res = sendCommand(serial, "AT+CSQ\r", "+CSQ:");

        if (res == NO_CELL_RESPONSE || strlen(g_cellBuffer) <= 6)
                return res;

        char *next_start = NULL;
        char *rssi_string = strtok_r(g_cellBuffer + 6, ",", &next_start);
        if (rssi_string != NULL) {
                const int signal = modp_atoi(rssi_string);
                pr_debug_int_msg("Cell: signal strength: ", signal);
                cell_info->signal = signal;
        }

        return res;
}

static int read_IMEI(Serial *serial, struct cellular_info *cell_info)
{
        const int res = sendCommand(serial, "AT+GSN\r", "");

        if (res == NO_CELL_RESPONSE || strlen(g_cellBuffer) != 15)
                return res;

        strncpy(cell_info->imei, g_cellBuffer, sizeof(cell_info->imei));
        pr_debug_str_msg("Cell: IMEI: ", cell_info->imei);

        return res;
}

static int isNetworkConnected(Serial *serial, size_t maxRetries,
                              size_t maxNoResponseRetries)
{
        flushModem(serial);
        return sendCommandRetry(serial, "AT+CREG?\r", "+CREG: 0,1|+CREG: 0,5",
                                maxRetries, maxNoResponseRetries);
}

static int isDataReady(Serial *serial, size_t maxRetries,
                       size_t maxNoResponseRetries)
{
        flushModem(serial);
        return sendCommandRetry(serial, "AT+CGATT?\r", "+CGATT: 1",
                                maxRetries, maxNoResponseRetries);
}

static int getIpAddress(Serial *serial)
{
	putsCell(serial, "AT+UPSND=0,0\r");
	readModemWait(serial, MEDIUM_TIMEOUT);
	readModemWait(serial, READ_TIMEOUT);
	if (strlen(g_cellBuffer) == 0) return -1;
	if (strncmp(g_cellBuffer, "ERROR", 5) == 0) return -2;
	delayMs(PAUSE_DELAY);
	return 0;
}

static int getDNSServer(Serial *serial)
{
    putsCell(serial, "AT+UPSND=0,1\r");
    readModemWait(serial, MEDIUM_TIMEOUT);
    readModemWait(serial, READ_TIMEOUT);
    if (strlen(g_cellBuffer) == 0) return -1;
    if (strncmp(g_cellBuffer, "ERROR", 5) == 0) return -2;
    delayMs(PAUSE_DELAY);
    return 0;
}

void putsCell(Serial *serial, const char *data)
{
	LED_toggle(0);
	serial->put_s(data);
	pr_debug_str_msg("cellWrite: ", data);
}

void putUintCell(Serial *serial, uint32_t num)
{
    char buf[10];
    modp_uitoa10(num,buf);
    putsCell(serial, buf);
}

void putIntCell(Serial *serial, int num)
{
    char buf[10];
    modp_itoa10(num,buf);
    putsCell(serial, buf);
}

void putFloatCell(Serial *serial, float num, int precision)
{
    char buf[20];
    modp_ftoa(num, buf, precision);
    putsCell(serial, buf);
}

void putQuotedStringCell(Serial *serial, char *s)
{
    putsCell(serial, "\"");
    putsCell(serial, s);
    putsCell(serial, "\"");
}


int configureNet(Serial *serial)
{

	if (sendCommandWait(serial, "AT+UPSDA=0,3\r", "OK", CONNECT_TIMEOUT) != 1){
		return -1;
	}

	if (getIpAddress(serial) !=0 ){
		return -2;
	}

    if (getDNSServer(serial) !=0 ){
        return -3;
    }

    return 0;
}


int connectNet(Serial *serial, const char *host, const char *port, int udpMode){
	flushModem(serial);

	//create TCP socket
    if (sendCommandWait(serial, "AT+USOCR=6\r", "+USOCR: 0", CONNECT_TIMEOUT) != 1){
        return -1;
    }

	strcpy(g_cellBuffer, "AT+USOCO=0,\"");
	strcat(g_cellBuffer, host);
	strcat(g_cellBuffer, "\",");
	strcat(g_cellBuffer, port);
	strcat(g_cellBuffer, "\r");
    if (sendCommandWait(serial, "AT+USOCO=0,\"race-capture.com\",8080\r", "OK", CONNECT_TIMEOUT) != 1){
//    if (sendCommandWait(serial, g_cellBuffer, "OK", CONNECT_TIMEOUT) != 1){
        return -2;
    }

    putsCell(serial, "AT+USODL=0\r");
	int attempt = 0;
	while (attempt++ < 5){
		readModemWait(serial, SHORT_TIMEOUT);
		if (strncmp(g_cellBuffer,"CONNECT",7) == 0) return 0;
		if (strncmp(g_cellBuffer,"ERROR",5) == 0) return -1;
		if (strncmp(g_cellBuffer,"FAIL",4) == 0) return -1;
		if (strncmp(g_cellBuffer,"CLOSED",6) == 0) return -1;
	}
	return -3;
}

int closeNet(Serial *serial){
	delayMs(1100);
	putsCell(serial, "+++");
	delayMs(1100);
	sendCommandWait(serial, "AT+USOCL=0\r", "OK", SHORT_TIMEOUT);
	sendCommandWait(serial, "AT+UPSDA=0,4\r", "OK", SHORT_TIMEOUT);
	return 0;
}

const char * readsCell(Serial *serial, size_t timeout)
{
    readModemWait(serial, timeout);
    return g_cellBuffer;
}

int isNetConnectionErrorOrClosed(){
	if (strncmp(g_cellBuffer,"DISCONNECT", 10) == 0){
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

static int initAPN(Serial *serial, CellularConfig *cellCfg){
    putsCell(serial, "AT+UPSD=0,1,\"");
    putsCell(serial, cellCfg->apnHost);
    putsCell(serial, "\"\r");
    if (!waitCommandResponse(serial, "OK", READ_TIMEOUT)){
        return -1;
    }

    putsCell(serial, "AT+UPSD=0,2,\"");
    putsCell(serial, cellCfg->apnUser);
    putsCell(serial, "\"\r");
    if (!waitCommandResponse(serial, "OK", READ_TIMEOUT)){
        return -2;
    }

    putsCell(serial, "AT+UPSD=0,3,\"");
    putsCell(serial, cellCfg->apnPass);
    putsCell(serial, "\"\r");
    if (!waitCommandResponse(serial, "OK", READ_TIMEOUT)){
        return -3;
    }

    putsCell(serial, "AT+UPSD=0,4,\"8.8.8.8\"\r");
    if (!waitCommandResponse(serial, "OK", READ_TIMEOUT)){
        return -4;
    }

    putsCell(serial, "AT+UPSD=0,5,\"8.8.4.4\"\r");
    if (!waitCommandResponse(serial, "OK", READ_TIMEOUT)){
        return -5;
    }
    return 0;
}

int initCellModem(Serial *serial, CellularConfig *cellCfg,
                  struct cellular_info *cell_info)
{
	size_t attempts = 0;
	cell_info->status = CELLMODEM_STATUS_NOT_INIT;

	while (attempts++ < 3){
		closeNet(serial);

		if (attempts > 1){
			pr_debug("ublox: power cycling\r\n");
			if (sendCommandOK(serial, "AT\r") == 1 && attempts > 1){
				pr_debug("ublox: powering down\r\n");
				powerCycleCellModem();
			}
			powerCycleCellModem();
		}

		if (sendCommandRetry(serial, "ATZ\r", "OK", 2, 2) != 1)
                        continue;
		if (sendCommandRetry(serial, "ATE0\r", "OK", 2, 2) != 1)
                        continue;
		if (isNetworkConnected(serial, 60, 3) != 1)
                        continue;
		if (initAPN(serial, cellCfg) != 0)
                        continue;

		getSignalStrength(serial, cell_info);
		read_subscriber_number(serial, cell_info);
		read_IMEI(serial, cell_info);

		if (isDataReady(serial, 30, 2) != 1) {
                        cell_info->status = CELLMODEM_STATUS_PROVISIONED;
                        return 0;
                }
	}

        /* If here, we failed :( */
        cell_info->status = CELLMODEM_STATUS_NO_NETWORK;
        return -1;
}
