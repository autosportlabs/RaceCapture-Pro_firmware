#include "cellModem.h"
#include "serial.h"
#include "modp_numtoa.h"
#include "mod_string.h"
#include "printk.h"
#include "devices_common.h"

#define min(a,b) ((a)<(b)?(a):(b))

#define NETWORK_CONNECT_MAX_TRIES 10

static char g_cellBuffer[200];

#define PAUSE_DELAY 167

#define READ_TIMEOUT 334
#define SHORT_TIMEOUT 1500
#define MEDIUM_TIMEOUT 5000
#define CONNECT_TIMEOUT 10000

static int readModemWait(Serial *serial, portTickType delay){
	printk(DEBUG, "cellRead: ");
	int c = serial->get_line_wait(g_cellBuffer, sizeof(g_cellBuffer),delay);
	printk(DEBUG, g_cellBuffer);
	printk(DEBUG, "\n");
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

static int waitCommandResponse(Serial *serial, portTickType wait){
	readModemWait(serial, wait);
	readModemWait(serial, READ_TIMEOUT);
	vTaskDelay(PAUSE_DELAY); //maybe take this out later - debugging SIM900
	return strncmp(g_cellBuffer,"OK",2) == 0;
}

static int sendCommandWait(Serial *serial, const char *cmd, portTickType wait){
	flushModem(serial);
	putsCell(serial, cmd);
	return waitCommandResponse(serial, wait);
	readModemWait(serial, wait);
}

static int sendCommand(Serial *serial, const char * cmd){
	return sendCommandWait(serial, cmd, READ_TIMEOUT);
}

static int isNetworkConnected(Serial *serial){

	flushModem(serial);
	sendCommand(serial, "AT+CREG?\r");
	int connected = (0 == strncmp(g_cellBuffer,"+CREG: 0,1",10));
	return connected;
}

static int isDataReady(Serial *serial){
	flushModem(serial);
	sendCommand(serial, "AT+CGATT?\r");
	int dataReady = (0 == strncmp(g_cellBuffer,"+CGATT: 1",9));
	return dataReady;
}

static int getIpAddress(Serial *serial){
	putsCell(serial, "AT+CIFSR\r");
	readModemWait(serial, MEDIUM_TIMEOUT);
	readModemWait(serial, READ_TIMEOUT);
	if (strlen(g_cellBuffer) == 0) return -1;
	if (strncmp(g_cellBuffer, "ERROR", 5) == 0) return -2;
	vTaskDelay(PAUSE_DELAY);
	return 0;
}

void putsCell(Serial *serial, const char *data){
	serial->put_s(data);
	printk(DEBUG, "cellWrite: ");
	printk(DEBUG, data);
	printk(DEBUG, "\r\n");
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
	if (!sendCommand(serial, "AT+CIPMUX=0\r")) return -1;  //TODO enable for SIM900
	if (!sendCommand(serial, "AT+CIPMODE=1\r")) return -1;

	//if (!sendCommand("AT+CIPCCFG=3,2,256,1\r")) return -1;

	flushModem(serial);
	putsCell(serial, "AT+CSTT=\"");
	putsCell(serial, apnHost);
	putsCell(serial, "\",\"");
	putsCell(serial, apnUser);
	putsCell(serial, "\",\"");
	putsCell(serial, apnPass);
	putsCell(serial, "\"\r");
	if (!waitCommandResponse(serial, READ_TIMEOUT)) return -2;

//	if (!sendCommand("AT+CIPHEAD=1\r")) return -2;

	if (!sendCommandWait(serial, "AT+CIICR\r", CONNECT_TIMEOUT)) return -4;

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
	vTaskDelay(335); //~1000ms
	putsCell(serial, "+++");
	vTaskDelay(168); //~500ms
	if (!sendCommandWait(serial, "AT+CIPCLOSE\r", SHORT_TIMEOUT)) return -1;
	return 0;
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
		if (strncmp(g_cellBuffer,"DATA ACCEPT",7) == 0) return 0;
		if (strncmp(g_cellBuffer,"SEND OK",7) == 0) return 0;
		if (strncmp(g_cellBuffer,"ERROR",5) == 0) return -1;
	}
}

const char * readsCell(Serial *serial, portTickType timeout){
	readModemWait(serial, timeout);
	return g_cellBuffer;
}

int isNetConnectionErrorOrClosed(Serial *serial){
	const char * readData = readsCell(serial, 0);
	if (strncmp(readData,"CLOSED",6) == 0) return 1;
	if (strncmp(readData,"ERROR", 5) == 0) return 1;
	return 0;
}

int configureTexting(Serial *serial){
	//Setup for Texting
	if (!sendCommand(serial, "AT+CMGF=1\r")) return -2;
	vTaskDelay(100);
	if (!sendCommand(serial, "AT+CSCS=\"GSM\"\r")) return -3;
	vTaskDelay(100);
	if (!sendCommand(serial, "AT+CSCA=\"+12063130004\"\r")) return -4;
	vTaskDelay(100);
	if (!sendCommand(serial, "AT+CSMP=17,167,0,240\r")) return -5;
	vTaskDelay(100);
	if (!sendCommand(serial, "AT+CNMI=0,0\r")) return -6;
	vTaskDelay(100);
	return 0;
}

int loadDefaultCellConfig(Serial *serial){
	if (!sendCommand(serial, "ATZ\r")) return -1;
	return 0;
}

void powerDownCellModem(Serial *serial){
	sendCommand(serial, "\rAT+CPOWD=1\r");
}

int initCellModem(Serial *serial){

	serial->init(8, 0, 1, 115200);

	closeNet(serial);

	while (1){
		if (loadDefaultCellConfig(serial) == 0) break;
		vTaskDelay(900);
	}
	if (!sendCommand(serial, "ATE0\r")) return -1;
	sendCommand(serial, "AT+CIPSHUT\r");
	//wait until network is connected
	while (1){
		if (isNetworkConnected(serial)) break;
		vTaskDelay(900);
	}
	while (1){
		if (isDataReady(serial)) break;
		vTaskDelay(900);
	}
	return 0;
}

void deleteAllTexts(Serial *serial){
	sendCommand(serial, "AT+CMGDA=\"DEL ALL\"\r");
}

void deleteInbox(Serial *serial){
	sendCommand(serial, "AT+CMGDA=\"DEL INBOX\"\r");
}

void deleteSent(Serial *serial){
	sendCommand(serial, "AT+CMGDA=\"DEL SENT\"\r");
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
