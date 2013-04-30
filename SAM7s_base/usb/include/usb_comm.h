#ifndef USB_COMM_H_
#define USB_COMM_H_

#include "FreeRTOS.h"

void usb_flush(void);

int usb_readLine(char *s, int len);

int usb_readLineWait(char *s, int len, portTickType delay);

char usb_getchar(void);

char usb_getchar_wait(portTickType delay);

int usb_putchar(char c);

int usb_puts(const char* s );

void onUSBCommTask(void *);


#endif /*USB_COMM_H_*/
