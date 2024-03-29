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
 * Implementation of functions defined in portable.h for the ARM CM4F port.
 *----------------------------------------------------------*/

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"

/* Constants required to manipulate the NVIC. */
#define portNVIC_SYSTICK_CTRL		( ( volatile unsigned long * ) 0xe000e010 )
#define portNVIC_SYSTICK_LOAD		( ( volatile unsigned long * ) 0xe000e014 )
#define portNVIC_SYSPRI2			( ( volatile unsigned long * ) 0xe000ed20 )
#define portNVIC_SYSTICK_CLK		0x00000004
#define portNVIC_SYSTICK_INT		0x00000002
#define portNVIC_SYSTICK_ENABLE		0x00000001
#define portNVIC_PENDSV_PRI			( ( ( unsigned long ) configKERNEL_INTERRUPT_PRIORITY ) << 16 )
#define portNVIC_SYSTICK_PRI		( ( ( unsigned long ) configKERNEL_INTERRUPT_PRIORITY ) << 24 )

/* Constants required to manipulate the VFP. */
#define portFPCCR					( ( volatile unsigned long * ) 0xe000ef34 ) /* Floating point context control register. */
#define portASPEN_AND_LSPEN_BITS	( 0x3UL << 30UL )

/* Constants required to set up the initial stack. */
#define portINITIAL_XPSR			( 0x01000000 )
#define portINITIAL_EXEC_RETURN		( 0xfffffffd )

/* Let the user override the pre-loading of the initial LR with the address of
prvTaskExitError() in case is messes up unwinding of the stack in the
debugger. */
#ifdef configTASK_RETURN_ADDRESS
#define portTASK_RETURN_ADDRESS	configTASK_RETURN_ADDRESS
#else
#define portTASK_RETURN_ADDRESS	prvTaskExitError
#endif

/* The priority used by the kernel is assigned to a variable to make access
from inline assembler easier. */
const unsigned long ulKernelPriority = configKERNEL_INTERRUPT_PRIORITY;

/* Each task maintains its own interrupt status in the critical nesting
variable. */
static unsigned long ulCriticalNesting = 0xaaaaaaaaUL;

/*
 * Setup the timer to generate the tick interrupts.
 */
static void prvSetupTimerInterrupt( void );

/*
 * Exception handlers.
 */
void SysTick_Handler( void );

/*
 * Functions defined in port_asm.asm.
 */
extern void vPortEnableVFP( void );
extern void vPortStartFirstTask( void );

/*
 * Used to catch tasks that attempt to return from their implementing function.
 */
static void prvTaskExitError( void );

/* This exists purely to allow the const to be used from within the
port_asm.asm assembly file. */
const unsigned long ulMaxSyscallInterruptPriorityConst = configMAX_SYSCALL_INTERRUPT_PRIORITY;

/*-----------------------------------------------------------*/

/*
 * See header file for description.
 */
portSTACK_TYPE *pxPortInitialiseStack( portSTACK_TYPE *pxTopOfStack, pdTASK_CODE pxCode, void *pvParameters )
{
        /* Simulate the stack frame as it would be created by a context switch
        interrupt. */

        /* Offset added to account for the way the MCU uses the stack on entry/exit
        of interrupts, and to ensure alignment. */
        pxTopOfStack--;

        *pxTopOfStack = portINITIAL_XPSR;	/* xPSR */
        pxTopOfStack--;
        *pxTopOfStack = ( portSTACK_TYPE ) pxCode;	/* PC */
        pxTopOfStack--;
        *pxTopOfStack = ( portSTACK_TYPE ) portTASK_RETURN_ADDRESS;	/* LR */

        /* Save code space by skipping register initialisation. */
        pxTopOfStack -= 5;	/* R12, R3, R2 and R1. */
        *pxTopOfStack = ( portSTACK_TYPE ) pvParameters;	/* R0 */

        /* A save method is being used that requires each task to maintain its
        own exec return value. */
        pxTopOfStack--;
        *pxTopOfStack = portINITIAL_EXEC_RETURN;

        pxTopOfStack -= 8;	/* R11, R10, R9, R8, R7, R6, R5 and R4. */

        return pxTopOfStack;
}
/*-----------------------------------------------------------*/

