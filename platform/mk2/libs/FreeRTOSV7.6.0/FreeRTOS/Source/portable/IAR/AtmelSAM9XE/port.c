/*
    FreeRTOS V7.6.0 - Copyright (C) 2013 Real Time Engineers Ltd.
    All rights reserved

    VISIT http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.

    ***************************************************************************
     *                                                                       *
     *    FreeRTOS provides completely free yet professionally developed,    *
     *    robust, strictly quality controlled, supported, and cross          *
     *    platform software that has become a de facto standard.             *
     *                                                                       *
     *    Help yourself get started quickly and support the FreeRTOS         *
     *    project by purchasing a FreeRTOS tutorial book, reference          *
     *    manual, or both from: http://www.FreeRTOS.org/Documentation        *
     *                                                                       *
     *    Thank you!                                                         *
     *                                                                       *
    ***************************************************************************

    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation >>!AND MODIFIED BY!<< the FreeRTOS exception.

    >>! NOTE: The modification to the GPL is included to allow you to distribute
    >>! a combined work that includes FreeRTOS without being obliged to provide
    >>! the source code for proprietary components outside of the FreeRTOS
    >>! kernel.

    FreeRTOS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.  Full license text is available from the following
    link: http://www.freertos.org/a00114.html

    1 tab == 4 spaces!

    ***************************************************************************
     *                                                                       *
     *    Having a problem?  Start by reading the FAQ "My application does   *
     *    not run, what could be wrong?"                                     *
     *                                                                       *
     *    http://www.FreeRTOS.org/FAQHelp.html                               *
     *                                                                       *
    ***************************************************************************

    http://www.FreeRTOS.org - Documentation, books, training, latest versions,
    license and Real Time Engineers Ltd. contact details.

    http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
    including FreeRTOS+Trace - an indispensable productivity tool, a DOS
    compatible FAT file system, and our tiny thread aware UDP/IP stack.

    http://www.OpenRTOS.com - Real Time Engineers ltd license FreeRTOS to High
    Integrity Systems to sell under the OpenRTOS brand.  Low cost OpenRTOS
    licenses offer ticketed support, indemnification and middleware.

    http://www.SafeRTOS.com - High Integrity Systems also provide a safety
    engineered and independently SIL3 certified version for use in safety and
    mission critical applications that require provable dependability.

    1 tab == 4 spaces!
*/

/*-----------------------------------------------------------
 * Implementation of functions defined in portable.h for the Atmel ARM7 port.
 *----------------------------------------------------------*/


/* Standard includes. */
#include <stdlib.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"

/* Hardware includes. */
#include <board.h>
#include <pio/pio.h>
#include <pio/pio_it.h>
#include <pit/pit.h>
#include <aic/aic.h>
#include <tc/tc.h>
#include <utility/led.h>
#include <utility/trace.h>

/*-----------------------------------------------------------*/

/* Constants required to setup the initial stack. */
#define portINITIAL_SPSR				( ( portSTACK_TYPE ) 0x1f ) /* System mode, ARM mode, interrupts enabled. */
#define portTHUMB_MODE_BIT				( ( portSTACK_TYPE ) 0x20 )
#define portINSTRUCTION_SIZE			( ( portSTACK_TYPE ) 4 )

/* Constants required to setup the PIT. */
#define port1MHz_IN_Hz 					( 1000000ul )
#define port1SECOND_IN_uS				( 1000000.0 )

/* Constants required to handle critical sections. */
#define portNO_CRITICAL_NESTING 		( ( unsigned long ) 0 )


#define portINT_LEVEL_SENSITIVE  0
#define portPIT_ENABLE      	( ( unsigned short ) 0x1 << 24 )
#define portPIT_INT_ENABLE     	( ( unsigned short ) 0x1 << 25 )
/*-----------------------------------------------------------*/

/* Setup the PIT to generate the tick interrupts. */
static void prvSetupTimerInterrupt( void );

/* The PIT interrupt handler - the RTOS tick. */
static void vPortTickISR( void );

/* ulCriticalNesting will get set to zero when the first task starts.  It
cannot be initialised to 0 as this will cause interrupts to be enabled
during the kernel initialisation process. */
unsigned long ulCriticalNesting = ( unsigned long ) 9999;

/*-----------------------------------------------------------*/

/*
 * Initialise the stack of a task to look exactly as if a call to
 * portSAVE_CONTEXT had been called.
 *
 * See header file for description.
 */
