#ifndef MOCKSERIAL_H_
#define MOCKSERIAL_H_

#include "serial.h"


void mock_flush(void);

int mock_get_c_wait(char *c, size_t delay);

char mock_get_c();

void mock_put_c(char c);

void mock_put_s(const char* s );

int mock_get_line_wait(char *s, int len, size_t delay);

int mock_get_line(char *s, int len);

void setupMockSerial();

Serial * getMockSerial();

void mock_setRxBuffer(const char *src);

char * mock_getTxBuffer();

void mock_appendRxBuffer(const char *src);

void mock_resetTxBuffer();

#endif /* MOCKSERIAL_H_ */
