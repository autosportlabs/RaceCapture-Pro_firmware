#include "cellModem.h"
#include "sim900_device.h"
#include "serial.h"
#include "modp_numtoa.h"
#include "modp_atonum.h"
#include "mod_string.h"
#include "printk.h"
#include "devices_common.h"
#include "taskUtil.h"
#include "LED.h"

#define min(a,b) ((a)<(b)?(a):(b))
#define NETWORK_CONNECT_MAX_TRIES 10
#define MAX_SUBSCRIBER_NUMBER_LENGTH 15
#define IMEI_NUMBER_LENGTH 16

static cellmodem_status_t g_cellmodem_status = CELLMODEM_STATUS_NOT_INIT;
static char g_subscriber_number[MAX_SUBSCRIBER_NUMBER_LENGTH];
static char g_IMEI_number[IMEI_NUMBER_LENGTH];

static char *g_cellBuffer;
static size_t g_bufferLen;
static uint8_t g_cell_signal_strength;

#define PAUSE_DELAY 500

#define READ_TIMEOUT 	1000
#define SHORT_TIMEOUT 	4500
#define MEDIUM_TIMEOUT 	15000
#define CONNECT_TIMEOUT 60000

#define NO_CELL_RESPONSE -99

cellmodem_status_t cellmodem_get_status( void )
{
    return g_cellmodem_status;
}

int cell_get_signal_strength()
{
    return g_cell_signal_strength;
}

char * cell_get_subscriber_number()
{
    return g_subscriber_number;
}

char * cell_get_IMEI()
{
    return g_IMEI_number;
}

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

static int read_subscriber_number(Serial *serial)
{
    int res = sendCommand(serial, "AT+CNUM\r", "+CNUM:");
    if (res != NO_CELL_RESPONSE) {
        char *num_start = strstr(g_cellBuffer, ",\"");
        if (num_start) {
            num_start += 2;
            char *num_end = strstr(num_start, "\"");
            if (num_end) {
                *num_end = '\0';
                strncpy(g_subscriber_number, num_start, MAX_SUBSCRIBER_NUMBER_LENGTH);
                pr_debug_str_msg("Cell: phone number: ", num_start);
            }
        }
    }
    return res;
}

static int getSignalStrength(Serial *serial)
{
    int res = sendCommand(serial, "AT+CSQ\r", "+CSQ:");
    if (res != NO_CELL_RESPONSE && strlen(g_cellBuffer) > 6) {
        char *next_start = NULL;
        char *rssi_string = strtok_r(g_cellBuffer + 6, ",", &next_start);
        if (rssi_string != NULL) {
            g_cell_signal_strength = modp_atoi(rssi_string);
            pr_debug_int_msg("Cell: signal strength: ", g_cell_signal_strength);
        }
    }
    return res;
}

static int read_IMEI(Serial *serial)
{
    int res = sendCommand(serial, "AT+GSN\r", "");
    if (res != NO_CELL_RESPONSE && strlen(g_cellBuffer) == 15) {
        strncpy(g_IMEI_number, g_cellBuffer, IMEI_NUMBER_LENGTH);
        pr_debug_str_msg("Cell: IMEI: ", g_IMEI_number);
    }
    return res;
}

static int isNetworkConnected(Serial *serial, size_t maxRetries, size_t maxNoResponseRetries)
{
    flushModem(serial);
    return sendCommandRetry(serial, "AT+CREG?\r", "+CREG: 0,1|+CREG: 0,5", maxRetries, maxNoResponseRetries);
}

static int isDataReady(Serial *serial, size_t maxRetries, size_t maxNoResponseRetries)
{
    flushModem(serial);
    return sendCommandRetry(serial, "AT+CGATT?\r", "+CGATT: 1", maxRetries, maxNoResponseRetries);
}

