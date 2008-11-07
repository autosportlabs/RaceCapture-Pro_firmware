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
#include "led_debug.h"
#include "USB-CDC.h"
#include "loggerHardware.h"
#include "usart.h"
#include "sdcard.h"


#define SW1_MASK        (1<<19)	// PA19		RK   FIQ     13
#define FIQ_INTERRUPT_LEVEL	0

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
#define mainLED_TASK_PRIORITY 				( tskIDLE_PRIORITY + 1)
#define USB_COMM_TASK_PRIORITY				( tskIDLE_PRIORITY + 2 )
#define mainUSB_PRIORITY					( tskIDLE_PRIORITY + 2 )
#define mainDEFAULT_TASK_PRIORITY 			( tskIDLE_PRIORITY  )

#define mainUSB_TASK_STACK					( 300 )
#define mainUSB_COMM_STACK					( 700 )
#define mainON_REVOLUTION_STACK				( 500 )
#define mainON_SELF_TEST_STACK				( 300 )

#define mainNO_ERROR_FLASH_PERIOD			( ( portTickType ) 1000 / portTICK_RATE_MS  )
#define mainBUSY_FLASH_PERIOD				( ( portTickType ) 500 / portTICK_RATE_MS )
#define mainERROR_FLASH_PERIOD				( ( portTickType ) 100 / portTICK_RATE_MS  )
#define mainDATA_DEBUG_PERIOD				( ( portTickType ) 100 / portTICK_RATE_MS  )


static void StatusLED1( void *pvParameters );
static void StatusLED2( void *pvParameters );
static void SerialPing1( void *pvParameters );
static void SerialPing2( void *pvParameters );

/*
 * Checks that all the demo application tasks are still executing without error
 * - as described at the top of the file.
 */
static portLONG prvCheckOtherTasksAreStillRunning( void );

static void prvSetupHardware( void )
{
	/* When using the JTAG debugger the hardware is not always initialised to
	the correct default state.  This line just ensures that this does not
	cause all interrupts to be masked at the start. */
	AT91C_BASE_AIC->AIC_EOICR = 0;
		
	/* Enable the peripheral clock. */
   AT91F_PMC_EnablePeriphClock( AT91C_BASE_PMC, (1 << AT91C_ID_PIOA) |  /* Enable Clock for PIO    */
                                                (1 << AT91C_ID_IRQ0) |  /* Enable Clock for IRQ0   */
                                                (1 << AT91C_ID_US0)  |  /* Enable Clock for USART0 */
                                                (1 << AT91C_ID_PWMC) |	/* Enable Clock for the PWM controller */
                                                (1 << AT91C_ID_US0)  | 
                                                (1 << AT91C_ID_US1)
                              );

   /* Enable reset-button */
   AT91F_RSTSetMode( AT91C_BASE_RSTC , AT91C_RSTC_URSTEN );
   
   InitADC();
   InitPWM();
  
 }

/*-----------------------------------------------------------*/

/*
 * Starts all the other tasks, then starts the scheduler. 
 */
 

int main( void )
{
	
	//Setup our global variables

	/* Setup any hardware that has not already been configured by the low
	level init routines. */
	prvSetupHardware();
	Init_LEDs();
	
	// Start the task that processes the RPM signal


	// Start the check task - which is defined in this file.
	
	xTaskCreate( vUSBCDCTask,	( signed portCHAR * ) "USB", 			mainUSB_TASK_STACK, 		NULL, 	mainUSB_PRIORITY, NULL );
	xTaskCreate( onUSBCommTask,	( signed portCHAR * ) "OnUSBComm", 		mainUSB_COMM_STACK, 		NULL, 	tskIDLE_PRIORITY + 1, NULL );
	xTaskCreate( StatusLED1,	( signed portCHAR * ) "StatusLED1", configMINIMAL_STACK_SIZE, 	NULL, 	mainLED_TASK_PRIORITY, NULL );
	xTaskCreate( StatusLED2,	( signed portCHAR * ) "StatusLED2", configMINIMAL_STACK_SIZE, 	NULL, 	mainLED_TASK_PRIORITY, NULL );
	xTaskCreate( SerialPing1,	( signed portCHAR * ) "DebugSerial1", configMINIMAL_STACK_SIZE, 	NULL, 	mainDEFAULT_TASK_PRIORITY, NULL );
	xTaskCreate( SerialPing2,	( signed portCHAR * ) "DebugSerial2", configMINIMAL_STACK_SIZE, 	NULL, 	mainDEFAULT_TASK_PRIORITY, NULL );
	
				
#define mainCHECK_TASK_PRIORITY 			( tskIDLE_PRIORITY + 1 )

// DJS--iprintf() requires syscalls.c and serial_simple.c to be compiled
//   iprintf("\r\nFreeRTOS %s\r\n\r\n",tskKERNEL_VERSION_NUMBER);

   /* Start the scheduler.

   NOTE : Tasks run in system mode and the scheduler runs in Supervisor mode.
   The processor MUST be in supervisor mode when vTaskStartScheduler is 
   called.  The demo applications included in the FreeRTOS.org download switch
   to supervisor mode prior to main being called.  If you are not using one of
   these demo application projects then ensure Supervisor mode is used here. */
   vTaskStartScheduler();

	while(1){
		Toggle_LED(LED2);
		for (int i=0;i<2000000;i++){}	
	}

   /* We should never get here as control is now taken by the scheduler. */
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
		
		Toggle_LED(LED1);
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
		Toggle_LED(LED2);
	}
}

static void SerialPing1( void *pvParameters )
{
	portTickType xDelayPeriod = mainDATA_DEBUG_PERIOD;
	
	for( ;; )
	{
		uart0_puts("Test_UART0\r\n");
		/* Delay until it is time to execute again. */
		vTaskDelay( xDelayPeriod );
	}
	
}

static void SerialPing2( void *pvParameters )
{
	portTickType xDelayPeriod = mainDATA_DEBUG_PERIOD;
	
	for( ;; )
	{
		uart1_puts("Test_UART1\r\n");
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
