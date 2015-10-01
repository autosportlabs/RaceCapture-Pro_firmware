/*
	OpenRTOS V7.4.1 Copyright (C) Real Time Engineers ltd.

	OpenRTOS is distributed exclusively by Wittenstein High Integrity Systems,
	and is subject to the terms of the License granted to your organization,
	including its warranties and limitations on distribution.  It cannot be
	copied or reproduced in any way except as permitted by the License.

	Licenses are issued for each concurrent user working on a specified product
	line.

	WITTENSTEIN high integrity systems is a trading name of WITTENSTEIN
	aerospace & simulation ltd, Registered Office: Brown's Court, Long Ashton
	Business Park, Yanley Lane, Long Ashton, Bristol, BS41 9LB, UK.
	Tel: +44 (0) 1275 395 600, fax: +44 (0) 1275 393 630.
	E-mail: info@HighIntegritySystems.com
	Registered in England No. 3711047; VAT No. GB 729 1583 15

	http://www.HighIntegritySystems.com
*/

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include "stm32f4xx.h"

/*-----------------------------------------------------------
 * Application specific definitions.
 *
 * These definitions should be adjusted for your particular hardware and
 * application requirements.
 *
 * THESE PARAMETERS ARE DESCRIBED WITHIN THE 'CONFIGURATION' SECTION OF THE
 * FreeRTOS API DOCUMENTATION AVAILABLE ON THE FreeRTOS.org WEB SITE.
 *
 * See http://www.freertos.org/a00110.html.
 *----------------------------------------------------------*/

#define configUSE_PREEMPTION		1
#define configUSE_IDLE_HOOK		0
#define configUSE_TICKLESS_IDLE		0
#define configUSE_TICK_HOOK		1
#define configCPU_CLOCK_HZ		( SystemCoreClock )
#define configTICK_RATE_HZ		1000
#define configMAX_PRIORITIES		((unsigned portBASE_TYPE) 5)
#define configMINIMAL_STACK_SIZE	((unsigned short) 170)
#define configMAX_TASK_NAME_LEN		( 16 )
#define configUSE_TRACE_FACILITY	1
#define configUSE_16_BIT_TICKS		0
#define configIDLE_SHOULD_YIELD		1
#define configUSE_MUTEXES		1
#define configQUEUE_REGISTRY_SIZE	10
#define configGENERATE_RUN_TIME_STATS	0
#define configUSE_STATS_FORMATTING_FUNCTIONS 1
#define configCHECK_FOR_STACK_OVERFLOW	0
#define configUSE_RECURSIVE_MUTEXES	1
#define configUSE_MALLOC_FAILED_HOOK	0
#define configUSE_APPLICATION_TASK_TAG	0

/* Co-routine definitions. */
#define configUSE_CO_ROUTINES 		0
#define configMAX_CO_ROUTINE_PRIORITIES ( 2 )

/* Set the following definitions to 1 to include the API function, or zero
to exclude the API function. */

#define INCLUDE_vTaskPrioritySet		1
#define INCLUDE_uxTaskPriorityGet		1
#define INCLUDE_vTaskDelete			1
#define INCLUDE_vTaskCleanUpResources		0
#define INCLUDE_vTaskSuspend			1
#define INCLUDE_vTaskDelayUntil			1
#define INCLUDE_vTaskDelay			1
#define INCLUDE_xTaskGetCurrentTaskHandle	1
#define INCLUDE_xSemaphoreGetMutexHolder	1

/* Software timer configuration. */
#define configUSE_TIMERS		1
#define configTIMER_TASK_PRIORITY	( 4 )
#define configTIMER_QUEUE_LENGTH	( 10 )
#define configTIMER_TASK_STACK_DEPTH	configMINIMAL_STACK_SIZE

/* Use the system definition, if there is one */
#ifdef __NVIC_PRIO_BITS
#define configPRIO_BITS       __NVIC_PRIO_BITS
#else
#define configPRIO_BITS       4        /* 15 priority levels */
#endif

#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY		15
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY	5

/* The lowest priority. */
#define configKERNEL_INTERRUPT_PRIORITY 	( configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )
/* Priority 5, or 95 as only the top four bits are implemented. */
/* !!!! configMAX_SYSCALL_INTERRUPT_PRIORITY must not be set to zero !!!!
See http://www.FreeRTOS.org/RTOS-Cortex-M3-M4.html. */
#define configMAX_SYSCALL_INTERRUPT_PRIORITY 	( configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )

#define vPortSVCHandler SVC_Handler
#define xPortPendSVHandler PendSV_Handler
#define xPortSysTickHandler SysTick_Handler

#endif /* FREERTOS_CONFIG_H */
