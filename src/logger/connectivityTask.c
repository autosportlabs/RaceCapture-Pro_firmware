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
#include "api.h"
#include "devices_common.h"
#include "capabilities.h"
#include "mem_mang.h"
#include "taskUtil.h"
//devices
#include "null_device.h"
#include "bluetooth.h"
#include "sim900.h"


#if (TELEMETRY_PORT == WIRELESS_PORT)
#define CONNECTIVITY_TASK_COUNT 1
#define CONNECTIVITY_TASK_INIT {NULL}
#else
#define CONNECTIVITY_TASK_COUNT 2
#define CONNECTIVITY_TASK_INIT {NULL, NULL}
#endif

//wait time for sample queue. can be portMAX_DELAY to wait forever, or zero to not wait at all
#define TELEMETRY_QUEUE_WAIT_TIME					0
//#define TELEMETRY_QUEUE_WAIT_TIME					portMAX_DELAY


#define IDLE_TIMEOUT							configTICK_RATE_HZ / 10
#define INIT_DELAY	 							600
#define BUFFER_SIZE 							1025

#define TELEMETRY_STACK_SIZE  					1000
#define SAMPLE_RECORD_QUEUE_SIZE				10
#define BAD_MESSAGE_THRESHOLD					10

#define METADATA_SAMPLE_INTERVAL				100

static xQueueHandle g_sampleQueue[CONNECTIVITY_TASK_COUNT] = CONNECTIVITY_TASK_INIT;


static size_t trimBuffer(char *buffer, size_t count){

	char *c = (buffer + count - 1);
	while (count > 0 && (*c == '\r' || *c == '\n')){
		*c = '\0';
		c--;
		count--;
	}
	return count;
}

static int processRxBuffer(Serial *serial, char *g_buffer, size_t *g_rxCount){
	size_t count = serial->get_line_wait(g_buffer + *g_rxCount, BUFFER_SIZE - *g_rxCount, 0);
	*g_rxCount += count;
	int processMsg = 0;

	if (*g_rxCount >= BUFFER_SIZE - 1){
		g_buffer[BUFFER_SIZE - 1] = '\0';
		*g_rxCount = BUFFER_SIZE - 1;
		processMsg = 1;
		pr_error("Rx Buffer overflow:");
		pr_error(g_buffer);
		pr_error("\r\n");
	}
	if ('\n' == g_buffer[*g_rxCount - 1]){
		*g_rxCount = trimBuffer(g_buffer, *g_rxCount);
		processMsg = 1;
	}
	return processMsg;
}

void queueTelemetryRecord(LoggerMessage *msg){
	for (size_t i = 0; i < CONNECTIVITY_TASK_COUNT; i++){
		xQueueHandle queue = g_sampleQueue[i];
		if (NULL != queue) xQueueSend(queue, &msg, TELEMETRY_QUEUE_WAIT_TIME);
	}
}

//combined telemetry - for when there's only one telemetry / wireless port available on system
//e.g. "Y-adapter" scenario
static void createConnectionParamsForCombinedTelemetry(int16_t priority){
	ConnectivityConfig *connConfig = &getWorkingLoggerConfig()->ConnectivityConfigs;
	size_t btEnabled = connConfig->bluetoothConfig.btEnabled;
	size_t cellEnabled = connConfig->cellularConfig.cellEnabled;

	if (btEnabled || cellEnabled){
		ConnParams * params = (ConnParams *)portMalloc(sizeof(ConnParams));

		params->periodicMeta = btEnabled && cellEnabled;

		//defaults
		params->check_connection_status = &null_device_check_connection_status;
		params->init_connection = &null_device_init_connection;
		params->serial = TELEMETRY_PORT;

		if (btEnabled){
			params->check_connection_status = &bt_check_connection_status;
			params->init_connection = &bt_init_connection;
		}
		//cell overrides wireless
		if (cellEnabled){
			params->check_connection_status = &sim900_check_connection_status;
			params->init_connection = &sim900_init_connection;
		}
		xTaskCreate(connectivityTask, (signed portCHAR *) "connTask", TELEMETRY_STACK_SIZE, params, priority, NULL );
	}
}

static void createConnectionParamsForBluetooth(int16_t priority){
	ConnectivityConfig *connConfig = &getWorkingLoggerConfig()->ConnectivityConfigs;
	if (connConfig->bluetoothConfig.btEnabled){
		ConnParams * params = (ConnParams *)portMalloc(sizeof(ConnParams));
		params->periodicMeta = 0;
		params->check_connection_status = &bt_check_connection_status;
		params->init_connection = &bt_init_connection;
		params->serial = WIRELESS_PORT;
		xTaskCreate(connectivityTask, (signed portCHAR *) "connWireless", TELEMETRY_STACK_SIZE, params, priority, NULL );
	}
}

