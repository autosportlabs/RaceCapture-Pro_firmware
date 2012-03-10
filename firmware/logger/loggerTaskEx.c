/*
 * loggerTaskEx.c
 *
 *  Created on: Mar 3, 2012
 *      Author: brent
 */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "fileWriter.h"
#include "telemetryTask.h"
#include "sampleRecord.h"
#include "loggerTaskEx.h"
#include "loggerHardware.h"
#include "accelerometer.h"
#include "luaLoggerBinding.h"
#include "gps.h"
#include "sdcard.h"
#include "usb_comm.h"

#define LOGGER_TASK_PRIORITY				( tskIDLE_PRIORITY + 4 )
#define LOGGER_STACK_SIZE  					200

int g_loggingShouldRun;

xSemaphoreHandle g_xLoggerStart;

void createLoggerTaskEx(){

	g_loggingShouldRun = 0;

	registerLuaLoggerBindings();

	vSemaphoreCreateBinary( g_xLoggerStart );
	xSemaphoreTake( g_xLoggerStart, 1 );
	xTaskCreate( loggerTaskEx,( signed portCHAR * ) "loggerEx",	LOGGER_STACK_SIZE, NULL, LOGGER_TASK_PRIORITY, NULL );
}

static void writeAccelerometer(SampleRecord *sampleRecord, portTickType currentTicks, LoggerConfig *config){

	unsigned int accelValues[CONFIG_ACCEL_CHANNELS];

	for (unsigned int i=0; i < CONFIG_ACCEL_CHANNELS;i++){
		AccelConfig *ac = &(config->AccelConfigs[i]);
		portTickType sr = ac->cfg.sampleRate;
		if (sr != SAMPLE_DISABLED && (currentTicks % sr) == 0){
			accelValues[i] = readAccelChannel(ac->accelChannel);
		}
	}

	for (unsigned int i=0; i < CONFIG_ACCEL_CHANNELS;i++){
		AccelConfig *ac = &(config->AccelConfigs[i]);
		portTickType sr = ac->cfg.sampleRate;
		if (sr != SAMPLE_DISABLED){
			if ((currentTicks % sr) == 0){
				sampleRecord->AccelSamples[i].floatValue = convertAccelRawToG(accelValues[i],ac->zeroValue);
			}
		}
	}
}

static void writeADC(SampleRecord *sampleRecord, portTickType currentTicks, LoggerConfig *config){

	unsigned int adc[CONFIG_ADC_CHANNELS];
	ReadAllADC(&adc[0],&adc[1],&adc[2],&adc[3],&adc[4],&adc[5],&adc[6],&adc[7]);

	for (unsigned int i=0; i < CONFIG_ADC_CHANNELS;i++){
		ADCConfig *ac = &(config->ADCConfigs[i]);
		portTickType sr = ac->cfg.sampleRate;
		if (sr != SAMPLE_DISABLED){
			if ((currentTicks % sr) == 0){
				sampleRecord->ADCSamples[i].floatValue = (ac->scaling * (float)adc[i]);
			}
		}
	}
}

static void writeGPSChannels(SampleRecord *sampleRecord, portTickType currentTicks, GPSConfig *config){

	{
		portTickType sr = config->timeCfg.sampleRate;
		if (sr != SAMPLE_DISABLED){
			if ((currentTicks % sr) == 0) sampleRecord->GPS_TimeSample.floatValue = getUTCTime();
		}
	}
	{
		//latitude sample rate is a stand-in for position sample rate
		portTickType sr = config->latitudeCfg.sampleRate;
		if (sr != SAMPLE_DISABLED){
			if ((currentTicks % sr) == 0){
				sampleRecord->GPS_LatitueSample.floatValue = getLatitude();
				sampleRecord->GPS_LongitudeSample.floatValue = getLongitude();
			}
		}
	}

	{
		portTickType sr = config->qualityCfg.sampleRate;
		if (sr != SAMPLE_DISABLED){
			if ((currentTicks % sr) == 0) sampleRecord->GPS_QualitySample.intValue = getGPSQuality();
		}
	}

	{
		portTickType sr = config->satellitesCfg.sampleRate;
		if (sr != SAMPLE_DISABLED){
			if ((currentTicks % sr) == 0) sampleRecord->GPS_SatellitesSample.intValue = getSatellitesUsedForPosition();
		}
	}

	{
		portTickType sr = config->velocityCfg.sampleRate;
		if (sr != SAMPLE_DISABLED){
			if ((currentTicks % sr) == 0) sampleRecord->GPS_VelocitySample.floatValue = getGPSVelocity();
		}
	}
}

static void writeGPIOs(SampleRecord *sampleRecord, portTickType currentTicks, LoggerConfig *loggerConfig){

	unsigned int gpioMasks[CONFIG_GPIO_CHANNELS];

	gpioMasks[0] = GPIO_1;
	gpioMasks[1] = GPIO_2;
	gpioMasks[2] = GPIO_3;

	unsigned int gpioStates = AT91F_PIO_GetInput(AT91C_BASE_PIOA);
	for (int i = 0; i < CONFIG_GPIO_CHANNELS; i++){
		GPIOConfig *c = &(loggerConfig->GPIOConfigs[i]);
		portTickType sr = c->cfg.sampleRate;
		if (sr != SAMPLE_DISABLED){
			if ((currentTicks % sr) == 0) sampleRecord->GPIOSamples[i].intValue = ((gpioStates & gpioMasks[i]) != 0);
		}
	}
}

