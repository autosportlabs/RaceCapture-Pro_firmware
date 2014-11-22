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

#include "capabilities.h"
#include "constants.h"
#include "LED.h"
#include "cpu.h"
#include "watchdog.h"
#include "loggerHardware.h"
#include "messaging.h"
#include "luaScript.h"

//tasks
#include "loggerTaskEx.h"
#include "fileWriter.h"
#include "connectivityTask.h"
#include "luaTask.h"
#include "OBD2_task.h"
#include "gpsTask.h"
#include "gpioTasks.h"
#include "usb_comm.h"

#include <app_info.h>

#define FATAL_ERROR_SCHEDULER	1
#define FATAL_ERROR_HARDWARE	2

#define FLASH_PAUSE_DELAY 	5000000
#define FLASH_DELAY 		1000000

__attribute__((aligned (4)))
static const struct app_info_block info_block = {
	.magic_number = APP_INFO_MAGIC_NUMBER,
	.info_crc = 0xDEADBEEF,
};

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

#define OBD2_TASK_PRIORITY			( tskIDLE_PRIORITY + 2 )
#define GPS_TASK_PRIORITY 			( tskIDLE_PRIORITY + 5 )
#define CONNECTIVITY_TASK_PRIORITY 	( tskIDLE_PRIORITY + 4 )
#define LOGGER_TASK_PRIORITY		( tskIDLE_PRIORITY + 6 )
#define FILE_WRITER_TASK_PRIORITY	( tskIDLE_PRIORITY + 4 )
#define LUA_TASK_PRIORITY			( tskIDLE_PRIORITY + 2 )
#define USB_COMM_TASK_PRIORITY		( tskIDLE_PRIORITY + 2 )
#define GPIO_TASK_PRIORITY 			( tskIDLE_PRIORITY + 4 )


void vApplicationStackOverflowHook(xTaskHandle pxTask, signed char *pcTaskName)
{
	(void)pxTask;
	(void)pcTaskName;
}

void setupTask(void *params)
{
	(void)params;

	initialize_tracks();
	initialize_logger_config();
	initialize_script();
	InitLoggerHardware();
	initMessaging();

	startGPIOTasks			( GPIO_TASK_PRIORITY );
	startUSBCommTask		( USB_COMM_TASK_PRIORITY );
	startLuaTask			( LUA_TASK_PRIORITY );
	startFileWriterTask		( FILE_WRITER_TASK_PRIORITY );
	startConnectivityTask	( CONNECTIVITY_TASK_PRIORITY );
	startGPSTask			( GPS_TASK_PRIORITY );
	startOBD2Task			( OBD2_TASK_PRIORITY);
	startLoggerTaskEx		( LOGGER_TASK_PRIORITY );

	/* Removes this setup task from the scheduler */
	vTaskDelete(NULL);
}


int main( void )
{
	ALWAYS_KEEP(info_block);
	cpu_init();
	watchdog_init(WATCHDOG_TIMEOUT_MS);

	//perform a clean reset if the watchdog fired
	if (watchdog_is_watchdog_reset())
		cpu_reset(0);

	/* Start the scheduler.

	   NOTE : Tasks run in system mode and the scheduler runs in Supervisor mode.
	   The processor MUST be in supervisor mode when vTaskStartScheduler is
	   called.  The demo applications included in the FreeRTOS.org download switch
	   to supervisor mode prior to main being called.  If you are not using one of
	   these demo application projects then ensure Supervisor mode is used here.
	*/

	if (TASK_TASK_INIT){
		xTaskCreate(setupTask,
				(signed portCHAR*)"Hardware Init",
				configMINIMAL_STACK_SIZE + 500,
				NULL,
				tskIDLE_PRIORITY + 4,
				NULL);
	}
	else{
		setupTask(NULL);
	}

	vTaskStartScheduler();
	fatalError(FATAL_ERROR_SCHEDULER);

	return 0;
}