static void prvTaskExitError( void )
{
        /* A function that implements a task must not exit or attempt to return to
        its caller as there is nothing to return to.  If a task wants to exit it
        should instead call vTaskDelete( NULL ).

        Artificially force an assert() to be triggered if configASSERT() is
        defined, then stop here so application writers can catch the error. */
        configASSERT( ulCriticalNesting == ~0UL );
        portDISABLE_INTERRUPTS();
        for( ;; );
}
/*-----------------------------------------------------------*/

/*
 * See header file for description.
 */
portBASE_TYPE xPortStartScheduler( void )
{
        /* configMAX_SYSCALL_INTERRUPT_PRIORITY must not be set to 0.
        See http://www.FreeRTOS.org/RTOS-Cortex-M3-M4.html */
        configASSERT( ( configMAX_SYSCALL_INTERRUPT_PRIORITY ) );

        /* Make PendSV and SysTick the lowest priority interrupts. */
        *(portNVIC_SYSPRI2) |= portNVIC_PENDSV_PRI;
        *(portNVIC_SYSPRI2) |= portNVIC_SYSTICK_PRI;

        /* Start the timer that generates the tick ISR.  Interrupts are disabled
        here already. */
        prvSetupTimerInterrupt();

        /* Initialise the critical nesting count ready for the first task. */
        ulCriticalNesting = 0;

        /* Ensure the VFP is enabled - it should be anyway. */
        vPortEnableVFP();

        /* Lazy save always. */
        *( portFPCCR ) |= portASPEN_AND_LSPEN_BITS;

        /* Start the first task. */
        vPortStartFirstTask();

        /* Should not get here! */
        return 0;
}
/*-----------------------------------------------------------*/

void vPortEndScheduler( void )
{
        /* It is unlikely that the CM4F port will require this function as there
        is nothing to return to.  */
}
/*-----------------------------------------------------------*/

void vPortYield( void )
{
        /* Set a PendSV to request a context switch. */
        *(portNVIC_INT_CTRL) = portNVIC_PENDSVSET;

        /* Barriers are normally not required but do ensure the code is completely
        within the specified behaviour for the architecture. */
        __DSB();
        __ISB();
}
/*-----------------------------------------------------------*/

void vPortEnterCritical( void )
{
        portDISABLE_INTERRUPTS();
        ulCriticalNesting++;
        __DSB();
        __ISB();
}
/*-----------------------------------------------------------*/

void vPortExitCritical( void )
{
        ulCriticalNesting--;
        if( ulCriticalNesting == 0 ) {
                portENABLE_INTERRUPTS();
        }
}
/*-----------------------------------------------------------*/

void SysTick_Handler( void )
{
        unsigned long ulDummy;

        ulDummy = portSET_INTERRUPT_MASK_FROM_ISR();
        {
                if( xTaskIncrementTick() != pdFALSE ) {
                        /* Pend a context switch. */
                        *(portNVIC_INT_CTRL) = portNVIC_PENDSVSET;
                }
        }
        portCLEAR_INTERRUPT_MASK_FROM_ISR( ulDummy );
}
/*-----------------------------------------------------------*/

/*
 * Setup the systick timer to generate the tick interrupts at the required
 * frequency.
 */
void prvSetupTimerInterrupt( void )
{
        /* Configure SysTick to interrupt at the requested rate. */
        *(portNVIC_SYSTICK_LOAD) = ( configCPU_CLOCK_HZ / configTICK_RATE_HZ ) - 1UL;
        *(portNVIC_SYSTICK_CTRL) = portNVIC_SYSTICK_CLK | portNVIC_SYSTICK_INT | portNVIC_SYSTICK_ENABLE;
}
/*-----------------------------------------------------------*/

