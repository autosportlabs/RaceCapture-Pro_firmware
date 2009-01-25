/* 
	NOTE : Tasks run in system mode and the scheduler runs in Supervisor mode.
	The processor MUST be in supervisor mode when vTaskStartScheduler is 
	called.  The demo applications included in the FreeRTOS.org download switch
	to supervisor mode prior to main being called.  If you are not using one of
	these demo application projects then ensure Supervisor mode is used.
*/


/* Standard includes. */
#include <stdlib.h>
#include <stdio.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "usb_comm.h"
#include "interrupt_utils.h"
#include "USB-CDC.h"
#include "loggerHardware.h"
#include "gpioTasks.h"
#include "usart.h"
#include "sdcard.h"
#include "loggerTask.h"
#include "gps.h"


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
#define mainDEFAULT_TASK_PRIORITY 			( tskIDLE_PRIORITY + 1 )

#define mainUSB_TASK_STACK					( 100 )
#define mainUSB_COMM_STACK					( 600 )

#define mainNO_ERROR_FLASH_PERIOD			( ( portTickType ) 1000 / portTICK_RATE_MS  )
#define mainBUSY_FLASH_PERIOD				( ( portTickType ) 500 / portTICK_RATE_MS )
#define mainERROR_FLASH_PERIOD				( ( portTickType ) 100 / portTICK_RATE_MS  )
#define mainDATA_DEBUG_PERIOD				( ( portTickType ) 100 / portTICK_RATE_MS  )

#define FATAL_ERROR_SCHEDULER	1
#define FATAL_ERROR_HARDWARE	2

static void StatusLED1( void *pvParameters );
static void StatusLED2( void *pvParameters );
static void SerialPing1( void *pvParameters );
static void SerialPing2( void *pvParameters );

/*
 * Checks that all the demo application tasks are still executing without error
 * - as described at the top of the file.
 */
static portLONG prvCheckOtherTasksAreStillRunning( void );

static int setupHardware( void )
{
	/* When using the JTAG debugger the hardware is not always initialised to
	the correct default state.  This line just ensures that this does not
	cause all interrupts to be masked at the start. */
	AT91C_BASE_AIC->AIC_EOICR = 0;
		
	/* Enable the peripheral clock. */
   AT91F_PMC_EnablePeriphClock( AT91C_BASE_PMC, (1 << AT91C_ID_PIOA) |  //Enable Clock for PIO
                                                (1 << AT91C_ID_IRQ0) |  //Enable Clock for IRQ0
                                                (1 << AT91C_ID_PWMC) |	//Enable Clock for the PWM controller
                                                (1 << AT91C_ID_US0)  |  //USART0
                                                (1 << AT91C_ID_US1)		//USART1
                              );

   /* Enable reset-button */
   AT91F_RSTSetMode( AT91C_BASE_RSTC , AT91C_RSTC_URSTEN );

	if (!initSerial()) return 0;
	if (!vInitUSBInterface()) return 0;	
	   
	InitADC();
	//EnableAllPWM();
	InitLEDs();
	InitGPIO();
	return 1;
 }

/*-----------------------------------------------------------*/

/*
 * Starts all the other tasks, then starts the scheduler. 
 */

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
	}
	
	while(1){
		for (int c = 0; c < count; c++){
			EnableLED(LED1);
			EnableLED(LED2);
			for (int i=0;i<flash;i++){}
			DisableLED(LED1);
			DisableLED(LED2);
			for (int i=0;i<flash;i++){}
		}
		for (int i=0;i<pause;i++){}	
	}
}

