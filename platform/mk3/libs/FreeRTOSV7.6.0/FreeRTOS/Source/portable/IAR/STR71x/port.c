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
 * Implementation of functions defined in portable.h for the ST STR71x ARM7
 * port.
 *----------------------------------------------------------*/

/* Library includes. */
#include "wdg.h"
#include "eic.h"

/* Standard includes. */
#include <stdlib.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"

/* Constants required to setup the initial stack. */
#define portINITIAL_SPSR				( ( portSTACK_TYPE ) 0x1f ) /* System mode, ARM mode, interrupts enabled. */
#define portTHUMB_MODE_BIT				( ( portSTACK_TYPE ) 0x20 )
#define portINSTRUCTION_SIZE			( ( portSTACK_TYPE ) 4 )

/* Constants required to handle critical sections. */
#define portNO_CRITICAL_NESTING 		( ( unsigned long ) 0 )

#define portMICROS_PER_SECOND 1000000

/*-----------------------------------------------------------*/

/* Setup the watchdog to generate the tick interrupts. */
static void prvSetupTimerInterrupt( void );

/* ulCriticalNesting will get set to zero when the first task starts.  It
cannot be initialised to 0 as this will cause interrupts to be enabled
during the kernel initialisation process. */
unsigned long ulCriticalNesting = ( unsigned long ) 9999;

/* Tick interrupt routines for cooperative and preemptive operation
respectively.  The preemptive version is not defined as __irq as it is called
from an asm wrapper function. */
__arm __irq void vPortNonPreemptiveTick( void );
void vPortPreemptiveTick( void );

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

        if( ( ( unsigned long ) pxCode & 0x01UL ) != 0x00UL ) {
                /* We want the task to start in thumb mode. */
                *pxTopOfStack |= portTHUMB_MODE_BIT;
        }

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

/* The cooperative scheduler requires a normal IRQ service routine to
simply increment the system tick. */
__arm __irq void vPortNonPreemptiveTick( void )
{
        /* Increment the tick count - which may wake some tasks but as the
        preemptive scheduler is not being used any woken task is not given
        processor time no matter what its priority. */
        xTaskIncrementTick();

        /* Clear the interrupt in the watchdog and EIC. */
        WDG->SR = 0x0000;
        portCLEAR_EIC();
}
/*-----------------------------------------------------------*/

/* This function is called from an asm wrapper, so does not require the __irq
keyword. */
void vPortPreemptiveTick( void )
{
        /* Increment the tick counter. */
        if( xTaskIncrementTick() != pdFALSE ) {
                /* Select a new task to execute. */
                vTaskSwitchContext();
        }

        /* Clear the interrupt in the watchdog and EIC. */
        WDG->SR = 0x0000;
        portCLEAR_EIC();
}
/*-----------------------------------------------------------*/

static void prvSetupTimerInterrupt( void )
{
        /* Set the watchdog up to generate a periodic tick. */
        WDG_ECITConfig( DISABLE );
        WDG_CntOnOffConfig( DISABLE );
        WDG_PeriodValueConfig( portMICROS_PER_SECOND / configTICK_RATE_HZ );

        /* Setup the tick interrupt in the EIC. */
        EIC_IRQChannelPriorityConfig( WDG_IRQChannel, 1 );
        EIC_IRQChannelConfig( WDG_IRQChannel, ENABLE );
        EIC_IRQConfig( ENABLE );
        WDG_ECITConfig( ENABLE );

        /* Start the timer - interrupts are actually disabled at this point so
        it is safe to do this here. */
        WDG_CntOnOffConfig( ENABLE );
}
/*-----------------------------------------------------------*/

__arm __interwork void vPortEnterCritical( void )
{
        /* Disable interrupts first! */
        __disable_interrupt();

        /* Now interrupts are disabled ulCriticalNesting can be accessed
        directly.  Increment ulCriticalNesting to keep a count of how many times
        portENTER_CRITICAL() has been called. */
        ulCriticalNesting++;
}
/*-----------------------------------------------------------*/

__arm __interwork void vPortExitCritical( void )
{
        if( ulCriticalNesting > portNO_CRITICAL_NESTING ) {
                /* Decrement the nesting count as we are leaving a critical section. */
                ulCriticalNesting--;

                /* If the nesting level has reached zero then interrupts should be
                re-enabled. */
                if( ulCriticalNesting == portNO_CRITICAL_NESTING ) {
                        __enable_interrupt();
                }
        }
}
/*-----------------------------------------------------------*/






