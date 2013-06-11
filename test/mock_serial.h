#ifndef MOCKSERIAL_H_
#define MOCKSERIAL_H_

#include "serial.h"


void mock_flush(void);

char mock_get_c_wait(size_t delay);

char mock_get_c();

int mock_put_c(char c);

int mock_put_s(const char* s );

int mock_get_line_wait(char *s, int len, size_t delay);

int mock_get_line(char *s, int len);

void setupMockSerial();

Serial * getMockSerial();

void mock_setBuffer(char *src);

void mock_appendBuffer(char *src);

#endif /* MOCKSERIAL_H_ */
