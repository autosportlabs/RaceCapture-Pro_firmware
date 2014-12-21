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
#include "LED.h"
//devices
#include "null_device.h"
#include "bluetooth.h"
#include "sim900.h"


#if (CONNECTIVITY_CHANNELS == 1)
#define CONNECTIVITY_TASK_INIT {NULL}
#elif (CONNECTIVITY_CHANNELS == 2)
#define CONNECTIVITY_TASK_INIT {NULL, NULL}
#else
#error "invalid connectivity task count"
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

static xQueueHandle g_sampleQueue[CONNECTIVITY_CHANNELS] = CONNECTIVITY_TASK_INIT;


static size_t trimBuffer(char *buffer, size_t count){

	char *c = (buffer + count - 1);
	while (count > 0 && (*c == '\r' || *c == '\n')){
		*c = '\0';
		c--;
		count--;
	}
	return count;
}

static int processRxBuffer(Serial *serial, char *buffer, size_t *rxCount){
	size_t count = serial->get_line_wait(buffer + *rxCount, BUFFER_SIZE - *rxCount, 0);

	*rxCount += count;
	int processMsg = 0;

	if (*rxCount >= BUFFER_SIZE - 1){
		buffer[BUFFER_SIZE - 1] = '\0';
		*rxCount = BUFFER_SIZE - 1;
		processMsg = 1;
		pr_error("Rx Buffer overflow:");
		pr_error(buffer);
		pr_error("\r\n");
	}
	if (*rxCount > 0){
		char lastChar = buffer[*rxCount - 1];
		if ('\r' == lastChar || '\n' == lastChar){
			*rxCount = trimBuffer(buffer, *rxCount);
			processMsg = 1;
		}
	}
	return processMsg;
}

void queueTelemetryRecord(LoggerMessage *msg){
	for (size_t i = 0; i < CONNECTIVITY_CHANNELS; i++){
		xQueueHandle queue = g_sampleQueue[i];
		if (NULL != queue) xQueueSend(queue, &msg, TELEMETRY_QUEUE_WAIT_TIME);
	}
}

