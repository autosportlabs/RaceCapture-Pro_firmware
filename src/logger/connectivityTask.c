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

//devices
#include "null_device.h"
#include "bluetooth.h"
#include "sim900.h"


//wait time for sample queue. can be portMAX_DELAY to wait forever, or zero to not wait at all
#define TELEMETRY_QUEUE_WAIT_TIME					0
//#define TELEMETRY_QUEUE_WAIT_TIME					portMAX_DELAY


#define IDLE_TIMEOUT	configTICK_RATE_HZ / 10
#define INIT_DELAY	 	600
#define BUFFER_SIZE 	1025

#define TELEMETRY_STACK_SIZE  					1000
#define SAMPLE_RECORD_QUEUE_SIZE				10
#define BAD_MESSAGE_THRESHOLD					10

#define METADATA_SAMPLE_INTERVAL				100
static char g_buffer[BUFFER_SIZE];
static size_t g_rxCount;
static xQueueHandle g_sampleQueue;
static ConnParams g_connParams;


static size_t trimBuffer(char *buffer, size_t count){

	char *c = (buffer + count - 1);
	while (count > 0 && (*c == '\r' || *c == '\n')){
		*c = '\0';
		c--;
		count--;
	}
	return count;
}

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
		g_rxCount = trimBuffer(g_buffer, g_rxCount);
		processMsg = 1;
	}
	return processMsg;
}

portBASE_TYPE queueTelemetryRecord(LoggerMessage *msg){
	if (NULL != g_sampleQueue){
		return xQueueSend(g_sampleQueue, &msg, TELEMETRY_QUEUE_WAIT_TIME);
	}
	else{
		return errQUEUE_EMPTY;
	}
}

void startConnectivityTask(int priority){
	g_sampleQueue = xQueueCreate(SAMPLE_RECORD_QUEUE_SIZE,sizeof( LoggerMessage *));
	if (NULL == g_sampleQueue){
		pr_error("fatal: could not create samplequeue\r\n");
		return;
	}
	//defaults
	g_connParams.check_connection_status = &null_device_check_connection_status;
	g_connParams.init_connection = &null_device_init_connection;

	ConnectivityConfig *connConfig = &getWorkingLoggerConfig()->ConnectivityConfigs;
	if (connConfig->bluetoothConfig.btEnabled){
		g_connParams.check_connection_status = &bt_check_connection_status;
		g_connParams.init_connection = &bt_init_connection;
	}
	//cell overrides bluetooth
	if (connConfig->cellularConfig.cellEnabled){
		g_connParams.check_connection_status = &sim900_check_connection_status;
		g_connParams.init_connection = &sim900_init_connection;
	}

	xTaskCreate(connectivityTask, (signed portCHAR *) "connTask", TELEMETRY_STACK_SIZE, &g_connParams, priority, NULL );
}

void connectivityTask(void *params) {

	ConnectivityConfig *connConfig = &getWorkingLoggerConfig()->ConnectivityConfigs;

	ConnParams *connParams = (ConnParams*)params;
	LoggerMessage *msg = NULL;

	Serial *serial = get_serial(SERIAL_WIRELESS);

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
		g_rxCount = 0;
		size_t badMsgCount = 0;
		int readOnlyBluetooth = connConfig->bluetoothConfig.btEnabled == BLUETOOTH_ENABLED && connConfig->cellularConfig.cellEnabled == CELL_ENABLED;
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
						int sendMeta = (tick == 0 || (readOnlyBluetooth && (tick % METADATA_SAMPLE_INTERVAL == 0)));
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
			g_rxCount = 0;
			int msgReceived = processRxBuffer(serial);
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
