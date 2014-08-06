/*
 * RaceCapture Pro main
	NOTE : Tasks run in system mode and the scheduler runs in Supervisor mode.
	The processor MUST be in supervisor mode when vTaskStartScheduler is
	called.  The demo applications included in the FreeRTOS.org download switch
	to supervisor mode prior to main being called.  If you are not using one of
	these demo application projects then ensure Supervisor mode is used.
*/

// Standard includes
#include "FreeRTOS.h"
#include "task.h"
#include "heap.h"
#include "serial.h"
#include "usb_comm.h"
#include "baseCommands.h"
#include "constants.h"
#include "cpu.h"
#include "OBD2_task.h"
#include "watchdog.h"
#include "LED.h"
#include "loggerHardware.h"
#include "gpsTask.h"
#include "usart.h"
#include "gpioTasks.h"
#include "messaging.h"

#include "loggerConfig.h"
#include "channelMeta.h"
#include "tracks.h"
#include "luaScript.h"

//logging related tasks
#include "loggerTaskEx.h"
#include "fileWriter.h"
#include "connectivityTask.h"
#include "luaTask.h"
#include "luaCommands.h"
#include "sdcard.h"
#include "serial.h"


#define FATAL_ERROR_SCHEDULER	1
#define FATAL_ERROR_HARDWARE	2

#define FLASH_PAUSE_DELAY 	5000000
#define FLASH_DELAY 		1000000

static void delayStart(int count){
	while(count-- > 0){
		for (int c = 0; c < count; c++){
			LED_enable(0);
			LED_enable(1);
			LED_disable(2);
			LED_disable(3);
			for (int i=0; i<FLASH_DELAY; i++){}
			LED_disable(0);
			LED_disable(1);
			LED_enable(2);
			LED_enable(3);
			for (int i=0; i < FLASH_DELAY; i++){}
		}
		for (int i=0; i < FLASH_PAUSE_DELAY; i++){}
	}
	LED_disable(0);
	LED_disable(1);
	LED_disable(2);
	LED_disable(3);
}

static void fatalError(int type){
	int count;

	switch (type){
		case FATAL_ERROR_HARDWARE:
			count = 1;
			break;
		case FATAL_ERROR_SCHEDULER:
			count = 2;
			break;
		default:
			count = 3;
			break;
	}

	while(1){
		for (int c = 0; c < count; c++){
			LED_enable(1);
			LED_enable(2);
			for (int i=0; i<FLASH_DELAY; i++){}
			LED_disable(1);
			LED_disable(2);
			for (int i=0; i < FLASH_DELAY; i++){}
		}
		for (int i=0; i < FLASH_PAUSE_DELAY; i++){}
	}
}

#define OBD2_TASK_PRIORITY					( tskIDLE_PRIORITY + 2 )
#define GPS_TASK_PRIORITY 					( tskIDLE_PRIORITY + 2 )
#define CONNECTIVITY_TASK_PRIORITY 			( tskIDLE_PRIORITY + 4 )
#define LOGGER_TASK_PRIORITY				( tskIDLE_PRIORITY + 4 )
#define FILE_WRITER_TASK_PRIORITY			( tskIDLE_PRIORITY + 3 )
#define LUA_TASK_PRIORITY					( tskIDLE_PRIORITY + 2 )
#define USB_COMM_TASK_PRIORITY				( tskIDLE_PRIORITY + 2 )
#define GPIO_TASK_PRIORITY 					( tskIDLE_PRIORITY + 4 )


void vApplicationStackOverflowHook(xTaskHandle pxTask, signed char *pcTaskName)
{
	(void)pxTask;
	(void)pcTaskName;
}


int main( void )
{
//	watchdog_init(WATCHDOG_TIMEOUT_MS);
	//perform a clean reset if the watchdog fired
//	if (watchdog_is_watchdog_reset()) cpu_reset();
//	initialize_tracks();
//	initialize_channels();
//	initialize_logger_config();
//	initialize_script();
	InitLoggerHardware();
	cpu_init();
//	initMessaging();

//	startGPIOTasks			( GPIO_TASK_PRIORITY );
//	startUSBCommTask		( USB_COMM_TASK_PRIORITY );
//	startLuaTask			( LUA_TASK_PRIORITY );
//	startFileWriterTask		( FILE_WRITER_TASK_PRIORITY );
	startLoggerTaskEx		( LOGGER_TASK_PRIORITY );
//	startConnectivityTask	( CONNECTIVITY_TASK_PRIORITY );
//	startGPSTask			( GPS_TASK_PRIORITY );
//	startOBD2Task			( OBD2_TASK_PRIORITY);


	/* Start the scheduler.

   NOTE : Tasks run in system mode and the scheduler runs in Supervisor mode.
   The processor MUST be in supervisor mode when vTaskStartScheduler is
   called.  The demo applications included in the FreeRTOS.org download switch
   to supervisor mode prior to main being called.  If you are not using one of
   these demo application projects then ensure Supervisor mode is used here.
   */

   //this is to let the zylin debugger catch up and halt the processor.
   //when we figure how to halt it correctly we'll remove this.
   delayStart(4);

   TestSDWrite(NULL, 100000, 0, 1);
   vTaskStartScheduler();
   fatalError(FATAL_ERROR_SCHEDULER);
   return 0;
}