//combined telemetry - for when there's only one telemetry / wireless port available on system
//e.g. "Y-adapter" scenario
static void createCombinedTelemetryTask(int16_t priority, xQueueHandle sampleQueue){
	ConnectivityConfig *connConfig = &getWorkingLoggerConfig()->ConnectivityConfigs;
	size_t btEnabled = connConfig->bluetoothConfig.btEnabled;
	size_t cellEnabled = connConfig->cellularConfig.cellEnabled;

	if (btEnabled || cellEnabled){
		ConnParams * params = (ConnParams *)portMalloc(sizeof(ConnParams));

		params->periodicMeta = btEnabled && cellEnabled;

		//defaults
		params->check_connection_status = &null_device_check_connection_status;
		params->init_connection = &null_device_init_connection;
		params->serial = SERIAL_TELEMETRY;
		params->sampleQueue = sampleQueue;

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

static void createWirelessConnectionTask(int16_t priority, xQueueHandle sampleQueue, uint8_t isPrimary){
	ConnParams * params = (ConnParams *)portMalloc(sizeof(ConnParams));
	params->isPrimary = isPrimary;
	params->connectionName = "Wireless";
	params->periodicMeta = 0;
	params->check_connection_status = &bt_check_connection_status;
	params->init_connection = &bt_init_connection;
	params->serial = SERIAL_WIRELESS;
	params->sampleQueue = sampleQueue;
	xTaskCreate(connectivityTask, (signed portCHAR *) "connWireless", TELEMETRY_STACK_SIZE, params, priority, NULL );
}

static void createTelemetryConnectionTask(int16_t priority, xQueueHandle sampleQueue, uint8_t isPrimary){
	ConnParams * params = (ConnParams *)portMalloc(sizeof(ConnParams));
	params->isPrimary = isPrimary;
	params->connectionName = "Telemetry";
	params->periodicMeta = 0;
	params->check_connection_status = &sim900_check_connection_status;
	params->init_connection = &sim900_init_connection;
	params->serial = SERIAL_TELEMETRY;
	params->sampleQueue = sampleQueue;
	xTaskCreate(connectivityTask, (signed portCHAR *) "connTelemetry", TELEMETRY_STACK_SIZE, params, priority, NULL );
}

void startConnectivityTask(int16_t priority){
	for (size_t i = 0; i < CONNECTIVITY_CHANNELS; i++){
		g_sampleQueue[i] = xQueueCreate(SAMPLE_RECORD_QUEUE_SIZE,sizeof( LoggerMessage *));
		if (NULL == g_sampleQueue[i]){
			pr_error("fatal: could not create sample queue\r\n");
			return;
		}
	}

	switch (CONNECTIVITY_CHANNELS){
	case 1:
		createCombinedTelemetryTask(priority, g_sampleQueue[0]);
		break;
	case 2:
		{
			ConnectivityConfig *connConfig = &getWorkingLoggerConfig()->ConnectivityConfigs;
			//logic to control which connection is considered 'primary', which is used later to determine which task has control over LED flashing
			uint8_t cellEnabled = connConfig->cellularConfig.cellEnabled;
			uint8_t btEnabled = connConfig->bluetoothConfig.btEnabled;
			if (cellEnabled) createTelemetryConnectionTask(priority, g_sampleQueue[1], 1);
			if (btEnabled) createWirelessConnectionTask(priority, g_sampleQueue[0], !cellEnabled);
		}
		break;
	default:
		pr_error("invalid connectivity task count!");
		break;
	}
}

void connectivityTask(void *params) {

	char * buffer = (char *)portMalloc(BUFFER_SIZE);
	size_t rxCount = 0;

	ConnParams *connParams = (ConnParams*)params;
	LoggerMessage *msg = NULL;

	Serial *serial = get_serial(connParams->serial);

	uint8_t isPrimary = connParams->isPrimary;
	size_t periodicMeta = connParams->periodicMeta;
	xQueueHandle sampleQueue = connParams->sampleQueue;

	DeviceConfig deviceConfig;
	deviceConfig.serial = serial;
	deviceConfig.buffer = buffer;
	deviceConfig.length = BUFFER_SIZE;

	while (1) {
		while (connParams->init_connection(&deviceConfig) != DEVICE_INIT_SUCCESS) {
			pr_info("device not connected. retrying..\r\n");
			vTaskDelay(INIT_DELAY);
		}
		serial->flush();
		rxCount = 0;
		size_t badMsgCount = 0;
		size_t tick = 0;
		while (1) {
			//wait for the next sample record
			char res = xQueueReceive(sampleQueue, &(msg), IDLE_TIMEOUT);

            if (!isValidLoggerMessageAge(msg)) {
                pr_warning("Comm Task Logger message too old.  Ignoring it.\r\n");
                continue;
            }

			////////////////////////////////////////////////////////////
			// Process a pending message from logger task, if exists
			////////////////////////////////////////////////////////////
			if (pdFALSE != res) {
				switch(msg->type){
					case LoggerMessageType_Start:
					{
						api_sendLogStart(serial);
						put_crlf(serial);
						tick = 0;
						break;
					}
					case LoggerMessageType_Stop:
					{
						api_sendLogEnd(serial);
						put_crlf(serial);
						break;
					}
					case LoggerMessageType_Sample:
					{
						int sendMeta = (tick == 0 || (periodicMeta && (tick % METADATA_SAMPLE_INTERVAL == 0)));
						api_sendSampleRecord(serial, msg->channelSamples, msg->sampleCount, tick, sendMeta);
						if (isPrimary) LED_toggle(0);
						put_crlf(serial);
						tick++;
						break;
					}
					default:
					{
						pr_warning("unknown logger msg type ");
						pr_warning_int(msg->type);
						pr_warning("\r\n");
						break;
					}
				}
			}

			////////////////////////////////////////////////////////////
			// Process incoming message, if available
			////////////////////////////////////////////////////////////
			//read in available characters, process message as necessary
			int msgReceived = processRxBuffer(serial, buffer, &rxCount);
			//check the latest contents of the buffer for something that might indicate an error condition
			if (connParams->check_connection_status(&deviceConfig) != DEVICE_STATUS_NO_ERROR){
				pr_info("device disconnected\r\n");
				break;
			}
			//now process a complete message if available
			if (msgReceived){
				if (DEBUG_LEVEL){
					pr_debug(connParams->connectionName);
					pr_debug(": msg rx: '");
					pr_debug(buffer);
					pr_debug("'");
				}
				int msgRes = process_api(serial, buffer, BUFFER_SIZE);

				int msgError = (msgRes == API_ERROR_MALFORMED);
				if (DEBUG_LEVEL){
					if (msgError){
						pr_debug(" (failed) ");
					}
				}
				pr_debug("\r\n");
				if (msgError) badMsgCount++;
				if (badMsgCount >= BAD_MESSAGE_THRESHOLD){
					pr_warning_int(badMsgCount);
					pr_warning(" empty/bad msgs - re-connecting...\r\n");
					break;
				}
				else{
					badMsgCount = 0;
				}
				rxCount = 0;
			}
		}
	}
}
