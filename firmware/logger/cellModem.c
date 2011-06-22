#include "cellModem.h"
#include "usart.h"
#include <string.h>
#include "usb_comm.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "modp_numtoa.h"

#define min(a,b) ((a)<(b)?(a):(b))

#define NETWORK_CONNECT_MAX_TRIES 10

char g_cellBuffer[200];
char g_latestTextMsg[200];

#define READ_TIMEOUT 900


static int readModemWait(portTickType delay){
	//SendString("read:");
	int c = usart0_readLineWait(g_cellBuffer, sizeof(g_cellBuffer),delay);
	//SendString(g_cellBuffer);
	return c;
}

static int readModem(void){
	return readModemWait(portMAX_DELAY);
}

static int putsModem(const char *s){
	int c = usart0_puts(s);
	//SendString("Write: ");
	//SendString(s);
	//SendCrlf();
	return c;
}

static int putc(char c){
	usart0_putchar(c);
	//SendChar(c);
	return 0;
}

static void flushModem(void){
	putc(26);
	usart0_flush();
}

static int sendCommand(const char * cmd){

	flushModem();
	putsModem(cmd);
	readModemWait(READ_TIMEOUT);
	readModemWait(READ_TIMEOUT);
	return strncmp(g_cellBuffer,"OK",2) == 0;
}



static int isNetworkConnected(){

	flushModem();
	sendCommand("AT+CREG?\r");
	int connected = (0 == strncmp(g_cellBuffer,"+CREG: 0,1",10));
	return connected;
}

int initCellModem(void){

	initUsart0(USART_MODE_8N1, 115200);

	//wait until network is connected
	while (1){
		if (isNetworkConnected()) break;
		vTaskDelay(900);
	}

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
		len = min(len, sizeof(g_latestTextMsg) - pos -1);
		memcpy(g_latestTextMsg + pos,g_cellBuffer,len);
		pos += len;
	}
	g_latestTextMsg[pos]='\0';
	return g_latestTextMsg;
}

int sendText(const char * number, const char * msg){

	putsModem("AT+CMGS=\"");
	putsModem(number);
	putsModem("\"\r");
	putsModem(msg);
	putc(26);
	readModem();
	readModem();
	readModem();
	readModem();
	readModem();

	return 0;
}
