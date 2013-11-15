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
#include "printk.h"
#include "messaging.h"
#include "telemetryTask.h"
#include "devices_common.h"

//devices
#include "null_device.h"
#include "bluetooth.h"
#include "sim900.h"


//wait time for sample queue. can be portMAX_DELAY to wait forever, or zero to not wait at all
#define TELEMETRY_QUEUE_WAIT_TIME					0
//#define TELEMETRY_QUEUE_WAIT_TIME					portMAX_DELAY


#define IDLE_TIMEOUT	configTICK_RATE_HZ / 10
#define INIT_DELAY	 	600
#define BUFFER_SIZE 	201

#define TELEMETRY_TASK_PRIORITY					( tskIDLE_PRIORITY + 4 )
#define TELEMETRY_STACK_SIZE  					1000
#define SAMPLE_RECORD_QUEUE_SIZE				10


static char g_buffer[BUFFER_SIZE];
static size_t g_rxCount;
static xQueueHandle g_sampleQueue;
static ConnParams g_connParams;

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

portBASE_TYPE queueTelemetryRecord(SampleRecord * sr){
	if (NULL != g_sampleQueue){
		return xQueueSend(g_sampleQueue, &sr, TELEMETRY_QUEUE_WAIT_TIME);
	}
	else{
		return errQUEUE_EMPTY;
	}
}

void createConnectivityTask(){
	g_sampleQueue = xQueueCreate(SAMPLE_RECORD_QUEUE_SIZE,sizeof( SampleRecord *));
	if (NULL == g_sampleQueue){
		//TODO log error
		return;
	}

	switch(getWorkingLoggerConfig()->ConnectivityConfigs.connectivityMode){
		case CONNECTIVITY_MODE_CONSOLE:
			g_connParams.check_connection_status = &null_device_check_connection_status;
			g_connParams.init_connection = &null_device_init_connection;
			break;
		case CONNECTIVITY_MODE_BLUETOOTH:
			g_connParams.check_connection_status = &bt_check_connection_status;
			g_connParams.init_connection = &bt_init_connection;
			break;
		case CONNECTIVITY_MODE_CELL:
			g_connParams.check_connection_status = &sim900_check_connection_status;
			g_connParams.init_connection = &sim900_init_connection;
			break;
	}
	xTaskCreate(connectivityTask, (signed portCHAR *) "connTask", TELEMETRY_STACK_SIZE, &g_connParams, TELEMETRY_TASK_PRIORITY, NULL );
}

void connectivityTask(void *params) {

	ConnParams *connParams = (ConnParams*)params;
	SampleRecord *sr = NULL;
	uint32_t sampleTick = 0;

	Serial *serial = get_serial_usart0();

	DeviceConfig deviceConfig;
	deviceConfig.serial = serial;
	deviceConfig.buffer = g_buffer;
	deviceConfig.length = BUFFER_SIZE;

	int tick = 0;
	while (1) {
		while (connParams->init_connection(&deviceConfig) != DEVICE_INIT_SUCCESS) {
			pr_info("device not connected. retrying..\r\n");
			vTaskDelay(INIT_DELAY);
		}
		serial->flush();
		g_rxCount = 0;
		while (1) {
			//wait for the next sample record
			char res = xQueueReceive(g_sampleQueue, &(sr), IDLE_TIMEOUT);
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
			if (connParams->check_connection_status(&deviceConfig) != DEVICE_STATUS_NO_ERROR){
				pr_info("device disconnected\n");
				break;
			}

			//now process a complete message if necessary
			if (msgReceived){
				pr_debug("msg rx:");
				pr_debug(g_buffer);
				pr_debug("\n");
				if (g_buffer[0] == '{') process_api(serial, g_buffer, BUFFER_SIZE);
				g_rxCount = 0;
			}
		}
	}
}
