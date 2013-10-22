#include "consoleConnectivity.h"
#include "task.h"
#include "serial.h"
#include "memory.h"
#include "usart.h"
#include "loggerApi.h"
#include "messaging.h"
#include "race_capture/printk.h"

#define BUFFER_SIZE MEMORY_PAGE_SIZE * 2
static char g_buffer[BUFFER_SIZE];
#define IDLE_TIMEOUT	configTICK_RATE_HZ / 1

void consoleConnectivityTaskx(void *params){

	initUsart0(8, 0, 1, 115200);
	Serial *serial = get_serial_usart0();

	while (1) {
		process_msg(serial, g_buffer, BUFFER_SIZE);
	}
}

void consoleConnectivityTask(void *params){

	int samplingActive = 0;
	int tick = 0;
	xQueueHandle sampleRecordQueue = (xQueueHandle) params;

	initUsart0(8, 0, 1, 115200);
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
				pr_debug("sample start\n");
				tick = 0;
			} else {
				break;
			}
		}
		if (samplingActive) {
			//a null sample record means end of sample run; like an EOF
			if (NULL != sr) {
				++tick;
				api_sendSampleRecord(serial, sr, tick, tick == 1);
				serial->put_s("\r\n");
				pr_debug("sample\n");
			} else {
				samplingActive = 0;
				pr_debug("sample end\n");
			}
		}
	}
}
