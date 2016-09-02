/*
 * Race Capture Firmware
 *
 * Copyright (C) 2016 Autosport Labs
 *
 * This file is part of the Race Capture firmware suite
 *
 * This is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details. You should
 * have received a copy of the GNU General Public License along with
 * this code. If not, see <http://www.gnu.org/licenses/>.
 */

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "cpu_device.h"
#include "led.h"
#include "led_device.h"
#include "panic.h"
#include "printk.h"
#include "task.h"

#define FLASH_PAUSE_DELAY_MS 	5000
#define FLASH_DELAY_MS		1000

/**
 * Called when the system hits a non-recoverable error.  Ensure to use
 * the correct PANIC_CAUSE_* enum to help users inform us of the issue.
 */
void panic(const enum panic_cause cause)
{
        taskDISABLE_INTERRUPTS();
	led_device_set_all(false);

        led_enable(LED_ERROR);
        for(;;) {
                led_enable(LED_GPS);
                led_enable(LED_LOGGER);
                cpu_device_spin(FLASH_PAUSE_DELAY_MS);
                led_disable(LED_GPS);
                led_disable(LED_LOGGER);
                cpu_device_spin(FLASH_DELAY_MS);

                for (int c = 0; c < cause - 1; ++c) {
                        led_enable(LED_GPS);
                        led_enable(LED_LOGGER);
                        cpu_device_spin(FLASH_DELAY_MS);
                        led_disable(LED_GPS);
                        led_disable(LED_LOGGER);
                        cpu_device_spin(FLASH_DELAY_MS);
                }
        }
}

/* *** FREE RTOS Hooks that lead to a panic *** */

void vApplicationMallocFailedHook(void)
{
        /*
         * vApplicationMallocFailedHook() will only be called if
         * configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h.  It
         * is a hook function that will get called if a call to pvPortMalloc()
         * fails. pvPortMalloc() is called internally by the kernel whenever a
         * task, queue, timer or semaphore is created.  It is also called by
         * various parts of the demo application.  If heap_1.c or heap_2.c are
         * used, then the size of the heap available to pvPortMalloc() is
         * defined by configTOTAL_HEAP_SIZE in FreeRTOSConfig.h, and the
         * xPortGetFreeHeapSize() API function can be used to query the size
         * of free heap space that remains (although it does not provide
         * information on how the remaining heap might be fragmented).
         */
        pr_error("MALLOC FAILURE\r\n");
        panic(PANIC_CAUSE_MALLOC);
}

void vApplicationStackOverflowHook(xTaskHandle pxTask, char *pcTaskName)
{
        /*
         * Run time stack overflow checking is performed if
         * configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.
         * This hook function is called if a stack overflow is detected.
         */
        (void) pxTask;
        pr_error_str_msg("STACK OVERFLOW in ", pcTaskName);
        panic(PANIC_CAUSE_OVERFLOW);
}

/* *** STM32 Hooks that lead to a panic *** */

/*
 * This is required by the STM32 libraries for their ASSERT macros to
 * work.  Useful if you need to catch HAL bugs
 */
void assert_failed(uint8_t* file, uint32_t line)
{
	pr_error("ASSERTION Failure\r\n");
	panic(PANIC_CAUSE_ASSERT);
}

void HardFault_Handler()
{
	panic(PANIC_CAUSE_ASSERT);
}