portSTACK_TYPE *pxPortInitialiseStack( portSTACK_TYPE *pxTopOfStack, pdTASK_CODE pxCode, void *pvParameters )
{
        portSTACK_TYPE *pxOriginalTOS;

        pxOriginalTOS = pxTopOfStack;

        /* To ensure asserts in tasks.c don't fail, although in this case the assert
        is not really required. */
        pxTopOfStack--;

        /* Setup the initial stack of the task.  The stack is set exactly as
        expected by the portRESTORE_CONTEXT() macro. */

        /* First on the stack is the return address - which in this case is the
        start of the task.  The offset is added to make the return address appear
        as it would within an IRQ ISR. */
        *pxTopOfStack = ( portSTACK_TYPE ) pxCode + portINSTRUCTION_SIZE;
        pxTopOfStack--;

        *pxTopOfStack = ( portSTACK_TYPE ) 0xaaaaaaaa;	/* R14 */
        pxTopOfStack--;
        *pxTopOfStack = ( portSTACK_TYPE ) pxOriginalTOS; /* Stack used when task starts goes in R13. */
        pxTopOfStack--;
        *pxTopOfStack = ( portSTACK_TYPE ) 0x12121212;	/* R12 */
        pxTopOfStack--;
        *pxTopOfStack = ( portSTACK_TYPE ) 0x11111111;	/* R11 */
        pxTopOfStack--;
        *pxTopOfStack = ( portSTACK_TYPE ) 0x10101010;	/* R10 */
        pxTopOfStack--;
        *pxTopOfStack = ( portSTACK_TYPE ) 0x09090909;	/* R9 */
        pxTopOfStack--;
        *pxTopOfStack = ( portSTACK_TYPE ) 0x08080808;	/* R8 */
        pxTopOfStack--;
        *pxTopOfStack = ( portSTACK_TYPE ) 0x07070707;	/* R7 */
        pxTopOfStack--;
        *pxTopOfStack = ( portSTACK_TYPE ) 0x06060606;	/* R6 */
        pxTopOfStack--;
        *pxTopOfStack = ( portSTACK_TYPE ) 0x05050505;	/* R5 */
        pxTopOfStack--;
        *pxTopOfStack = ( portSTACK_TYPE ) 0x04040404;	/* R4 */
        pxTopOfStack--;
        *pxTopOfStack = ( portSTACK_TYPE ) 0x03030303;	/* R3 */
        pxTopOfStack--;
        *pxTopOfStack = ( portSTACK_TYPE ) 0x02020202;	/* R2 */
        pxTopOfStack--;
        *pxTopOfStack = ( portSTACK_TYPE ) 0x01010101;	/* R1 */
        pxTopOfStack--;

        /* When the task starts is will expect to find the function parameter in
        R0. */
        *pxTopOfStack = ( portSTACK_TYPE ) pvParameters; /* R0 */
        pxTopOfStack--;

        /* The status register is set for system mode, with interrupts enabled. */
        *pxTopOfStack = ( portSTACK_TYPE ) portINITIAL_SPSR;

#ifdef THUMB_INTERWORK
        {
                /* We want the task to start in thumb mode. */
                *pxTopOfStack |= portTHUMB_MODE_BIT;
        }
#endif

        pxTopOfStack--;

        /* Interrupt flags cannot always be stored on the stack and will
        instead be stored in a variable, which is then saved as part of the
        tasks context. */
        *pxTopOfStack = portNO_CRITICAL_NESTING;

        return pxTopOfStack;
}
/*-----------------------------------------------------------*/

portBASE_TYPE xPortStartScheduler( void )
{
        extern void vPortStartFirstTask( void );

        /* Start the timer that generates the tick ISR.  Interrupts are disabled
        here already. */
        prvSetupTimerInterrupt();

        /* Start the first task. */
        vPortStartFirstTask();

        /* Should not get here! */
        return 0;
}
/*-----------------------------------------------------------*/

void vPortEndScheduler( void )
{
        /* It is unlikely that the ARM port will require this function as there
        is nothing to return to.  */
}
/*-----------------------------------------------------------*/

static __arm void vPortTickISR( void )
{
        volatile unsigned long ulDummy;

        /* Increment the tick count - which may wake some tasks but as the
        preemptive scheduler is not being used any woken task is not given
        processor time no matter what its priority. */
        if( xTaskIncrementTick() != pdFALSE ) {
                vTaskSwitchContext();
        }

        /* Clear the PIT interrupt. */
        ulDummy = AT91C_BASE_PITC->PITC_PIVR;

        /* To remove compiler warning. */
        ( void ) ulDummy;

        /* The AIC is cleared in the asm wrapper, outside of this function. */
}
/*-----------------------------------------------------------*/

static void prvSetupTimerInterrupt( void )
{
        const unsigned long ulPeriodIn_uS = ( 1.0 / ( double ) configTICK_RATE_HZ ) * port1SECOND_IN_uS;

        /* Setup the PIT for the required frequency. */
        PIT_Init( ulPeriodIn_uS, BOARD_MCK / port1MHz_IN_Hz );

        /* Setup the PIT interrupt. */
        AIC_DisableIT( AT91C_ID_SYS );
        AIC_ConfigureIT( AT91C_ID_SYS, AT91C_AIC_PRIOR_LOWEST, vPortTickISR );
        AIC_EnableIT( AT91C_ID_SYS );
        PIT_EnableIT();
}
/*-----------------------------------------------------------*/

void vPortEnterCritical( void )
{
        /* Disable interrupts first! */
        __disable_irq();

        /* Now interrupts are disabled ulCriticalNesting can be accessed
        directly.  Increment ulCriticalNesting to keep a count of how many times
        portENTER_CRITICAL() has been called. */
        ulCriticalNesting++;
}
/*-----------------------------------------------------------*/

void vPortExitCritical( void )
{
        if( ulCriticalNesting > portNO_CRITICAL_NESTING ) {
                /* Decrement the nesting count as we are leaving a critical section. */
                ulCriticalNesting--;

                /* If the nesting level has reached zero then interrupts should be
                re-enabled. */
                if( ulCriticalNesting == portNO_CRITICAL_NESTING ) {
                        __enable_irq();
                }
        }
}
/*-----------------------------------------------------------*/






