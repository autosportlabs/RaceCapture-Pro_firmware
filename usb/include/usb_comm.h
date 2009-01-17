#ifndef USB_COMM_H_
#define USB_COMM_H_

#include "FreeRTOS.h"

void onUSBCommTask(void *);

void SendBytes(char *data, unsigned int length);

void SendString(char *s);

#endif /*USB_COMM_H_*/
