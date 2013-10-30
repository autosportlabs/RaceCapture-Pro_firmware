#include "connectivityTask.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "loggerConfig.h"
#include "sampleRecord.h"
#include "modp_numtoa.h"
#include "stdint.h"
#include "mod_string.h"
#include "loggerHardware.h"
#include "loggerApi.h"
#include "serial.h"
#include "usart.h"
#include "race_capture/printk.h"
#include "messaging.h"

#define IDLE_TIMEOUT	configTICK_RATE_HZ / 10
#define INIT_DELAY	 	600
#define BUFFER_SIZE 	201

static char g_buffer[BUFFER_SIZE];
size_t g_rxCount;

static int processRxBuffer(Serial *serial){
	size_t count = serial->get_line_wait(g_buffer + g_rxCount, BUFFER_SIZE - g_rxCount, 0);
	g_rxCount += count;
	int processMsg = 0;

	if (g_rxCount >= BUFFER_SIZE - 1){
		g_buffer[BUFFER_SIZE - 1] = '\0';
		g_rxCount = BUFFER_SIZE - 1;
		processMsg = 1;
		pr_error("Rx Buffer overflow:");
		pr_error(g_buffer);
		pr_error("\r\n");
	}

	if ('\n' == g_buffer[g_rxCount - 1]){
		g_buffer[g_rxCount - 1] = '\0';
		processMsg = 1;
	}
	return processMsg;
}

static int resetRxBuffer(){
	g_rxCount = 0;
}

static int processRxMessage(Serial *serial){
	process_msg(serial, g_buffer, BUFFER_SIZE);
}

void connectivityTask(void *params) {

	ConnParams *connParams = (ConnParams*)params;
	xQueueHandle sampleRecordQueue = (xQueueHandle)params->sampleRecord;
	SampleRecord *sr = NULL;
	uint32_t sampleTick = 0;

	int tick = 0;
	Serial *serial = get_serial_usart0();

	while (1) {
		while (connParams->init_connection() != CONN_INIT_SUCCESS) {
			vTaskDelay(INIT_DELAY);
		}
		while (1) {
			//wait for the next sample record
			char res = xQueueReceive(sampleRecordQueue, &(sr), IDLE_TIMEOUT);
			sampleTick++;
			if (pdFALSE == res) {
				//perform idle task?
			} else {
				//a null sample record means end of sample run; like an EOF
				if (NULL != sr) {
					if (0 == tick){
						api_sendLogStart(serial);
						put_crlf(serial);
					}
					++tick;
					api_sendSampleRecord(serial, sr, tick, tick == 1);
					put_crlf(serial);
				}
				else{
					api_sendLogEnd(serial);
					put_crlf(serial);
					//end of sample
					tick = 0;
				}
			}

			//read in available characters, process message as necessary
			int msgReceived = processRxBuffer(serial);

			//check the latest contents of the buffer for something that might indicate an error condition
			if (connParams->check_connection_status(g_buffer, BUFFER_SIZE) != CONN_STATUS_NO_ERROR) break;

			//now process a complete message if necessary
			if (msgReceived){
				processRxMessage(serial);
				resetRxBuffer();
			}

			processRxMessage(serial, connParams);
		}
	}
}
