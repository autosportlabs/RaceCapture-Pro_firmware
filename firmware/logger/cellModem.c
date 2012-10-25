#include "cellModem.h"
#include "usart.h"
#include <string.h>
#include "usb_comm.h"
#include "modp_numtoa.h"

#define min(a,b) ((a)<(b)?(a):(b))

#define NETWORK_CONNECT_MAX_TRIES 10

char g_cellBuffer[200];
char g_latestTextMsg[200];

#define PAUSE_DELAY 167

#define READ_TIMEOUT 334
#define SHORT_TIMEOUT 1500
#define MEDIUM_TIMEOUT 5000
#define CONNECT_TIMEOUT 10000

//#define DEBUG

static int readModemWait(portTickType delay){
#ifdef DEBUG
	SendString("Read:");
#endif
	int c = usart0_readLineWait(g_cellBuffer, sizeof(g_cellBuffer),delay);
#ifdef DEBUG
	SendString(g_cellBuffer);
#endif
	return c;
}

static int readModem(void){
	return readModemWait(portMAX_DELAY);
}

static int putsModem(const char *s){
	int c = usart0_puts(s);
#ifdef DEBUG
	SendString("Write: ");
	SendString(s);
	SendCrlf();
#endif
	return c;
}

int putcCell(char c){
	usart0_putchar(c);
	return 0;
}

static void flushModem(void){
	g_cellBuffer[0] = '\0';
	usart0_flush();
}

static void stripTrailingWhitespace(char *data){

	char * ch = data;
	while(*ch >= 32){ ch++;	}
	*ch = 0;
}

static int sendCommandWait(const char *cmd, portTickType wait){
	flushModem();
	putsModem(cmd);
	readModemWait(wait);
	readModemWait(READ_TIMEOUT);
	vTaskDelay(PAUSE_DELAY); //maybe take this out later - debugging SIM900
	return strncmp(g_cellBuffer,"OK",2) == 0;
}

static int sendCommand(const char * cmd){
	return sendCommandWait(cmd, READ_TIMEOUT);
}

static int isNetworkConnected(){

	flushModem();
	sendCommand("AT+CREG?\r");
	int connected = (0 == strncmp(g_cellBuffer,"+CREG: 0,1",10));
	return connected;
}

static int isDataReady(){
	flushModem();
	sendCommand("AT+CGATT?\r");
	int dataReady = (0 == strncmp(g_cellBuffer,"+CGATT: 1",9));
	return dataReady;
}

static int getIpAddress(){
	putsModem("AT+CIFSR\r");
	readModemWait(MEDIUM_TIMEOUT);
	readModemWait(READ_TIMEOUT);
	if (strlen(g_cellBuffer) == 0) return -1;
	if (strncmp(g_cellBuffer, "ERROR", 5) == 0) return -2;
	vTaskDelay(PAUSE_DELAY);
	return 0;
}

int configureNet(void){
	if (!sendCommand("AT+CIPMUX=0\r")) return -1;  //TODO enable for SIM900
	if (!sendCommand("AT+CIPMODE=1\r")) return -1;
	//if (!sendCommand("AT+CIPCCFG=3,2,256,1\r")) return -1;
	if (!sendCommandWait("AT+CSTT=\"epc.tmobile.com\",\"\",\"\"\r", READ_TIMEOUT)) return -2;
//	if (!sendCommand("AT+CIPHEAD=1\r")) return -2;
	if (!sendCommandWait("AT+CIICR\r", CONNECT_TIMEOUT)) return -4;
	if (getIpAddress() !=0 ) return -5;
	return 0;
}


int connectNet(const char *host, const char *port, int udpMode){
	flushModem();
	strcpy(g_cellBuffer, "AT+CIPSTART=\"");
	strcat(g_cellBuffer, udpMode ? "UDP" : "TCP");
	strcat(g_cellBuffer, "\",\"");
	strcat(g_cellBuffer, host);
	strcat(g_cellBuffer, "\",\"");
	strcat(g_cellBuffer, port);
	strcat(g_cellBuffer, "\"\r");
	putsModem(g_cellBuffer);
	int attempt = 0;
	while (attempt++ < 5){
		readModemWait(SHORT_TIMEOUT);
		if (strncmp(g_cellBuffer,"CONNECT",7) == 0) return 0;
		if (strncmp(g_cellBuffer,"ERROR",5) == 0) return -1;
		if (strncmp(g_cellBuffer,"FAIL",4) == 0) return -1;
		if (strncmp(g_cellBuffer,"CLOSED",6) == 0) return -1;
	}
	return -1;
}

