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
 * Implementation of functions defined in portable.h for the ARM7 port.
 *
 * Components that can be compiled to either ARM or THUMB mode are
 * contained in this file.  The ISR routines, which can only be compiled
 * to ARM mode are contained in portISR.c.
 *----------------------------------------------------------*/


/* Standard includes. */
#include <stdlib.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"

/* Constants required to setup the task context. */
#define portINITIAL_SPSR				( ( portSTACK_TYPE ) 0x1f ) /* System mode, ARM mode, interrupts enabled. */
#define portTHUMB_MODE_BIT				( ( portSTACK_TYPE ) 0x20 )
#define portINSTRUCTION_SIZE			( ( portSTACK_TYPE ) 4 )
#define portNO_CRITICAL_SECTION_NESTING	( ( portSTACK_TYPE ) 0 )

/* Constants required to setup the tick ISR. */
#define portENABLE_TIMER			( ( unsigned char ) 0x01 )
#define portPRESCALE_VALUE			0x00
#define portINTERRUPT_ON_MATCH		( ( unsigned long ) 0x01 )
#define portRESET_COUNT_ON_MATCH	( ( unsigned long ) 0x02 )

/* Constants required to setup the VIC for the tick ISR. */
#define portTIMER_VIC_CHANNEL		( ( unsigned long ) 0x0004 )
#define portTIMER_VIC_CHANNEL_BIT	( ( unsigned long ) 0x0010 )
#define portTIMER_VIC_ENABLE		( ( unsigned long ) 0x0020 )

/*-----------------------------------------------------------*/

/* Setup the timer to generate the tick interrupts. */
static void prvSetupTimerInterrupt( void );

/*
 * The scheduler can only be started from ARM mode, so
 * vPortISRStartFirstSTask() is defined in portISR.c.
 */
extern void vPortISRStartFirstTask( void );

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

        /* The last thing onto the stack is the status register, which is set for
        system mode, with interrupts enabled. */
        *pxTopOfStack = ( portSTACK_TYPE ) portINITIAL_SPSR;

        if( ( ( unsigned long ) pxCode & 0x01UL ) != 0x00 ) {
                /* We want the task to start in thumb mode. */
                *pxTopOfStack |= portTHUMB_MODE_BIT;
        }

        pxTopOfStack--;

        /* Some optimisation levels use the stack differently to others.  This
        means the interrupt flags cannot always be stored on the stack and will
        instead be stored in a variable, which is then saved as part of the
        tasks context. */
        *pxTopOfStack = portNO_CRITICAL_SECTION_NESTING;

        return pxTopOfStack;
}
/*-----------------------------------------------------------*/

portBASE_TYPE xPortStartScheduler( void )
{
        /* Start the timer that generates the tick ISR.  Interrupts are disabled
        here already. */
        prvSetupTimerInterrupt();

        /* Start the first task. */
        vPortISRStartFirstTask();

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

/*
 * Setup the timer 0 to generate the tick interrupts at the required frequency.
 */
static void prvSetupTimerInterrupt( void )
{
        unsigned long ulCompareMatch;
        extern void ( vTickISR )( void );

        /* A 1ms tick does not require the use of the timer prescale.  This is
        defaulted to zero but can be used if necessary. */
        T0_PR = portPRESCALE_VALUE;

        /* Calculate the match value required for our wanted tick rate. */
        ulCompareMatch = configCPU_CLOCK_HZ / configTICK_RATE_HZ;

        /* Protect against divide by zero.  Using an if() statement still results
        in a warning - hence the #if. */
#if portPRESCALE_VALUE != 0
        {
                ulCompareMatch /= ( portPRESCALE_VALUE + 1 );
        }
#endif
        T0_MR0 = ulCompareMatch;

        /* Generate tick with timer 0 compare match. */
        T0_MCR = portRESET_COUNT_ON_MATCH | portINTERRUPT_ON_MATCH;

        /* Setup the VIC for the timer. */
        VICIntSelect &= ~( portTIMER_VIC_CHANNEL_BIT );
        VICIntEnable |= portTIMER_VIC_CHANNEL_BIT;

        /* The ISR installed depends on whether the preemptive or cooperative
        scheduler is being used. */

        VICVectAddr0 = ( long ) vTickISR;
        VICVectCntl0 = portTIMER_VIC_CHANNEL | portTIMER_VIC_ENABLE;

        /* Start the timer - interrupts are disabled when this function is called
        so it is okay to do this here. */
        T0_TCR = portENABLE_TIMER;
}
/*-----------------------------------------------------------*/



