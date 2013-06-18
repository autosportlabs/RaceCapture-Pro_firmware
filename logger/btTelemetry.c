#include "btTelemetry.h"
#include "task.h"
#include "modp_numtoa.h"
#include "mod_string.h"
#include "loggerHardware.h"
#include "usart.h"

//#define DEBUG

#define IDLE_TIMEOUT	configTICK_RATE_HZ / 1
#define COMMAND_WAIT 	600

static char g_buffer[200];
static int g_telemetryActive;

static int readBtWait(portTickType delay){
#ifdef DEBUG
	SendString("Read:");
#endif
	int c = usart0_readLineWait(g_buffer, sizeof(g_buffer),delay);
#ifdef DEBUG
	SendString(g_buffer);
#endif
	return c;
}

static void flushBt(void){
	g_buffer[0] = '\0';
	usart0_flush();
}


static int putcBt(char c){
	usart0_putchar(c);
	return 0;
}

void putsBt(const char *data){
	usart0_puts(data);
#ifdef DEBUG
	SendString("Write: ");
	SendString(data);
	SendCrlf();
#endif
}

static void putUintBt(uint32_t num){
	char buf[10];
	modp_uitoa10(num,buf);
	putsBt(buf);
}

static void putIntBt(int num){
	char buf[10];
	modp_itoa10(num,buf);
	putsBt(buf);
}

static void putFloatBt(float num, int precision){
	char buf[20];
	modp_ftoa(num, buf, precision);
	putsBt(buf);
}

static void putQuotedStringBt(char *s){
	putcBt('"');
	putsBt(s);
	putcBt('"');
}

static int sendCommandWaitResponse(const char *cmd, const char *rsp, portTickType wait){
	flushBt();
	vTaskDelay(COMMAND_WAIT);
	putsBt(cmd);
	readBtWait(wait);
	return strncmp(g_buffer,rsp, strlen(rsp)) == 0;
}

static int sendCommandWait(const char *cmd, portTickType wait){
	return sendCommandWaitResponse(cmd, "OK",COMMAND_WAIT);
}

static int sendCommand(const char * cmd){
	return sendCommandWait(cmd, COMMAND_WAIT);
}

static int configureBt(){
	//set baud rate
	if (!sendCommand("AT+BAUD9")) return -1;
	initUsart0(8, 0, 1, 230400);
	//set Device Name
	if (!sendCommandWaitResponse("AT+NAMERaceCapturePro","OK",COMMAND_WAIT)) return -2;
	return 0;
}

static int initBluetooth(){


	vTaskDelay(1000);

	initUsart0(8, 0, 1, 9600);
	if (sendCommand("AT")){
		if (configureBt() != 0) return -1;
	}
	initUsart0(8, 0, 1, 230400);
	if (!sendCommand("AT")) return -1;
	return 0;
}

static int writeAuthJSON(const char *deviceId){
	putsBt("t:{\"cmd\":{\"auth\":{\"deviceId\":\"");
	putsBt(deviceId);
	putsBt("\"}}}\n");

/*	int attempts = 20;
	while (attempts-- > 0){
		const char * data = readsBt(334); //~1000ms
		if (strncmp(data, "{\"status\":\"ok\"}",15) == 0) return 0;
	}
	return -1;
	*/
	return 0;
}

static int isNetConnectionErrorOrClosed(){
	return 0;
}


static void writeSampleRecordJSON(SampleRecord * sampleRecord, uint32_t sampleTick){

	//check for closed connection

	putsBt("t:{\"data\":{");
	int fieldCount = 0;
	putsBt("\"tick\":");
	putUintBt(sampleTick);
	putcBt(',');
	for (int i = 0; i < SAMPLE_RECORD_CHANNELS; i++){
		ChannelSample * sample = &(sampleRecord->Samples[i]);
		ChannelConfig * channelConfig = sample->channelConfig;

		if (SAMPLE_DISABLED == channelConfig->sampleRate) continue;

		if (sample->intValue == NIL_SAMPLE) continue;

		if (fieldCount++ > 0) putcBt(',');

		putQuotedStringBt(channelConfig->label);
		putcBt(':');
		int precision = sample->precision;
		if (precision > 0){
			putFloatBt(sample->floatValue,precision);
		}
		else{
			putIntBt(sample->intValue);
		}
	}
	putsBt("}}\n");
}

void btTelemetryTask(void *params){

	xQueueHandle sampleRecordQueue = (xQueueHandle)params;
	SampleRecord *sr = NULL;
	uint32_t sampleTick = 0;

	while(1){
		g_telemetryActive = 0;
		int btReady = 0;

		while (btReady == 0){
			if (0 == initBluetooth()){
					btReady = 1;
			}
		}

		while(1){
			//wait for the next sample record
			char res = xQueueReceive(sampleRecordQueue, &(sr), IDLE_TIMEOUT);
			sampleTick++;
			if (pdFALSE == res){
				//initTxFrame(&g_xBeeFrame);
				//writeSampleRecordBinary(NULL,sampleTick);
			}
			else{
				if (0 == g_telemetryActive){
					if (0 == writeAuthJSON("8f4240f8-0816-4896-8527-f1d7c5285ad9")){
						g_telemetryActive = 1;
					}
					else{
						break;
					}
				}
				if (g_telemetryActive){
					if (isNetConnectionErrorOrClosed()){
						g_telemetryActive = 0;
					}
					else{
						//a null sample record means end of sample run; like an EOF
						if (NULL != sr){
							writeSampleRecordJSON(sr,sampleTick);
						}
						else{
							g_telemetryActive = 0;
						}
					}
				}
			}
		}
	}
}

