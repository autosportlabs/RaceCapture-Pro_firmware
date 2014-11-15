#ifndef CELLMODEM_H_
#define CELLMODEM_H_
#include "stdint.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "serial.h"

void setCellBuffer(char *buffer, size_t len);
int loadDefaultCellConfig(Serial *serial);
int initCellModem(Serial *serial);
int configureTexting(Serial *serial);
int configureNet(Serial *serial, const char *apnHost, const char *apnUser, const char *apnPass);
int connectNet(Serial *serial, const char *host, const char *port, int udpMode);
int closeNet(Serial *serial);
int isNetConnectionErrorOrClosed();
const char * readsCell(Serial *serial, portTickType timeout);
void putsCell(Serial *serial, const char *data);
void putQuotedStringCell(Serial *serial, char *s);
void putUintCell(Serial *serial, uint32_t num);
void putIntCell(Serial *serial, int num);
void putFloatCell(Serial *serial, float num, int precision);
int sendText(Serial *serial, const char * number, const char *msg);
void receiveText(Serial *serial, int txtNumber, char *textMsgBuffer, size_t textMsgBufferLen);
void deleteAllTexts(Serial *serial);
void deleteInbox(Serial *serial);
void deleteSent(Serial *serial);
#endif