int closeNet(){
	vTaskDelay(335); //~1000ms
	putsModem("+++");
	vTaskDelay(168); //~500ms
	if (!sendCommandWait("AT+CIPCLOSE\r", SHORT_TIMEOUT)) return -1;
	return 0;
}

int startNetData(){
	flushModem();
	putsModem("AT+CIPSEND\r");
	while (1){
		readModemWait(READ_TIMEOUT);
		if (strncmp(g_cellBuffer,">",1) == 0) return 0;
		if (strncmp(g_cellBuffer,"ERROR",5) == 0) return -1;
	}
}

int endNetData(){
	putcCell(26);
	while (1){
		readModemWait(READ_TIMEOUT);
		if (strncmp(g_cellBuffer,"DATA ACCEPT",7) == 0) return 0;
		if (strncmp(g_cellBuffer,"SEND OK",7) == 0) return 0;
		if (strncmp(g_cellBuffer,"ERROR",5) == 0) return -1;
	}
}

const char * readsCell(portTickType timeout){
	readModemWait(timeout);
	return g_cellBuffer;
}

int putsCell(const char *data){
	return putsModem(data);
}

void putUintCell(uint32_t num){
	char buf[10];
	modp_uitoa10(num,buf);
	putsCell(buf);
}

void putIntCell(int num){
	char buf[10];
	modp_itoa10(num,buf);
	putsCell(buf);
}

void putFloatCell(float num, int precision){
	char buf[20];
	modp_ftoa(num, buf, precision);
	putsCell(buf);
}

void putQuotedStringCell(char *s){
	usart0_putchar('"');
	usart0_puts(s);
	usart0_putchar('"');
}

int isNetConnectionErrorOrClosed(){
	const char * readData = readsCell(0);
	if (strncmp(readData,"CLOSED",6) == 0) return 1;
	if (strncmp(readData,"ERROR", 5) == 0) return 1;
	return 0;
}

int configureTexting(void){
	//Setup for Texting
	if (!sendCommand("AT+CMGF=1\r")) return -2;
	vTaskDelay(100);
	if (!sendCommand("AT+CSCS=\"GSM\"\r")) return -3;
	vTaskDelay(100);
	if (!sendCommand("AT+CSCA=\"+12063130004\"\r")) return -4;
	vTaskDelay(100);
	if (!sendCommand("AT+CSMP=17,167,0,240\r")) return -5;
	vTaskDelay(100);
	if (!sendCommand("AT+CNMI=0,0\r")) return -6;
	vTaskDelay(100);
	return 0;
}

int loadDefaultCellConfig(){
	if (!sendCommand("ATZ\r")) return -1;
	return 0;
}

void powerDownCellModem(){
	sendCommand("\rAT+CPOWD=1\r");
}

int initCellModem(void){

	initUsart0(USART_MODE_8N1, 115200);

	closeNet();

	while (1){
		if (loadDefaultCellConfig() == 0) break;
		vTaskDelay(900);
	}
	if (!sendCommand("ATE0\r")) return -1;
	sendCommand("AT+CIPSHUT\r");
	//wait until network is connected
	while (1){
		if (isNetworkConnected()) break;
		vTaskDelay(900);
	}
	while (1){
		if (isDataReady()) break;
		vTaskDelay(900);
	}
	return 0;
}

void deleteAllTexts(void){
	sendCommand("AT+CMGDA=\"DEL ALL\"\r");
}

void deleteInbox(void){
	sendCommand("AT+CMGDA=\"DEL INBOX\"\r");
}

void deleteSent(void){
	sendCommand("AT+CMGDA=\"DEL SENT\"\r");
}

const char * receiveText(int txtNumber){

	flushModem();
	putsModem("AT+CMGR=");
	char txtNumberBuffer[10];
	modp_itoa10(txtNumber,txtNumberBuffer);
	putsModem(txtNumberBuffer);
	putsModem("\r");

	readModem();
	readModem();
	if (0 != strncmp(g_cellBuffer, "+CMGR",5)) return NULL;


	size_t pos = 0;
	while(1){
		readModem();
		if (0 == strncmp(g_cellBuffer,"OK",2)) break;
		size_t len = strlen(g_cellBuffer);
		if (len == 0) continue;
		len = min(len, sizeof(g_latestTextMsg) - pos - 1);
		memcpy(g_latestTextMsg + pos,g_cellBuffer,len);
		pos += len;
	}
	g_latestTextMsg[pos]='\0';
	stripTrailingWhitespace(g_latestTextMsg);
	return g_latestTextMsg;
}

int sendText(const char * number, const char * msg){

	putsModem("AT+CMGS=\"");
	putsModem(number);
	putsModem("\"\r");
	putsModem(msg);
	putcCell(26);
	readModem();
	readModem();
	readModem();
	readModem();
	readModem();

	return 0;
}
