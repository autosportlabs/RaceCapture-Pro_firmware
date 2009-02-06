#ifndef USB_COMM_H_
#define USB_COMM_H_

#include "FreeRTOS.h"

void onUSBCommTask(void *);

void SendBytes(char *data, unsigned int length);

void SendString(char *s);

void SendNumber(int n);

void SendUint(unsigned int n);

void SendCrlf();

#endif /*USB_COMM_H_*/