int main( void )
{
	//setup hardware
	int success = setupHardware();
	if (! success) fatalError(FATAL_ERROR_HARDWARE);

	xTaskCreate( vUSBCDCTask,		( signed portCHAR * ) "USB", 				mainUSB_TASK_STACK, 		NULL, 	mainUSB_PRIORITY, 			NULL );
	xTaskCreate( onUSBCommTask,		( signed portCHAR * ) "OnUSBComm", 			mainUSB_COMM_STACK, 		NULL, 	mainDEFAULT_TASK_PRIORITY, 	NULL );
	createLoggerTask();
	createGPIOTasks();
	startGPSTask();
	//xTaskCreate( StatusLED1,		( signed portCHAR * ) "StatusLED1", 		configMINIMAL_STACK_SIZE, 	NULL, 	mainDEFAULT_TASK_PRIORITY, 	NULL );
	//xTaskCreate( StatusLED2,		( signed portCHAR * ) "StatusLED2", 		configMINIMAL_STACK_SIZE, 	NULL, 	mainDEFAULT_TASK_PRIORITY, 	NULL );
	//xTaskCreate( SerialPing1,		( signed portCHAR * ) "DebugSerial1", 		configMINIMAL_STACK_SIZE, 	NULL, 	mainDEFAULT_TASK_PRIORITY, 	NULL );
	//xTaskCreate( SerialPing2,		( signed portCHAR * ) "DebugSerial2", 		configMINIMAL_STACK_SIZE, 	NULL, 	mainDEFAULT_TASK_PRIORITY, 	NULL );

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


static void StatusLED1( void *pvParameters )
{

	portTickType xDelayPeriod = mainBUSY_FLASH_PERIOD;
	
	/* The parameters are not used in this task. */
	( void ) pvParameters;

	/* Cycle for ever, delaying then checking all the other tasks are still
	operating without error.  If an error is detected then the delay period
	is decreased from mainNO_ERROR_FLASH_PERIOD to mainERROR_FLASH_PERIOD so
	the on board LED flash rate will increase. */

	for( ;; )
	{
		/* Delay until it is time to execute again. */
		vTaskDelay( xDelayPeriod );
		/* Check all the standard demo application tasks are executing without 
		error. */
		if( prvCheckOtherTasksAreStillRunning() != pdPASS )
		{
			/* An error has been detected in one of the tasks - flash faster. */
			xDelayPeriod = mainERROR_FLASH_PERIOD;
		}
		
		ToggleLED(LED1);
	}
}

static void StatusLED2( void *pvParameters )
{

	portTickType xDelayPeriod = mainNO_ERROR_FLASH_PERIOD;
	
	/* The parameters are not used in this task. */
	( void ) pvParameters;

	/* Cycle for ever, delaying then checking all the other tasks are still
	operating without error.  If an error is detected then the delay period
	is decreased from mainNO_ERROR_FLASH_PERIOD to mainERROR_FLASH_PERIOD so
	the on board LED flash rate will increase. */

	for( ;; )
	{
		/* Delay until it is time to execute again. */
		vTaskDelay( xDelayPeriod );
		/* Check all the standard demo application tasks are executing without 
		error. */
		if( prvCheckOtherTasksAreStillRunning() != pdPASS )
		{
			/* An error has been detected in one of the tasks - flash faster. */
			xDelayPeriod = mainERROR_FLASH_PERIOD;
		}
		ToggleLED(LED2);
	}
}

static void SerialPing1( void *pvParameters )
{
	portTickType xDelayPeriod = mainDATA_DEBUG_PERIOD;
	
	for( ;; )
	{
		usart0_puts("Test_UART0\r\n");
		/* Delay until it is time to execute again. */
		vTaskDelay( xDelayPeriod );
	}
	
}

static void SerialPing2( void *pvParameters )
{
	portTickType xDelayPeriod = mainDATA_DEBUG_PERIOD;
	
	for( ;; )
	{
		usart1_puts("Test_UART1\r\n");
		/* Delay until it is time to execute again. */
		vTaskDelay( xDelayPeriod );
	}
	
}
/*-----------------------------------------------------------*/

static portLONG prvCheckOtherTasksAreStillRunning( void )
{
portLONG lReturn = ( portLONG ) pdPASS;

	/* Check all the demo tasks (other than the flash tasks) to ensure
	that they are all still running, and that none of them have detected
	an error. */

	return lReturn;
}
/*-----------------------------------------------------------*/
