/*
 * mock_serial.cpp
 *
 *  Created on: Jun 10, 2013
 *      Author: brent
 */
#include "mock_serial.h"
#include "mod_string.h"
#include <stddef.h>
static Serial mockSerial;
static char rxBuffer[20000];
static char txBuffer[20000];
size_t bufIndex;

void setupMockSerial(){
	mockSerial.flush = &mock_flush;
	mockSerial.get_c = &mock_get_c;
	mockSerial.get_c_wait = &mock_get_c_wait;
	mockSerial.get_line = &mock_get_line;
	mockSerial.get_line_wait = &mock_get_line_wait;
	mockSerial.put_c = &mock_put_c;
	mockSerial.put_s = &mock_put_s;
}

char * mock_getTxBuffer(){
	return txBuffer;
}

void mock_setRxBuffer(const char *src){
	strcpy(rxBuffer, src);
	bufIndex = 0;
}

void mock_appendRxBuffer(const char *src){
	strcat(rxBuffer, src);
}

void mock_resetTxBuffer(){
	txBuffer[0] = '\0';
}

Serial * getMockSerial(){
	return &mockSerial;
}

void mock_flush(void)
{
	char rx;
}

char mock_get_c_wait(size_t delay){

	if (bufIndex < strlen(rxBuffer)){
		return rxBuffer[bufIndex++];
	}
	else{
		return 0;
	}
}

char mock_get_c()
{
	return mock_get_c_wait(0);
}

void mock_put_c(char c){
	char append[2];
	append[0] = c;
	append[1] = '\0';
	strcat(txBuffer, append);
}

void mock_put_s(const char* s )
{
	while ( *s ) mock_put_c(*s++ );
}

int mock_get_line_wait(char *s, int len, size_t delay)
{
	int count = 0;
	while(count < len - 1){
		int c = mock_get_c_wait(delay);
		if (c == 0) break; //timeout
		*s++ = c;
		count++;
		if (c == '\n') break;
	}
	*s = '\0';
	return count;
}

int mock_get_line(char *s, int len)
{
	return mock_get_line_wait(s,len,0);
}