static int getIpAddress(Serial *serial)
{
    putsCell(serial, "AT+CIFSR\r");
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

int configureNet(Serial *serial, const char *apnHost, const char *apnUser, const char *apnPass)
{
    if (!sendCommandOK(serial, "AT+CIPMUX=0\r")) return -1;  //TODO enable for SIM900
    if (!sendCommandOK(serial, "AT+CIPMODE=1\r")) return -1;

    //if (!sendCommand("AT+CIPCCFG=3,2,256,1\r")) return -1;

    flushModem(serial);
    putsCell(serial, "AT+CSTT=\"");
    putsCell(serial, apnHost);
    putsCell(serial, "\",\"");
    putsCell(serial, apnUser);
    putsCell(serial, "\",\"");
    putsCell(serial, apnPass);
    putsCell(serial, "\"\r");
    if (!waitCommandResponse(serial, "OK", READ_TIMEOUT)) {
        return -2;
    }

//	if (!sendCommand("AT+CIPHEAD=1\r")) return -2;

    if (sendCommandWait(serial, "AT+CIICR\r", "OK", CONNECT_TIMEOUT) != 1) {
        return -3;
    }

    if (getIpAddress(serial) !=0 ) {
        return -4;
    }

    // Configure DNS to use Google DNS
    const char dnsCmd[] = "AT+CDNSCFG=\"8.8.8.8\",\"8.8.4.4\"\r";
    if (sendCommandWait(serial, dnsCmd, "OK", READ_TIMEOUT) != 1) {
        return -5;
    }

    return 0;
}


int connectNet(Serial *serial, const char *host, const char *port, int udpMode)
{
    flushModem(serial);
    strcpy(g_cellBuffer, "AT+CIPSTART=\"");
    strcat(g_cellBuffer, udpMode ? "UDP" : "TCP");
    strcat(g_cellBuffer, "\",\"");
    strcat(g_cellBuffer, host);
    strcat(g_cellBuffer, "\",\"");
    strcat(g_cellBuffer, port);
    strcat(g_cellBuffer, "\"\r");
    putsCell(serial, g_cellBuffer);
    int attempt = 0;
    while (attempt++ < 5) {
        readModemWait(serial, SHORT_TIMEOUT);
        if (strncmp(g_cellBuffer,"CONNECT",7) == 0) return 0;
        if (strncmp(g_cellBuffer,"ERROR",5) == 0) return -1;
        if (strncmp(g_cellBuffer,"FAIL",4) == 0) return -1;
        if (strncmp(g_cellBuffer,"CLOSED",6) == 0) return -1;
    }
    return -1;
}

int closeNet(Serial *serial)
{
    delayMs(1100);
    putsCell(serial, "+++");
    delayMs(1100);
    return sendCommandWait(serial, "AT+CIPCLOSE\r", "OK", SHORT_TIMEOUT);
}

const char * readsCell(Serial *serial, size_t timeout)
{
    readModemWait(serial, timeout);
    return g_cellBuffer;
}

int isNetConnectionErrorOrClosed()
{
    if (strncmp(g_cellBuffer,"CLOSED",6) == 0) return 1;
    if (strncmp(g_cellBuffer,"ERROR", 5) == 0) return 1;
    return 0;
}


static void powerCycleCellModem(void)
{

    sim900_device_power_button(1);
    delayMs(2000);
    sim900_device_power_button(0);
    delayMs(3000);
}

int initCellModem(Serial *serial)
{

    size_t success = 0;
    size_t attempts = 0;
    g_cellmodem_status = CELLMODEM_STATUS_NOT_INIT;

    while (!success && attempts++ < 3) {
        closeNet(serial);

        if (attempts > 1) {
            pr_debug("SIM900: power cycling\r\n");
            if (sendCommandOK(serial, "AT\r") == 1 && attempts > 1) {
                pr_debug("SIM900: powering down\r\n");
                powerCycleCellModem();
            }
            powerCycleCellModem();
        }

        if (sendCommandRetry(serial, "ATZ\r", "OK", 2, 2) != 1) continue;
        if (sendCommandRetry(serial, "ATE0\r", "OK", 2, 2) != 1) continue;
        sendCommand(serial, "AT+CIPSHUT\r", "OK");
        if (isNetworkConnected(serial, 60, 3) != 1) continue;
        getSignalStrength(serial);
        read_subscriber_number(serial);
        read_IMEI(serial);
        if (isDataReady(serial, 30, 2) != 1) continue;
        success = 1;
    }
    if ( success ) {
        g_cellmodem_status = CELLMODEM_STATUS_PROVISIONED;
        return 0;
    } else {
        g_cellmodem_status = CELLMODEM_STATUS_NO_NETWORK;
        return -1;
    }
}
