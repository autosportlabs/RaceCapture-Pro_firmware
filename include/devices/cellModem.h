#ifndef CELLMODEM_H_
#define CELLMODEM_H_
#include <stdint.h>
#include "serial.h"
#include "loggerConfig.h"

typedef enum {
    CELLMODEM_STATUS_NOT_INIT = 0,
    CELLMODEM_STATUS_PROVISIONED,
    CELLMODEM_STATUS_NO_NETWORK
} cellmodem_status_t;

cellmodem_status_t cellmodem_get_status( void );

int cell_get_signal_strength();
char * cell_get_subscriber_number();
char * cell_get_IMEI();
void setCellBuffer(char *buffer, size_t len);
int loadDefaultCellConfig(Serial *serial);
int initCellModem(Serial *serial, CellularConfig *cellCfg);
int configureNet(Serial *serial);
int connectNet(Serial *serial, const char *host, const char *port, int udpMode);
int closeNet(Serial *serial);
int isNetConnectionErrorOrClosed();
const char * readsCell(Serial *serial, size_t timeout);
void putsCell(Serial *serial, const char *data);
void putQuotedStringCell(Serial *serial, char *s);
void putUintCell(Serial *serial, uint32_t num);
void putIntCell(Serial *serial, int num);
void putFloatCell(Serial *serial, float num, int precision);
#endif

