#ifndef CELLMODEM_H_
#define CELLMODEM_H_
#include "stdint.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

int loadDefaultCellConfig();
int initCellModem(void);
void powerDownCellModem();
int configureTexting(void);
int configureNet(const char *apnHost, const char *apnUser, const char *apnPass);
int connectNet(const char *host, const char *port, int udpMode);
int closeNet();
int isNetConnectionErrorOrClosed();
int startUdpData();
int startNetData();
int endNetData();
const char * readsCell(portTickType timeout);
void putsCell(const char *data);
void putQuotedStringCell(char *s);
void putUintCell(uint32_t num);
void putIntCell(int num);
void putFloatCell(float num, int precision);
int sendText(const char * number, const char *msg);
void receiveText(int txtNumber, char *textMsgBuffer, size_t textMsgBufferLen);
void deleteAllTexts(void);
void deleteInbox(void);
void deleteSent(void);
#endif

