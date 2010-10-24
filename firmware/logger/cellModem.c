#include "cellModem.h"
#include "usart.h"
#include <string.h>
#include "usb_comm.h"
#include "FreeRTOS.h"
#include "task.h"
#define min(a,b) ((a)<(b)?(a):(b))



char g_cellBuffer[200];
char g_latestTextMsg[200];


static int sendCommand(const char * cmd){

	usart0_puts(cmd);
	usart0_readLine(g_cellBuffer, sizeof(g_cellBuffer));
	usart0_readLine(g_cellBuffer, sizeof(g_cellBuffer));
	/*SendString("cmd:");
	SendString(cmd);
	SendString("\n");
	SendString("rsp:");
	SendString(g_cellBuffer);
	SendString("\n");
	*/

	return strncmp(g_cellBuffer,"OK",2) == 0;
}


int initCellModem(void){

	while(1){
		usart0_readLine(g_cellBuffer, sizeof(g_cellBuffer));
		if (0 == strncmp(g_cellBuffer,"Call Ready",10)) break;
	}

	//Setup for Texting
	if (!sendCommand("AT+CMGF=1\r")) return -1;
	vTaskDelay(500);
	if (!sendCommand("AT+CSCS=\"GSM\"\r")) return -2;
	vTaskDelay(500);
	if (!sendCommand("AT+CSCA=\"+12063130004\"\r")) return -3;
	vTaskDelay(500);
	if (!sendCommand("AT+CSMP=17,167,0,240\r")) return -4;
	vTaskDelay(500);
	if (!sendCommand("AT+CNMI=0,0\r")) return -5;
	vTaskDelay(500);
	return 0;
}


const char * receiveText(){

	usart0_puts("AT+CMGL=\"REC UNREAD\"\r");

	usart0_readLine(g_cellBuffer, sizeof(g_cellBuffer));
	usart0_readLine(g_cellBuffer, sizeof(g_cellBuffer));
	if (0 != strncmp(g_cellBuffer, "+CMGL",5)) return NULL;


	size_t pos = 0;
	while(1){
		usart0_readLine(g_cellBuffer, sizeof(g_cellBuffer));
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

	usart0_puts("AT+CMGS=\"");
	usart0_puts(number);
	usart0_puts("\"\r");
	usart0_puts(msg);
	usart0_putchar(26);
	usart0_readLine(g_cellBuffer, sizeof(g_cellBuffer));
	usart0_readLine(g_cellBuffer, sizeof(g_cellBuffer));
	usart0_readLine(g_cellBuffer, sizeof(g_cellBuffer));
	usart0_readLine(g_cellBuffer, sizeof(g_cellBuffer));
	usart0_readLine(g_cellBuffer, sizeof(g_cellBuffer));

	return 0;
}
