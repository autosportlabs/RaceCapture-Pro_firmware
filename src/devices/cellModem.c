#include "cellModem.h"
#include "sim900_device.h"
#include "serial.h"
#include "modp_numtoa.h"
#include "mod_string.h"
#include "printk.h"
#include "devices_common.h"
#include "taskUtil.h"
#include "LED.h"

#define min(a,b) ((a)<(b)?(a):(b))

#define NETWORK_CONNECT_MAX_TRIES 10


static char *g_cellBuffer;
static size_t g_bufferLen;

#define PAUSE_DELAY 500

#define READ_TIMEOUT 	1000
#define SHORT_TIMEOUT 	4500
#define MEDIUM_TIMEOUT 	15000
#define CONNECT_TIMEOUT 30000

#define NO_CELL_RESPONSE -99

void setCellBuffer(char *buffer, size_t len){
	g_cellBuffer = buffer;
	g_bufferLen = len;
}

static int readModemWait(Serial *serial, portTickType delay){
	int c = serial->get_line_wait(g_cellBuffer, g_bufferLen, msToTicks(delay));
	if (DEBUG_LEVEL && c > 0){
		printk(DEBUG, "cellRead: ");
		printk(DEBUG, g_cellBuffer);
		printk(DEBUG, "\r\n");
	}
	return c;
}

static int readModem(Serial *serial){
	return readModemWait(serial, portMAX_DELAY);
}

static int putcModem(Serial *serial, char c){
	serial->put_c(c);
	return 0;
}

static void flushModem(Serial *serial){
	g_cellBuffer[0] = '\0';
	serial->flush();
}

static void stripTrailingWhitespace(char *data){
	char * ch = data;
	while(*ch >= 32){ ch++;	}
	*ch = 0;
}

static int waitCommandResponse(Serial *serial, const char *expectedRsp, portTickType wait){
	readModemWait(serial, wait);
	readModemWait(serial, READ_TIMEOUT);
	delayMs(PAUSE_DELAY); //maybe take this out later - debugging SIM900
	if (strlen(g_cellBuffer) == 0) return NO_CELL_RESPONSE;
	return strncmp(g_cellBuffer, expectedRsp , strlen(expectedRsp)) == 0;
}

static int sendCommandWait(Serial *serial, const char *cmd, const char *expectedRsp, portTickType wait){
	flushModem(serial);
	putsCell(serial, cmd);
	return waitCommandResponse(serial, expectedRsp, wait);
}

static int sendCommand(Serial *serial, const char * cmd, const char *expectedRsp){
	return sendCommandWait(serial, cmd, expectedRsp, READ_TIMEOUT);
}

static int sendCommandOK(Serial *serial, const char * cmd){
	return sendCommand(serial, cmd, "OK");
}

static int sendCommandRetry(Serial *serial, const char * cmd, const char * expectedRsp, size_t maxAttempts, size_t maxNoResponseAttempts){
	int result = 0;
	size_t attempts = 0;

	while (attempts++ < maxAttempts){
		result = sendCommand(serial, cmd, expectedRsp);
		if (result == 1) break;
		if (result == NO_CELL_RESPONSE && attempts > maxNoResponseAttempts) break;
		delayMs(1000);
	}
	return result;
}

static int isNetworkConnected(Serial *serial, size_t maxRetries, size_t maxNoResponseRetries){
	flushModem(serial);
	return sendCommandRetry(serial, "AT+CREG?\r", "+CREG: 0,1", maxRetries, maxNoResponseRetries);
}

static int isDataReady(Serial *serial, size_t maxRetries, size_t maxNoResponseRetries){
	flushModem(serial);
	return sendCommandRetry(serial, "AT+CGATT?\r", "+CGATT: 1", maxRetries, maxNoResponseRetries);
}

static int getIpAddress(Serial *serial){
	putsCell(serial, "AT+CIFSR\r");
	readModemWait(serial, MEDIUM_TIMEOUT);
	readModemWait(serial, READ_TIMEOUT);
	if (strlen(g_cellBuffer) == 0) return -1;
	if (strncmp(g_cellBuffer, "ERROR", 5) == 0) return -2;
	delayMs(PAUSE_DELAY);
	return 0;
}

void putsCell(Serial *serial, const char *data){
	LED_toggle(0);
	serial->put_s(data);
	pr_debug("cellWrite: ");
	pr_debug(data);
	pr_debug("\r\n");
}

void putUintCell(Serial *serial, uint32_t num){
	char buf[10];
	modp_uitoa10(num,buf);
	putsCell(serial, buf);
}

void putIntCell(Serial *serial, int num){
	char buf[10];
	modp_itoa10(num,buf);
	putsCell(serial, buf);
}

void putFloatCell(Serial *serial, float num, int precision){
	char buf[20];
	modp_ftoa(num, buf, precision);
	putsCell(serial, buf);
}

void putQuotedStringCell(Serial *serial, char *s){
	putsCell(serial, "\"");
	putsCell(serial, s);
	putsCell(serial, "\"");
}

int configureNet(Serial *serial, const char *apnHost, const char *apnUser, const char *apnPass){
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
	if (!waitCommandResponse(serial, "OK", READ_TIMEOUT)) return -2;

//	if (!sendCommand("AT+CIPHEAD=1\r")) return -2;

   // Configure DNS to use Google DNS
   const char dnsCmd[] = "AT+CDNSCFG=\"8.8.8.8\",\"8.8.4.4\"\r";
   if (!sendCommandWait(serial, dnsCmd, "OK", READ_TIMEOUT))
      return -3;

	if (!sendCommandWait(serial, "AT+CIICR\r", "OK", CONNECT_TIMEOUT)) return -4;

	if (getIpAddress(serial) !=0 ) return -5;
	return 0;
}


