#include "cellTelemetry.h"
#include "cellModem.h"
#include "usb_comm.h"
#include "task.h"
#include "modp_numtoa.h"
#include "loggerHardware.h"

static int g_telemetryActive;
#define IDLE_TIMEOUT							configTICK_RATE_HZ / 1


static void writeSampleRecordJSON(SampleRecord * sampleRecord, uint32_t sampleTick){

	//check for closed connection

	putcCell('{');
	int fieldCount = 0;
	putsCell("\"tick\":");
	putUintCell(sampleTick);
	putcCell(',');
	for (int i = 0; i < SAMPLE_RECORD_CHANNELS; i++){
		ChannelSample * sample = &(sampleRecord->Samples[i]);
		ChannelConfig * channelConfig = sample->channelConfig;

		if (SAMPLE_DISABLED == channelConfig->sampleRate) continue;

		if (sample->intValue == NIL_SAMPLE) continue;

		if (fieldCount++ > 0) putcCell(',');

		putQuotedStringCell(channelConfig->label);
		putcCell(':');
		int precision = sample->precision;
		if (precision > 0){
			putFloatCell(sample->floatValue,precision);
		}
		else{
			putIntCell(sample->intValue);
		}
	}
	putsCell("}\r\n");
}

void cellTelemetryTask(void *params){

	xQueueHandle sampleRecordQueue = (xQueueHandle)params;
	SampleRecord *sr = NULL;
	uint32_t sampleTick = 0;

	while(1){
		g_telemetryActive = 0;
		int cellReady = 0;

		EnableLED(LED3);
		while (cellReady == 0){
			if (0 == initCellModem()){
				if (0 == configureNet()){
					cellReady = 1;
					DisableLED(LED3);
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
				if (0 == g_telemetryActive && cellReady){
					if( 0 == connectNet("67.222.3.214","8080",0)){
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
							ToggleLED(LED3);
							writeSampleRecordJSON(sr,sampleTick);
						}
						else{
							DisableLED(LED3);
							g_telemetryActive = 0;
							closeNet();
						}
					}
				}
			}
		}
	}
}

