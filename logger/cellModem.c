#include "cellModem.h"
#include "usart.h"
#include "serial.h"
#include "modp_numtoa.h"
#include "mod_string.h"
#include "race_capture/printk.h"

#define min(a,b) ((a)<(b)?(a):(b))

#define NETWORK_CONNECT_MAX_TRIES 10

static char g_cellBuffer[200];

#define PAUSE_DELAY 167

#define READ_TIMEOUT 334
#define SHORT_TIMEOUT 1500
#define MEDIUM_TIMEOUT 5000
#define CONNECT_TIMEOUT 10000

static int readModemWait(portTickType delay){
	printk(DEBUG, "cellRead: ");
	int c = usart0_readLineWait(g_cellBuffer, sizeof(g_cellBuffer),delay);
	printk(DEBUG, g_cellBuffer);
	printk(DEBUG, "\n");
	return c;
}

static int readModem(void){
	return readModemWait(portMAX_DELAY);
}

static int putcModem(char c){
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

static int waitCommandResponse(portTickType wait){
	readModemWait(wait);
	readModemWait(READ_TIMEOUT);
	vTaskDelay(PAUSE_DELAY); //maybe take this out later - debugging SIM900
	return strncmp(g_cellBuffer,"OK",2) == 0;
}

static int sendCommandWait(const char *cmd, portTickType wait){
	flushModem();
	putsCell(cmd);
	return waitCommandResponse(wait);
	readModemWait(wait);
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
	putsCell("AT+CIFSR\r");
	readModemWait(MEDIUM_TIMEOUT);
	readModemWait(READ_TIMEOUT);
	if (strlen(g_cellBuffer) == 0) return -1;
	if (strncmp(g_cellBuffer, "ERROR", 5) == 0) return -2;
	vTaskDelay(PAUSE_DELAY);
	return 0;
}

void putsCell(const char *data){
	usart0_puts(data);
	printk(DEBUG, "cellWrite: ");
	printk(DEBUG, data);
	printk(DEBUG, "\r\n");
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
	putsCell("\"");
	putsCell(s);
	putsCell("\"");
}

int configureNet(const char *apnHost, const char *apnUser, const char *apnPass){
	if (!sendCommand("AT+CIPMUX=0\r")) return -1;  //TODO enable for SIM900
	if (!sendCommand("AT+CIPMODE=1\r")) return -1;

	//if (!sendCommand("AT+CIPCCFG=3,2,256,1\r")) return -1;

	flushModem();
	putsCell("AT+CSTT=\"");
	putsCell(apnHost);
	putsCell("\",\"");
	putsCell(apnUser);
	putsCell("\",\"");
	putsCell(apnPass);
	putsCell("\"\r");
	if (!waitCommandResponse(READ_TIMEOUT)) return -2;

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
	putsCell(g_cellBuffer);
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
	putsCell("+++");
	vTaskDelay(168); //~500ms
	if (!sendCommandWait("AT+CIPCLOSE\r", SHORT_TIMEOUT)) return -1;
	return 0;
}

int startNetData(){
	flushModem();
	putsCell("AT+CIPSEND\r");
	while (1){
		readModemWait(READ_TIMEOUT);
		if (strncmp(g_cellBuffer,">",1) == 0) return 0;
		if (strncmp(g_cellBuffer,"ERROR",5) == 0) return -1;
	}
}

int endNetData(){
	putcModem(26);
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

	initUsart0(8, 0, 1, 115200);

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

void receiveText(int txtNumber, char * txtMsgBuffer, size_t txtMsgBufferLen){

	flushModem();
	putsCell("AT+CMGR=");
	char txtNumberBuffer[10];
	modp_itoa10(txtNumber,txtNumberBuffer);
	putsCell(txtNumberBuffer);
	putsCell("\r");

	readModem();
	readModem();
	if (0 != strncmp(g_cellBuffer, "+CMGR",5)) return;


	size_t pos = 0;
	while(1){
		readModem();
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

int sendText(const char * number, const char * msg){

	putsCell("AT+CMGS=\"");
	putsCell(number);
	putsCell("\"\r");
	putsCell(msg);
	putcModem(26);
	readModem();
	readModem();
	readModem();
	readModem();
	readModem();

	return 0;
}
