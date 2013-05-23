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
#include "usb_comm.h"
#include "baseCommands.h"
#include "constants.h"
#include "interrupt_utils.h"
#include "USB-CDC.h"

//RaceCapture specific includes
#include "loggerHardware.h"
#include "gpioTasks.h"
#include "usart.h"
#include "sdcard.h"
#include "gps.h"
#include "loggerConfig.h"
#include "loggerCommands.h"
#include "sdcard.h"

//logging related tasks
#include "loggerTaskEx.h"
#include "fileWriter.h"
#include "telemetryTask.h"

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

	//* Set MCK at 48 054 850
	// 1 Enabling the Main Oscillator:
	// SCK = 1/32768 = 30.51 uSecond
	// Start up time = 8 * 6 / SCK = 56 * 30.51 = 1,46484375 ms
	AT91C_BASE_PMC->PMC_MOR = (( AT91C_CKGR_OSCOUNT & (0x06 <<8) | AT91C_CKGR_MOSCEN ));
	// Wait the startup time
	while(!(AT91C_BASE_PMC->PMC_SR & AT91C_PMC_MOSCS));
	// 2 Checking the Main Oscillator Frequency (Optional)
	// 3 Setting PLL and divider:
	// - div by 14 Fin = 1.3165 =(18,432 / 14)
	// - Mul 72+1: Fout = 96.1097 =(3,6864 *73)
	// for 96 MHz the error is 0.11%
	// Field out NOT USED = 0
	// PLLCOUNT pll startup time estimate at : 0.844 ms
	// PLLCOUNT 28 = 0.000844 /(1/32768)
	AT91C_BASE_PMC->PMC_PLLR = ((AT91C_CKGR_DIV & 14 ) | (AT91C_CKGR_PLLCOUNT & (28<<8)) | (AT91C_CKGR_MUL & (72<<16)));

	// Wait the startup time
	while(!(AT91C_BASE_PMC->PMC_SR & AT91C_PMC_LOCK));
	while(!(AT91C_BASE_PMC->PMC_SR & AT91C_PMC_MCKRDY));
	// 4. Selection of Master Clock and Processor Clock
	// select the PLL clock divided by 2
	AT91C_BASE_PMC->PMC_MCKR = AT91C_PMC_PRES_CLK_2 ;
	while(!(AT91C_BASE_PMC->PMC_SR & AT91C_PMC_MCKRDY));
	AT91C_BASE_PMC->PMC_MCKR |= AT91C_PMC_CSS_PLL_CLK ;
	while(!(AT91C_BASE_PMC->PMC_SR & AT91C_PMC_MCKRDY));

	enableLED(LED2);

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
	InitLEDs();
	enableLED(LED1);

	//setup hardware
	updateActiveLoggerConfig();
	int success = setupHardware();
	if (! success) fatalError(FATAL_ERROR_HARDWARE);

	xTaskCreate( vUSBCDCTask,		( signed portCHAR * ) "USB", 				mainUSB_TASK_STACK, NULL, 	mainUSB_PRIORITY, 		NULL );
	xTaskCreate( onUSBCommTask,	( signed portCHAR * ) "OnUSBComm", 				mainUSB_COMM_STACK, NULL, 	USB_COMM_TASK_PRIORITY, NULL );

#ifdef LUA_ENABLED
	startLuaTask();
#endif
	createFileWriterTask();
	//createLoggerTask();
	createLoggerTaskEx();
	createGPIOTasks();
	createTelemetryTask();
	startGPSTask();
//	startRaceTask();

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
