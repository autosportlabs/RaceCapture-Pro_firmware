#include "btTelemetry.h"
#include "task.h"
#include "modp_numtoa.h"
#include "mod_string.h"
#include "loggerHardware.h"
#include "loggerApi.h"
#include "serial.h"
#include "usart.h"

//#define DEBUG

#define IDLE_TIMEOUT	configTICK_RATE_HZ / 1
#define COMMAND_WAIT 	600

static char g_buffer[200];
static int g_telemetryActive;

static int readBtWait(portTickType delay) {
	int c = usart0_readLineWait(g_buffer, sizeof(g_buffer), delay);
	return c;
}

static void flushBt(void) {
	g_buffer[0] = '\0';
	usart0_flush();
}

void putsBt(const char *data) {
	usart0_puts(data);
}

static int sendCommandWaitResponse(const char *cmd, const char *rsp,
		portTickType wait) {
	flushBt();
	vTaskDelay(COMMAND_WAIT);
	putsBt(cmd);
	readBtWait(wait);
	return strncmp(g_buffer, rsp, strlen(rsp)) == 0;
}

static int sendCommandWait(const char *cmd, portTickType wait) {
	return sendCommandWaitResponse(cmd, "OK", COMMAND_WAIT);
}

static int sendCommand(const char * cmd) {
	return sendCommandWait(cmd, COMMAND_WAIT);
}

static int configureBt() {
	//set baud rate
	if (!sendCommand("AT+BAUD9"))
		return -1;
	initUsart0(8, 0, 1, 230400);
	//set Device Name
	if (!sendCommandWaitResponse("AT+NAMERaceCapturePro", "OK", COMMAND_WAIT))
		return -2;
	return 0;
}

static int initBluetooth() {
	vTaskDelay(1000);
	initUsart0(8, 0, 1, 9600);
	if (sendCommand("AT")) {
		if (configureBt() != 0)
			return -1;
	}
	initUsart0(8, 0, 1, 230400);
	if (!sendCommand("AT"))
		return -1;
	return 0;
}

void btTelemetryTask(void *params) {

	xQueueHandle sampleRecordQueue = (xQueueHandle) params;
	SampleRecord *sr = NULL;
	uint32_t sampleTick = 0;

	int sendMeta = 0;
	Serial *serial = get_serial_usart0();

	while (1) {
		g_telemetryActive = 0;
		int btReady = 0;

		while (btReady == 0) {
			if (initBluetooth() == 0) {
				btReady = 1;
			}
		}

		while (1) {
			//wait for the next sample record
			char res = xQueueReceive(sampleRecordQueue, &(sr), IDLE_TIMEOUT);
			sampleTick++;
			if (pdFALSE == res) {
				//initTxFrame(&g_xBeeFrame);
				//writeSampleRecordBinary(NULL,sampleTick);
			} else {
				if (0 == g_telemetryActive) {
					g_telemetryActive = 1;
					sendMeta = 1;
				} else {
					break;
				}
			}
			if (g_telemetryActive) {
				//a null sample record means end of sample run; like an EOF
				if (NULL != sr) {
					writeSampleRecord(serial, sr, sendMeta);
					sendMeta = 0;
				} else {
					g_telemetryActive = 0;
				}
			}
		}
	}
}

