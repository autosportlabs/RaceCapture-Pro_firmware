#ifndef CELLMODEM_H_
#define CELLMODEM_H_

int initCellModem(void);
int sendText(const char * number, const char *msg);
const char * receiveText();

#endif