int connectNet(Serial *serial, const char *host, const char *port, int udpMode){
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
	while (attempt++ < 5){
		readModemWait(serial, SHORT_TIMEOUT);
		if (strncmp(g_cellBuffer,"CONNECT",7) == 0) return 0;
		if (strncmp(g_cellBuffer,"ERROR",5) == 0) return -1;
		if (strncmp(g_cellBuffer,"FAIL",4) == 0) return -1;
		if (strncmp(g_cellBuffer,"CLOSED",6) == 0) return -1;
	}
	return -1;
}

int closeNet(Serial *serial){
	delayMs(1100);
	putsCell(serial, "+++");
	delayMs(1100);
	return sendCommandWait(serial, "AT+CIPCLOSE\r", "OK", SHORT_TIMEOUT);
}

int startNetData(Serial *serial){
	flushModem(serial);
	putsCell(serial, "AT+CIPSEND\r");
	while (1){
		readModemWait(serial, READ_TIMEOUT);
		if (strncmp(g_cellBuffer,">",1) == 0) return 0;
		if (strncmp(g_cellBuffer,"ERROR",5) == 0) return -1;
	}
}

int endNetData(Serial *serial){
	putcModem(serial, 26);
	while (1){
		readModemWait(serial, READ_TIMEOUT);
		if (strncmp(g_cellBuffer,"DATA ACCEPT",11) == 0) return 0;
		if (strncmp(g_cellBuffer,"SEND OK",7) == 0) return 0;
		if (strncmp(g_cellBuffer,"ERROR",5) == 0) return -1;
	}
}

const char * readsCell(Serial *serial, portTickType timeout){
	readModemWait(serial, timeout);
	return g_cellBuffer;
}

int isNetConnectionErrorOrClosed(){
	if (strncmp(g_cellBuffer,"CLOSED",6) == 0) return 1;
	if (strncmp(g_cellBuffer,"ERROR", 5) == 0) return 1;
	return 0;
}

int configureTexting(Serial *serial){
	//Setup for Texting
	if (!sendCommandOK(serial, "AT+CMGF=1\r")) return -2;
	delayMs(100);
	if (!sendCommandOK(serial, "AT+CSCS=\"GSM\"\r")) return -3;
	delayMs(100);
	if (!sendCommandOK(serial, "AT+CSCA=\"+12063130004\"\r")) return -4;
	delayMs(100);
	if (!sendCommandOK(serial, "AT+CSMP=17,167,0,240\r")) return -5;
	delayMs(100);
	if (!sendCommandOK(serial, "AT+CNMI=0,0\r")) return -6;
	delayMs(100);
	return 0;
}


static void powerCycleCellModem(void){

	sim900_device_power_button(1);
	delayMs(2000);
	sim900_device_power_button(0);
	delayMs(3000);
}

int initCellModem(Serial *serial){

	size_t success = 0;
	size_t attempts = 0;

	while (!success && attempts++ < 3){
		closeNet(serial);

		if (attempts > 1){
			pr_debug("SIM900: power cycling\r\n");
			if (sendCommandOK(serial, "AT\r") == 1 && attempts > 1){
				pr_debug("SIM900: powering down\r\n");
				powerCycleCellModem();
			}
			powerCycleCellModem();
		}

		if (sendCommandRetry(serial, "ATZ\r", "OK", 2, 2) != 1) continue;
		if (sendCommandRetry(serial, "ATE0\r", "OK", 2, 2) != 1) continue;
		sendCommand(serial, "AT+CIPSHUT\r", "OK");
		if (isNetworkConnected(serial, 60, 3) != 1) continue;
		if (isDataReady(serial, 30, 2) != 1) continue;
		success = 1;
	}
	return success ? 0 : -1;
}

void deleteAllTexts(Serial *serial){
	sendCommandOK(serial, "AT+CMGDA=\"DEL ALL\"\r");
}

void deleteInbox(Serial *serial){
	sendCommandOK(serial, "AT+CMGDA=\"DEL INBOX\"\r");
}

void deleteSent(Serial *serial){
	sendCommandOK(serial, "AT+CMGDA=\"DEL SENT\"\r");
}

void receiveText(Serial *serial, int txtNumber, char * txtMsgBuffer, size_t txtMsgBufferLen){

	flushModem(serial);
	putsCell(serial, "AT+CMGR=");
	char txtNumberBuffer[10];
	modp_itoa10(txtNumber,txtNumberBuffer);
	putsCell(serial, txtNumberBuffer);
	putsCell(serial, "\r");

	readModem(serial);
	readModem(serial);
	if (0 != strncmp(g_cellBuffer, "+CMGR",5)) return;


	size_t pos = 0;
	while(1){
		readModem(serial);
		if (0 == strncmp(g_cellBuffer,"OK",2)) break;
		size_t len = strlen(g_cellBuffer);
		if (len == 0) continue;
		len = min(len, txtMsgBufferLen - pos - 1);
		memcpy(txtMsgBuffer + pos,g_cellBuffer,len);
		pos += len;
	}
	txtMsgBuffer[pos] = '\0';
	stripTrailingWhitespace(txtMsgBuffer);
}

int sendText(Serial *serial, const char * number, const char * msg){

	putsCell(serial, "AT+CMGS=\"");
	putsCell(serial, number);
	putsCell(serial, "\"\r");
	putsCell(serial, msg);
	putcModem(serial, 26);
	readModem(serial);
	readModem(serial);
	readModem(serial);
	readModem(serial);
	readModem(serial);

	return 0;
}
