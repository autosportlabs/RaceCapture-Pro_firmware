#include "consoleConnectivity.h"
#include "task.h"
#include "serial.h"
#include "memory.h"
#include "usart.h"
#include "loggerApi.h"
#include "messaging.h"

#define BUFFER_SIZE MEMORY_PAGE_SIZE * 2
static char g_buffer[BUFFER_SIZE];
#define IDLE_TIMEOUT	configTICK_RATE_HZ / 1

void consoleConnectivityTaskx(void *params){

	initUsart0(8, 0, 1, 230400);
	Serial *serial = get_serial_usart0();

	while (1) {
		process_message(serial, g_buffer, BUFFER_SIZE);
	}
}

void consoleConnectivityTask(void *params){

	int sendMeta = 0;
	int samplingActive = 0;
	xQueueHandle sampleRecordQueue = (xQueueHandle) params;

	initUsart0(8, 0, 1, 230400);
	Serial *serial = get_serial_usart0();

	while (1) {
		SampleRecord *sr = NULL;
		//wait for the next sample record
		char res = xQueueReceive(sampleRecordQueue, &(sr), IDLE_TIMEOUT);
		if (pdFALSE == res) {
			//initTxFrame(&g_xBeeFrame);
			//writeSampleRecordBinary(NULL,sampleTick);
		} else {
			if (0 == samplingActive) {
				sendMeta = 1;
			} else {
				break;
			}
		}
		if (samplingActive) {
			//a null sample record means end of sample run; like an EOF
			if (NULL != sr) {
				writeSampleRecord(serial, sr, sendMeta);
				sendMeta = 0;
			} else {
				samplingActive = 0;
			}
		}
	}
}