static void writeTimerChannels(SampleRecord *sampleRecord, portTickType currentTicks, LoggerConfig *loggerConfig){

	unsigned int timers[CONFIG_TIMER_CHANNELS];
	getAllTimerPeriods(timers,timers + 1,timers + 2);

	for (int i = 0; i < CONFIG_TIMER_CHANNELS; i++){
		TimerConfig *c = &(loggerConfig->TimerConfigs[i]);
		portTickType sr = c->cfg.sampleRate;
		if (sr != SAMPLE_DISABLED){
			if ((currentTicks % sr) == 0){
				int value = 0;
				int scaling = c->calculatedScaling;
				unsigned int timerValue = timers[i];
				switch (c->config){
					case CONFIG_LOGGING_TIMER_RPM:
						value = calculateRPM(timerValue,scaling);
						break;
					case CONFIG_LOGGING_TIMER_FREQUENCY:
						value = calculateFrequencyHz(timerValue,scaling);
						break;
					case CONFIG_LOGGING_TIMER_PERIOD_MS:
						value = calculatePeriodMs(timerValue,scaling);
						break;
					case CONFIG_LOGGING_TIMER_PERIOD_USEC:
						value = calculatePeriodUsec(timerValue,scaling);
						break;
					default:
						value = -1;
						break;
				}
				sampleRecord->TimerSamples[i].intValue = value;
			}
		}
	}
}

static void writePWMChannels(SampleRecord *sampleRecord, portTickType currentTicks, LoggerConfig *loggerConfig){

	for (int i = 0; i < CONFIG_PWM_CHANNELS; i++){
		PWMConfig *c = &(loggerConfig->PWMConfigs[i]);
		portTickType sr = c->cfg.sampleRate;
		if (sr != SAMPLE_DISABLED){
			if ((currentTicks % sr) == 0){
				switch (c->loggingConfig){
					case CONFIG_LOGGING_PWM_PERIOD:
						sampleRecord->PWMSamples[i].intValue = PWM_GetPeriod(i);
						break;
					case CONFIG_LOGGING_PWM_DUTY:
						sampleRecord->PWMSamples[i].intValue = PWM_GetDutyCycle(i);
						break;
					case CONFIG_LOGGING_PWM_VOLTS:
						sampleRecord->PWMSamples[i].floatValue =  PWM_GetDutyCycle(i) * c->voltageScaling;
						break;
					default:
						break;
				}
			}
		}
	}
}

#define SAMPLE_RECORD_BUFFER_SIZE 10

static SampleRecord g_sampleRecordBuffer[SAMPLE_RECORD_BUFFER_SIZE];

void loggerTaskEx(void *params){

	LoggerConfig *loggerConfig = getWorkingLoggerConfig();

	if ( loggerConfig->AccelInstalled == CONFIG_FEATURE_INSTALLED ) accel_setup();

	while(1){
		//wait for signal to start logging
		if ( xSemaphoreTake(g_xLoggerStart, portMAX_DELAY) != pdTRUE){
			//perform idle tasks
		}
		else {
			//perform logging tasks
			int gpsInstalled = (int)loggerConfig->GPSInstalled;
			int accelInstalled = (int)loggerConfig->AccelInstalled;

			const portTickType xFrequency = getHighestSampleRate(loggerConfig);

			g_loggingShouldRun = 1;

			portTickType currentTicks = 0;

			//SampleRecord **srBuffer = createSampleRecordBuffer(loggerConfig,SAMPLE_RECORD_BUFFER_SIZE );

			//the below gives a warning.. why?
			//initSampleRecordBuffer(loggerConfig,&g_sampleRecordBuffer,SAMPLE_RECORD_BUFFER_SIZE);
			//sub this for now
			for (int i=0; i < SAMPLE_RECORD_BUFFER_SIZE; i++) initSampleRecord(loggerConfig,&g_sampleRecordBuffer[i]);


			int bufferIndex = 0;

			//run until signalled to stop
			portTickType xLastWakeTime = xTaskGetTickCount();
			while (g_loggingShouldRun){
				ToggleLED(LED2);

				currentTicks += xFrequency;

				SampleRecord *sr = &g_sampleRecordBuffer[bufferIndex];//srBuffer[bufferIndex];

				clearSampleRecord(sr);

				//Write ADC channels
				writeADC(sr, currentTicks, loggerConfig);
				//Write GPIO channels
				writeGPIOs(sr,currentTicks, loggerConfig);
				//Write Timer channels
				writeTimerChannels(sr,currentTicks, loggerConfig);
				//Write PWM channels
				writePWMChannels(sr,currentTicks, loggerConfig);

				//Optional hardware
				//Write Accelerometer
				//if (accelInstalled) writeAccelerometer(sr,currentTicks, loggerConfig);
				//Write GPS
				if (gpsInstalled) writeGPSChannels(sr,currentTicks, &(loggerConfig->GPSConfig));

				//queueLogfileRecord(sr);
				queueTelemetryRecord(sr);

				bufferIndex++;
				if (bufferIndex >= SAMPLE_RECORD_BUFFER_SIZE ) bufferIndex = 0;

				vTaskDelayUntil( &xLastWakeTime, xFrequency );
			}
			queueLogfileRecord(NULL);
			//freeSampleRecordBuffer(srBuffer);
			DisableLED(LED2);
		}
	}


}

//test code for detecting power loss via +12v bus.
/*				if (ReadADC(7) < 230){
	g_loggingShouldRun = 0;

	fileWriteString(&f,"x\r\n");
	break;
}

*/