static void createConnectionParamsForCellularTelemetry(int16_t priority){
	ConnectivityConfig *connConfig = &getWorkingLoggerConfig()->ConnectivityConfigs;
	if (connConfig->cellularConfig.cellEnabled){
		ConnParams * params = (ConnParams *)portMalloc(sizeof(ConnParams));
		params->periodicMeta = 0;
		params->check_connection_status = &sim900_check_connection_status;
		params->init_connection = &sim900_init_connection;
		params->serial = TELEMETRY_PORT;
		xTaskCreate(connectivityTask, (signed portCHAR *) "connTelemetry", TELEMETRY_STACK_SIZE, params, priority, NULL );
	}
}

void startConnectivityTask(int16_t priority){
	for (size_t i = 0; i < CONNECTIVITY_TASK_COUNT; i++){
		g_sampleQueue[i] = xQueueCreate(SAMPLE_RECORD_QUEUE_SIZE,sizeof( LoggerMessage *));
		if (NULL == g_sampleQueue[i]){
			pr_error("fatal: could not create sample queue\r\n");
			return;
		}
	}

	switch (CONNECTIVITY_TASK_COUNT){
	case 1:
		createConnectionParamsForCombinedTelemetry(priority);
		break;
	case 2:
		createConnectionParamsForBluetooth(priority);
		createConnectionParamsForCellularTelemetry(priority);
		break;
	default:
		pr_error("invalid connectivity task count!");
		break;
	}
}

void connectivityTask(void *params) {

	char * g_buffer = (char *)portMalloc(BUFFER_SIZE);
	size_t rxCount = 0;

	ConnParams *connParams = (ConnParams*)params;
	LoggerMessage *msg = NULL;

	Serial *serial = get_serial(connParams->serial);
	size_t periodicMeta = connParams->periodicMeta;

	DeviceConfig deviceConfig;
	deviceConfig.serial = serial;
	deviceConfig.buffer = g_buffer;
	deviceConfig.length = BUFFER_SIZE;

	size_t tick = 0;
	while (1) {
		while (connParams->init_connection(&deviceConfig) != DEVICE_INIT_SUCCESS) {
			pr_info("device not connected. retrying..\r\n");
			vTaskDelay(INIT_DELAY);
		}
		serial->flush();
		rxCount = 0;
		size_t badMsgCount = 0;
		while (1) {
			//wait for the next sample record
			char res = xQueueReceive(g_sampleQueue, &(msg), IDLE_TIMEOUT);

			////////////////////////////////////////////////////////////
			// Process a pending message from logger task, if exists
			////////////////////////////////////////////////////////////
			if (pdFALSE != res) {
				switch(msg->messageType){
					case LOGGER_MSG_START_LOG:
					{
						api_sendLogStart(serial);
						put_crlf(serial);
						tick = 0;
						break;
					}
					case LOGGER_MSG_END_LOG:
					{
						api_sendLogEnd(serial);
						put_crlf(serial);
						break;
					}
					case LOGGER_MSG_SAMPLE:
					{
						int sendMeta = (tick == 0 || (periodicMeta && (tick % METADATA_SAMPLE_INTERVAL == 0)));
						api_sendSampleRecord(serial, msg->channelSamples, msg->sampleCount, tick, sendMeta);
						put_crlf(serial);
						tick++;
						break;
					}
					default:
					{
						pr_warning("unknown logger msg type ");
						pr_warning_int(msg->messageType);
						pr_warning("\r\n");
						break;
					}
				}
			}

			////////////////////////////////////////////////////////////
			// Process incoming message, if available
			////////////////////////////////////////////////////////////
			//read in available characters, process message as necessary
			rxCount = 0;
			int msgReceived = processRxBuffer(serial, g_buffer, &rxCount);
			//check the latest contents of the buffer for something that might indicate an error condition
			if (connParams->check_connection_status(&deviceConfig) != DEVICE_STATUS_NO_ERROR){
				pr_info("device disconnected\n");
				break;
			}

			//now process a complete message if available
			if (msgReceived){
				if (DEBUG_LEVEL){
					pr_debug("msg rx: '");
					pr_debug(g_buffer);
					pr_debug("'\r\n");
				}
				int msgRes = process_api(serial, g_buffer, BUFFER_SIZE);
				if (! API_MSG_SUCCESS(msgRes)) badMsgCount++;
				if (badMsgCount >= BAD_MESSAGE_THRESHOLD){
					pr_warning_int(badMsgCount);
					pr_warning(" empty/bad msgs - re-connecting...\r\n");
					break;
				}
			}
		}
	}
}
