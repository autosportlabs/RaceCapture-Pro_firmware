#include "cellTelemetry.h"
#include "cellModem.h"
#include "task.h"
#include "modp_numtoa.h"
#include "loggerHardware.h"
#include "loggerConfig.h"
#include "usart.h"
#include "mod_string.h"

static int g_telemetryActive;
#define IDLE_TIMEOUT							configTICK_RATE_HZ / 1


static int writeAuthJSON(const char *deviceId){
	//send linefeed at slow intervals until we have the auth packet ack from server
	for (int i = 0; i < 5; i++){
		putsCell(" ");
		vTaskDelay(84); //250ms pause
	}
	putsCell("{\"cmd\":{\"auth\":{\"deviceId\":\"");
	putsCell(deviceId);
	putsCell("\"}}}\n");

	int attempts = 20;
	while (attempts-- > 0){
		const char * data = readsCell(334); //~1000ms
		if (strncmp(data, "{\"status\":\"ok\"}",15) == 0) return 0;
	}
	return -1;
}

static void writeTelemetryMeta(SampleRecord *sampleRecord){
	putsCell("{\"meta\":{");

	int fieldCount = 0;
	putsCell("\"channels\":[");
	for (int i = 0; i < SAMPLE_RECORD_CHANNELS; i++){
		ChannelSample * sample = &(sampleRecord->Samples[i]);
		ChannelConfig * channelConfig = sample->channelConfig;
		if (SAMPLE_DISABLED == channelConfig->sampleRate) continue;
		if (fieldCount++ > 0) putcModem(',');
		putsCell("{\"name\":");
		putQuotedStringCell(channelConfig->label);
		if (strlen(channelConfig->units) > 0){
			putsCell(",\"units\":");
			putQuotedStringCell(channelConfig->units);
		}
		putcModem('}');
	}
	putsCell("]}");
	putsCell("}\n");
}

static void writeSampleRecordJSON(SampleRecord * sampleRecord, uint32_t sampleTick){

	//check for closed connection

	putsCell("{\"data\":{");
	int fieldCount = 0;
	putsCell("\"tick\":");
	putUintCell(sampleTick);
	putcModem(',');
	for (int i = 0; i < SAMPLE_RECORD_CHANNELS; i++){
		ChannelSample * sample = &(sampleRecord->Samples[i]);
		ChannelConfig * channelConfig = sample->channelConfig;

		if (SAMPLE_DISABLED == channelConfig->sampleRate) continue;

		if (sample->intValue == NIL_SAMPLE) continue;

		if (fieldCount++ > 0) putcModem(',');

		putQuotedStringCell(channelConfig->label);
		putcModem(':');
		int precision = sample->precision;
		if (precision > 0){
			putFloatCell(sample->floatValue,precision);
		}
		else{
			putIntCell(sample->intValue);
		}
	}
	putsCell("}}\n");
}

void cellTelemetryTask(void *params){

	initUsart0(USART_MODE_8N1, 115200);

	xQueueHandle sampleRecordQueue = (xQueueHandle)params;
	SampleRecord *sr = NULL;
	uint32_t sampleTick = 0;

	LoggerConfig *loggerConfig = getWorkingLoggerConfig();

	while(1){
		g_telemetryActive = 0;
		int cellReady = 0;

		while (cellReady == 0){
			if (0 == initCellModem()){
				if (0 == configureNet()){
					cellReady = 1;
				}
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
					if( 0 == connectNet(loggerConfig->LoggerOutputConfig.telemetryServerHost,"8080",0) &&
						0 == writeAuthJSON(loggerConfig->LoggerOutputConfig.telemetryDeviceId)){
							g_telemetryActive = 1;
							writeTelemetryMeta(sr);
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
							closeNet();
						}
					}
				}
			}
		}
	}
}

