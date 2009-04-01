#ifndef USB_COMM_H_
#define USB_COMM_H_

#include "FreeRTOS.h"

char * readLine();

void onUSBCommTask(void *);

void SendBytes(char *data, unsigned int length);

void SendString(const char *s);

void SendInt(int n);

void SendFloat(float f,int precision);

void SendDouble(double f, int precision);

void SendUint(unsigned int n);

void SendCrlf();

#endif /*USB_COMM_H_*/
