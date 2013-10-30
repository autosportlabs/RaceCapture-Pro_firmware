#include "btTelemetry.h"
#include "task.h"
#include "modp_numtoa.h"
#include "mod_string.h"
#include "loggerHardware.h"
#include "loggerApi.h"
#include "serial.h"
#include "usart.h"
#include "race_capture/printk.h"
#include "messaging.h"

#define IDLE_TIMEOUT	configTICK_RATE_HZ / 10
#define COMMAND_WAIT 	600
#define BUFFER_SIZE 	200

static char g_buffer[BUFFER_SIZE];
size_t g_rxCount;

static int readBtWait(portTickType delay) {
	int c = usart0_readLineWait(g_buffer, BUFFER_SIZE, delay);
	return c;
}

static void flushBt(void) {
	g_buffer[0] = '\0';
	usart0_flush();
}

void putsBt(const char *data) {
	usart0_puts(data);
}

static int sendCommandWaitResponse(const char *cmd, const char *rsp, portTickType wait) {
	flushBt();
	vTaskDelay(COMMAND_WAIT);
	putsBt(cmd);
	readBtWait(wait);
	pr_debug("btrsp: ");
	pr_debug(g_buffer);
	pr_debug("\n");
	int res = strncmp(g_buffer, rsp, strlen(rsp));
	pr_debug(res == 0 ? "btMatch\n" : "btnomatch\n");
	return  res == 0;
}

static int sendCommandWait(const char *cmd, portTickType wait) {
	return sendCommandWaitResponse(cmd, "OK", COMMAND_WAIT);
}

static int sendCommand(const char * cmd) {
	pr_debug("btcmd: ");
	pr_debug(cmd);
	pr_debug("\n");
	return sendCommandWait(cmd, COMMAND_WAIT);
}

static int configureBt() {
	//set baud rate
	if (!sendCommand("AT+BAUD8"))
		return -1;
	initUsart0(8, 0, 1, 115200);
	//set Device Name
	if (!sendCommandWaitResponse("AT+NAMERaceCapturePro", "OK", COMMAND_WAIT))
		return -2;
	return 0;
}

static int initBluetooth() {
	initUsart0(8, 0, 1, 9600);
	if (sendCommand("AT")) {
		if (configureBt() != 0)
			return -1;
	}
	initUsart0(8, 0, 1, 115200);
	if (sendCommand("AT")) return 0; else return -1;
}

static void processRxMessage(Serial *serial){
	size_t count = serial->get_line_wait(g_buffer + g_rxCount, BUFFER_SIZE - g_rxCount, 0);
	g_rxCount += count;
	if (g_rxCount >= BUFFER_SIZE - 1){
		pr_error("Rx Buffer overflow:");
		pr_error(g_buffer);
		g_rxCount = 0;
	}
	if ('\n' == g_buffer[g_rxCount - 1]){
		process_msg(serial,g_buffer, BUFFER_SIZE);
		g_rxCount = 0;
	}
}

void btTelemetryTask(void *params) {

	xQueueHandle sampleRecordQueue = (xQueueHandle) params;
	SampleRecord *sr = NULL;
	uint32_t sampleTick = 0;

	int tick = 0;
	Serial *serial = get_serial_usart0();

	while (1) {
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
			processRxMessage(serial);
		}
	}
}
