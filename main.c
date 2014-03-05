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
#include "serial.h"
#include "magic.h"
#include "usb_comm.h"
#include "baseCommands.h"
#include "constants.h"
#include "USB-CDC.h"
#include "cpu.h"
#include "OBD2_task.h"
#include "watchdog.h"
#include "LED.h"
#include "loggerHardware.h"
#include "gpsTask.h"
#include "usart.h"
#include "gpioTasks.h"
#include "messaging.h"

//logging related tasks
#include "loggerTaskEx.h"
#include "fileWriter.h"
#include "connectivityTask.h"
#include "luaTask.h"
#include "luaCommands.h"

#define FATAL_ERROR_SCHEDULER	1
#define FATAL_ERROR_HARDWARE	2

static void fatalError(int type){
	int count;
	int pause = 5000000;
	int flash = 1000000;

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
			for (int i=0;i<flash;i++){}
			LED_disable(1);
			LED_disable(2);
			for (int i=0;i<flash;i++){}
		}
		for (int i=0;i<pause;i++){}
	}
}

#define OBD2_TASK_PRIORITY					( tskIDLE_PRIORITY + 2 )
#define GPS_TASK_PRIORITY 					( tskIDLE_PRIORITY + 2 )
#define CONNECTIVITY_TASK_PRIORITY 			( tskIDLE_PRIORITY + 4 )
#define LOGGER_TASK_PRIORITY				( tskIDLE_PRIORITY + 4 )
#define FILE_WRITER_TASK_PRIORITY			( tskIDLE_PRIORITY + 3 )
#define LUA_TASK_PRIORITY					( tskIDLE_PRIORITY + 2 )
#define USB_COMM_TASK_PRIORITY				( tskIDLE_PRIORITY + 2 )
#define USB_CDC_TASK_PRIORITY				( tskIDLE_PRIORITY + 2 )
#define GPIO_TASK_PRIORITY 					( tskIDLE_PRIORITY + 4 )


int main( void )
{
	cpu_init();
	watchdog_init(WATCHDOG_TIMEOUT_MS);
	initialize_magic_info();
	initialize_logger_config();
	if (!initUsart()) fatalError(FATAL_ERROR_HARDWARE);
	if (!vInitUSBInterface()) fatalError(FATAL_ERROR_HARDWARE);
	init_serial();
	InitLoggerHardware();
	initMessaging();

	startGPIOTasks			( GPIO_TASK_PRIORITY );
	startUSBCDCTask			( USB_CDC_TASK_PRIORITY );
	startUSBCommTask		( USB_COMM_TASK_PRIORITY );
	startLuaTask			( LUA_TASK_PRIORITY );
	startFileWriterTask		( FILE_WRITER_TASK_PRIORITY );
	startLoggerTaskEx		( LOGGER_TASK_PRIORITY );
	startConnectivityTask	( CONNECTIVITY_TASK_PRIORITY );
	startGPSTask			( GPS_TASK_PRIORITY );
	startOBD2Task			( OBD2_TASK_PRIORITY);

	/* Start the scheduler.

   NOTE : Tasks run in system mode and the scheduler runs in Supervisor mode.
   The processor MUST be in supervisor mode when vTaskStartScheduler is
   called.  The demo applications included in the FreeRTOS.org download switch
   to supervisor mode prior to main being called.  If you are not using one of
   these demo application projects then ensure Supervisor mode is used here. */
   vTaskStartScheduler();
   fatalError(FATAL_ERROR_SCHEDULER);

   return 0;
}
