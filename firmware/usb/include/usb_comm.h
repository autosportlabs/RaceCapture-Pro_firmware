#ifndef USB_COMM_H_
#define USB_COMM_H_

#include "FreeRTOS.h"

//* Global variable


void onUSBEchoTask(void *);

//On revolution
void onUSBCommTask(void *);

void SendBytes(portCHAR *data, unsigned int length);

void SendString(portCHAR *s);

#endif /*USB_COMM_H_*/
