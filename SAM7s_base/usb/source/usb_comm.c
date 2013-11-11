#include "usb_comm.h"
#include "FreeRTOS.h"
#include "task.h"
#include "mod_string.h"
#include "USB-CDC.h"
#include "board.h"
#include "modp_numtoa.h"
#include "modp_atonum.h"
#include "memory.h"
#include "serial.h"
#include "messaging.h"

#define BUFFER_SIZE MEMORY_PAGE_SIZE * 2

static char lineBuffer[BUFFER_SIZE];


void usb_init(unsigned int bits, unsigned int parity, unsigned int stopBits, unsigned int baud){
	//null function - does not apply to USB CDC
}

void usb_flush(void)
{
	while(usb_getchar_wait(0));
}

char usb_getchar_wait(size_t delay){
	char c = 0;
	vUSBReceiveByteDelay(&c, delay);
	return c;
}

char usb_getchar(void){
	return usb_getchar_wait(portMAX_DELAY);
}

int usb_readLine(char *s, int len)
{
	return usb_readLineWait(s,len,portMAX_DELAY);
}

int usb_readLineWait(char *s, int len, size_t delay)
{
	int count = 0;
	while(count < len - 1){
		int c = usb_getchar_wait(delay);
		if (c == 0) break; //timeout
		*s++ = c;
		count++;
		if (c == '\n') break;
	}
	*s = '\0';
	return count;
}

void usb_puts(const char *s){
	while ( *s ){
		vUSBSendByte(*s++ );
	}
}

void usb_putchar(char c){
	vUSBSendByte(c);
}

void onUSBCommTask(void *pvParameters) {
	while (!vUSBIsInitialized()){
		vTaskDelay(1);
	}
	Serial *serial = get_serial_usb();

	while (1) {
		process_msg_interactive(serial, lineBuffer, BUFFER_SIZE);
	}
}


