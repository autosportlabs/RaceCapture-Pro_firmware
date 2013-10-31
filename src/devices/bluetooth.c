#include "bluetooth.h"
#include "race_capture/printk.h"
#include "mod_string.h"
#include "FreeRTOS.h"
#include "task.h"

#define COMMAND_WAIT 	600

static int readBtWait(DeviceConfig *config, size_t delay) {
	int c = config->serial->get_line_wait(config->buffer, config->length, delay);
	return c;
}

static void flushBt(DeviceConfig *config) {
	config->buffer[0] = '\0';
	config->serial->flush();
}

void putsBt(DeviceConfig *config, const char *data) {
	config->serial->put_s(data);
}

static int sendBtCommandWaitResponse(DeviceConfig *config, const char *cmd, const char *rsp, size_t wait) {
	flushBt(config);
	vTaskDelay(COMMAND_WAIT);
	putsBt(config, cmd);
	readBtWait(config, wait);
	pr_debug("btrsp: ");
	pr_debug(config->buffer);
	pr_debug("\n");
	int res = strncmp(config->buffer, rsp, strlen(rsp));
	pr_debug(res == 0 ? "btMatch\n" : "btnomatch\n");
	return  res == 0;
}

static int sendBtCommandWait(DeviceConfig *config, const char *cmd, size_t wait) {
	return sendBtCommandWaitResponse(config, cmd, "OK", COMMAND_WAIT);
}

static int sendCommand(DeviceConfig *config, const char * cmd) {
	pr_debug("btcmd: ");
	pr_debug(cmd);
	pr_debug("\n");
	return sendBtCommandWait(config, cmd, COMMAND_WAIT);
}

static int configureBt(DeviceConfig *config) {
	//set baud rate
	if (!sendCommand(config, "AT+BAUD8"))
		return -1;

	config->serial->init(8, 0, 1, 115200);
	//set Device Name
	if (!sendBtCommandWaitResponse(config, "AT+NAMERaceCapturePro", "OK", COMMAND_WAIT))
		return -2;
	return 0;
}

int bt_init_connection(DeviceConfig *config){
	config->serial->init(8, 0, 1, 9600);
	if (sendCommand(config, "AT")) {
		if (configureBt(config) != 0)
			return DEVICE_INIT_FAIL;
	}
	config->serial->init(8, 0, 1, 115200);
	if (sendCommand(config, "AT")) return DEVICE_INIT_SUCCESS; else return DEVICE_INIT_FAIL;
}

int bt_check_connection_status(DeviceConfig *config){
	return DEVICE_STATUS_NO_ERROR;
}
