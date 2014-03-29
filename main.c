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
#include "interrupt_utils.h"
#include "USB-CDC.h"

//RaceCapture specific includes
#include "loggerHardware.h"
#include "loggerData.h"
#include "gpioTasks.h"
#include "gpsTask.h"
#include "usart.h"
#include "sdcard.h"
#include "loggerConfig.h"
#include "loggerCommands.h"
#include "sdcard.h"
#include <tasks/heartbeat.h>
#include "messaging.h"

//logging related tasks
#include "loggerTaskEx.h"
#include "fileWriter.h"
#include "connectivityTask.h"

#ifdef LUA_ENABLED
#include "luaTask.h"
#include "luaCommands.h"
#endif
/*-----------------*/
/* Clock Selection */
/*-----------------*/
#define TC_CLKS                  0x7
#define TC_CLKS_MCK2             0x0
#define TC_CLKS_MCK8             0x1
#define TC_CLKS_MCK32            0x2
#define TC_CLKS_MCK128           0x3
#define TC_CLKS_MCK1024          0x4

/* Priorities for the demo application tasks. */
#define USB_COMM_TASK_PRIORITY				( tskIDLE_PRIORITY + 2 )
#define mainUSB_PRIORITY					( tskIDLE_PRIORITY + 2 )

#define mainUSB_TASK_STACK					( 100 )
#define mainUSB_COMM_STACK					( 1000 )

#define mainNO_ERROR_FLASH_PERIOD			( ( portTickType ) 1000 / portTICK_RATE_MS  )
#define mainBUSY_FLASH_PERIOD				( ( portTickType ) 500 / portTICK_RATE_MS )
#define mainERROR_FLASH_PERIOD				( ( portTickType ) 100 / portTICK_RATE_MS  )
#define mainDATA_DEBUG_PERIOD				( ( portTickType ) 100 / portTICK_RATE_MS  )

#define FATAL_ERROR_SCHEDULER	1
#define FATAL_ERROR_HARDWARE	2

void fatalError(int type);

static int setupHardware( void )
{
	// When using the JTAG debugger the hardware is not always initialised to
	// the correct default state.  This line just ensures that this does not
	// cause all interrupts to be masked at the start.
	AT91C_BASE_AIC->AIC_EOICR = 0;

	// Enable the peripheral clock.
	AT91F_PMC_EnablePeriphClock( AT91C_BASE_PMC, (1 << AT91C_ID_PIOA) |  //Enable Clock for PIO
												(1 << AT91C_ID_IRQ0) |  //Enable Clock for IRQ0
												(1 << AT91C_ID_PWMC) |	//Enable Clock for the PWM controller
												(1 << AT91C_ID_US0)  |  //USART0
												(1 << AT91C_ID_US1)		//USART1
							  );


	// Enable reset-button
	AT91F_RSTSetMode( AT91C_BASE_RSTC , AT91C_RSTC_URSTEN );

	if (!initUsart()) return 0;
	if (!vInitUSBInterface()) return 0;

	init_serial();

	InitLoggerHardware();
	return 1;
 }


void fatalError(int type){
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
			enableLED(LED1);
			enableLED(LED2);
			for (int i=0;i<flash;i++){}
			disableLED(LED1);
			disableLED(LED2);
			for (int i=0;i<flash;i++){}
		}
		for (int i=0;i<pause;i++){}
	}
}

int main( void )
{
	initMemoryPool();
	//setup hardware
	updateActiveLoggerConfig();
	int success = setupHardware();
	if (! success) fatalError(FATAL_ERROR_HARDWARE);
	init_logger_data();
	initMessaging();

	xTaskCreate( vUSBCDCTask,		( signed portCHAR * ) "USB", 				mainUSB_TASK_STACK, NULL, 	mainUSB_PRIORITY, 		NULL );
	xTaskCreate( onUSBCommTask,	( signed portCHAR * ) "OnUSBComm", 				mainUSB_COMM_STACK, NULL, 	USB_COMM_TASK_PRIORITY, NULL );

#ifdef LUA_ENABLED
	startLuaTask();
#endif
	createFileWriterTask();
	createLoggerTaskEx();
	createGPIOTasks();
	createConnectivityTask();
	startGPSTask();

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
